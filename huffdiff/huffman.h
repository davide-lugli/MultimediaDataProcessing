#pragma once

#include <cstdint>         // For uint8_t, uint64_t
#include <unordered_map>   // For std::unordered_map
#include <vector>          // For std::vector
#include <tuple>           // For std::tuple
#include <memory>          // For std::unique_ptr
#include <ranges>          // For std::ranges (C++20)
#include <string>          // For std::string
#include <fstream>         // For std::ifstream, std::ofstream
#include <iostream>        // For std::cout
#include <algorithm>       // For std::ranges::for_each
#include <iterator>        // For std::istreambuf_iterator
#include "bitrw.h"

template<typename T, typename CT = uint64_t>
struct frequency {
    std::unordered_map<T, CT> counters_;

    // Increments the count for the given value.
    void operator()(const T& val) {
        ++counters_[val];
    };

    // Returns a constant iterator to the beginning of the counters.
    auto begin() const { return counters_.begin(); };

    // Returns a constant iterator to the end of the counters.
    auto end() const { return counters_.end(); };

    // Returns the number of unique keys in the counters.
    auto size() const { return counters_.size(); };
};

template<typename T>
struct huffman {
    // Internal node structure.
    struct node {
        T sym_;
        uint64_t freq_;
        node* left_;
        node* right_;
    };

    // Comparison functor for node pointers.
    struct nodeptr_less {
        bool operator()(const node* a, const node* b) {
            return a->freq_ > b->freq_;
        };
    };

    // Container holding all created nodes.
    std::vector<std::unique_ptr<node>> nodes_;

    // Creates a new node with forwarded arguments.
    template<typename... Ts>
    node* make_node(Ts... args) {
        nodes_.emplace_back(std::make_unique<node>(std::forward<Ts>(args)...));
        return nodes_.back().get();
    };

    // Map from symbol to its Huffman code and bit-length.
    std::unordered_map<T, std::pair<uint64_t, uint64_t>> code_map;

    // For canonical code generation: each tuple is {length, symbol, code}.
    std::vector<std::tuple<uint64_t, T, uint64_t>> canonical;

    // Recursively generates Huffman codes from the tree rooted at 'n'.
    void generate_codes(const node* n, uint64_t code = 0, uint64_t len = 0) {
        if (n->left_ == 0) {
            code_map[n->sym_] = { code, len };
        }
        else {
            generate_codes(n->left_, (code << 1) | 0, len + 1);
            generate_codes(n->right_, (code << 1) | 1, len + 1);
        }
    };

    // Converts the generated codes into canonical form.
    void make_canonical() {
        canonical.clear();
        for (const auto& [sym, x] : code_map) {
            auto&& [code, len] = x;
            canonical.emplace_back(len, sym, code);
        }
        std::ranges::sort(canonical);
        uint64_t curcode = 0, curlen = 0;
        for (auto& [len, sym, code] : canonical) {
            uint64_t addbits = len - curlen;
            curcode <<= addbits;
            code_map[sym].first = curcode;
            code = curcode;
            curlen = len;
            ++curcode;
        }
    };

    // Default constructor.
    huffman() {};

    // Constructs the Huffman tree from a range (for example, a frequency table).
    template<std::ranges::range R>
    huffman(R&& r) {
        std::vector<node*> v;
        for (auto it = std::begin(r); it != std::end(r); ++it) {
            v.emplace_back(make_node(it->first, it->second));
        }
        std::ranges::sort(v, nodeptr_less{});

        while (v.size() > 1) {
            node* n1 = v.back();
            v.pop_back();
            node* n2 = v.back();
            v.pop_back();

            node* n = make_node(T{}, n1->freq_ + n2->freq_, n1, n2);

            auto pos = std::ranges::lower_bound(v, n, nodeptr_less{});
            v.insert(pos, n);
        }
        node* root = v.back();
        v.pop_back();

        generate_codes(root);
    };
};

/**
 * Compresses a file using Huffman coding.
 *
 * @param input_filename  The path to the file to compress.
 * @param output_filename The path for the compressed output.
 */
inline void huffman_compress(const std::string& input_filename, const std::string& output_filename, bool canonical = false) {
    std::ifstream is(input_filename, std::ios::binary);
    if (!is) {
        std::cout << "Cannot open file" << input_filename << "for reading";
    }

    std::vector<uint8_t> v{ std::istreambuf_iterator<char>{is}, std::istreambuf_iterator<char>() };

    auto f = std::ranges::for_each(v, frequency<uint8_t, uint64_t>{}).fun;

    huffman<uint8_t> h(f);
    if (canonical) h.make_canonical();

    std::ofstream os(output_filename, std::ios::binary);
    if (!os) {
        std::cout << "Cannot open file" << output_filename << "for writing";
    }
    bitwriter bw(os);

    if (!canonical) os << "HUFFMAN1";
    else os << "HUFFMAN2";
    os.put(static_cast<char>(f.size()));
    if (!canonical) {
        for (const auto& [sym, x] : h.code_map) {
            auto&& [code, len] = x;
            bw(sym, 8);
            bw(len, 5);
            bw(code, len);
        }
    }
    else {
        for (const auto& [len, sym, code] : h.canonical) {
            bw(sym, 8);
            bw(len, 5);
            //std::println("{:02x}, {}, {:b}", sym, len, code);
        }
    }
    bw(v.size(), 32);
    for (const auto& x : v) {
        auto&& [code, len] = h.code_map[x];
        bw(code, len);
    }
}

/**
 * Decompresses a Huffman-coded file.
 *
 * @param input_filename  The path to the compressed file.
 * @param output_filename The path for the decompressed output.
 */
inline void huffman_decompress(const std::string& input_filename, const std::string& output_filename, bool canonical = false) {
    std::ifstream is(input_filename, std::ios::binary);
    if (!is) {
        std::cout << "Cannot open file" << input_filename << " for reading";
    }
    bitreader br(is);

    std::string magic(8, ' ');
    is.read(magic.data(), 8);
    if ((!canonical && magic != "HUFFMAN1") || (canonical && magic != "HUFFMAN2")) {
        return;
    }

    size_t tblsize = is.get();
    if (tblsize == 0) {
        tblsize = 256;
    }
    std::vector<std::tuple<uint64_t, uint8_t, uint64_t>> tbl; // len,sym,code
    if (!canonical) {
        for (size_t i = 0; i < tblsize; ++i) {
            uint8_t sym = static_cast<uint8_t>(br(8));
            uint64_t len = br(5);
            uint64_t code = br(len);
            tbl.emplace_back(len, sym, code);
        }
        std::ranges::sort(tbl);
    }
    else {
        huffman<uint8_t> h;
        for (size_t i = 0; i < tblsize; ++i) {
            uint8_t sym = static_cast<uint8_t>(br(8));
            uint64_t len = br(5);
            h.code_map[sym] = { 0, len };
        }
        h.make_canonical();
        tbl = h.canonical;
    }
    size_t numsym = br(32);

    std::ofstream os(output_filename, std::ios::binary);
    if (!os) {
        std::cout << "Cannot open file" << output_filename << "for writing";
    }
    for (size_t i = 0; i < numsym; ++i) {
        uint64_t curlen = 0;
        uint64_t curcode = 0;
        size_t tblpos;
        for (tblpos = 0; tblpos < tbl.size(); ++tblpos) {
            auto& [len, sym, code] = tbl[tblpos];
            auto readbits = len - curlen;
            curcode = (curcode << readbits) | br(readbits);
            curlen = len;
            if (curcode == code) {
                os.put(sym);
                break;
            }
        }
        if (tblpos == tbl.size()) {
            std::cout << "Cannot find Huffman code!";
        }
    }
}
