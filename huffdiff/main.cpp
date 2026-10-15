#include "huffman.h"
#include "bitrw.h"
#include "pam.h"

struct diff_matrix {
	uint32_t width;
	uint32_t height;
	std::vector<int> matrix;

	diff_matrix(uint32_t w, uint32_t h) : width(w), height(h) { }

	void generate(const uint8_t* raster) {
		matrix.push_back(raster[0]);
		for (uint32_t r = 0; r < height; r++) {
			for (uint32_t c = 0; c < width; c++) {
				if (c == 0 && r > 0) {
					matrix.push_back(raster[r * width + c] - raster[(r - 1) * width + c]);
				}
				else if (c > 0) {
					matrix.push_back(raster[r * width + c] - raster[r * width + (c - 1)]);
				}
			}
		}
	}

	std::vector<uint8_t> revert() {
		std::vector<uint8_t> original;
		original.push_back(matrix[0]);
		for (uint32_t r = 0; r < height; r++) {
			for (uint32_t c = 0; c < width; c++) {
				if (c == 0 && r > 0) {
					original.push_back(matrix[r * width + c] + original[(r - 1) * width + c]);
				}
				else if (c > 0) {
					original.push_back(matrix[r * width + c] + original[r * width + (c - 1)]);
				}
			}
		}
		return original;
	}
};

void huffdiff_compress(const std::string& input_filename, const std::string& output_filename) {
	pam image(0, 0, TupleType::GRAYSCALE);
	if (!image.read(input_filename)) return;

	diff_matrix diff(static_cast<uint32_t>(image.get_width()), static_cast<uint32_t>(image.get_height()));
	diff.generate(image.get_raster());
	auto f = std::ranges::for_each(diff.matrix, frequency<int, uint64_t>{}).fun;

	huffman<int> h(f);
	h.make_canonical();

	std::ofstream os(output_filename, std::ios::binary);
	if (!os) {
		std::cout << "Cannot open file" << output_filename << "for writing";
	}
	bitwriter bw(os);

	os << "HUFFDIFF";
	bw(diff.width, 32);
	bw(diff.height, 32);
	bw(static_cast<int>(h.canonical.size()), 9);
	for (const auto& [len, sym, code] : h.canonical) {
		bw(sym, 9);
		bw(len, 5);
	}
	for (size_t i = 0; i < diff.width * diff.height; i++) {
		auto&& [code, len] = h.code_map[diff.matrix[i]];
		bw(code, len);
	}
	bw.flush();
}

void huffdiff_decompress(const std::string& input_filename, const std::string& output_filename, bool canonical = false) {
	std::ifstream is(input_filename, std::ios::binary);
	if (!is) {
		std::cout << "Cannot open file" << input_filename << " for reading";
	}
	bitreader br(is);

	std::string magic(8, ' ');
	is.read(magic.data(), 8);
	if (magic != "HUFFDIFF") {
		return;
	}

	uint32_t width = static_cast<uint32_t>(br(32));
	uint32_t height = static_cast<uint32_t>(br(32));
	uint32_t num_elements = static_cast<uint32_t>(br(9));
	
	huffman<int> h;
	for (size_t i = 0; i < num_elements; ++i) {
		int sym = static_cast<int>(br(9));
		if (sym & 0x100) {
			sym -= 0x200; // 9-bit two's complement
		}
		uint64_t len = br(5);
		h.code_map[sym] = { 0, len };
	}
	h.make_canonical();

	diff_matrix diff(width, height);
	for (size_t i = 0; i < width * height; ++i) {
		uint64_t curlen = 0;
		uint64_t curcode = 0;
		size_t tblpos;
		for (tblpos = 0; tblpos < num_elements; ++tblpos) {
			auto& [len, sym, code] = h.canonical[tblpos];
			auto readbits = len - curlen;
			curcode = (curcode << readbits) | br(readbits);
			curlen = len;
			if (curcode == code) {
				diff.matrix.push_back(sym);
				break;
			}
		}
		if (tblpos == num_elements) {
			std::cout << "Cannot find Huffman code!";
		}
	}

	pam image(width, height, TupleType::GRAYSCALE);
	image.set_raster(diff.revert());

	image.write(output_filename);
}


int main(int argc, char* argv[]) {

	if (argc != 4) {
		std::cout << "SYNTAX:\n"
			"huffdiff [c|d] <input file> <output file>";
		
		return EXIT_FAILURE;
	}

	std::string command = argv[1];
	if (command == "c") {
		huffdiff_compress(argv[2], argv[3]);
	}
	else if (command == "d") {
		huffdiff_decompress(argv[2], argv[3]);
	}
	else {
		std::cout << "Wrong option";
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}
