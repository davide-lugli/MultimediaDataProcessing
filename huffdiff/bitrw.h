#pragma once

#include <cstdint>   // For uint8_t and uint64_t
#include <cstddef>   // For size_t
#include <istream>   // For std::istream
#include <ostream>   // For std::ostream

template<typename T>
std::ostream& raw_write(std::ostream& os, const T& num, size_t size = sizeof(T)) {
    return os.write(reinterpret_cast<const char*>(&num), size);
}

template<typename T>
std::istream& raw_read(std::istream& is, T& num, size_t size = sizeof(T)) {
    return is.read(reinterpret_cast<char*>(&num), size);
}

// bitwriter class declaration.
class bitwriter {
public:
    // Constructor: takes a reference to an output stream.
    bitwriter(std::ostream& os) : os_(os) {};

    // Destructor: should flush remaining bits.
    ~bitwriter() {
        flush();
    };

    // Writes the lower 'numbits' bits of 'x' to the stream.
    std::ostream& operator()(uint64_t x, uint64_t numbits) {
        while (numbits-- > 0) {
            writebit(x >> numbits);
        }
        return os_;
    };

    // Flushes the current buffer, padding with 'padbit' (default 0) if needed.
    std::ostream& flush(int padbit = 0) {
        while (n_ > 0) {
            writebit(padbit);
        }
        return os_;
    };

private:
    // Internal buffer to collect bits.
    uint8_t buffer_;
    // Number of bits currently stored in the buffer.
    size_t n_ = 0;
    // Reference to the output stream.
    std::ostream& os_;

    // Helper function to write a single bit into the buffer.
    void writebit(uint64_t curbit) {
        buffer_ = (buffer_ << 1) | (curbit & 1);
        ++n_;
        if (n_ == 8) {
            os_.put(buffer_);
            n_ = 0;
        }
    };
};

// bitreader class declaration.
class bitreader {
public:
    // Constructor: takes a reference to an input stream.
    bitreader(std::istream& is) : is_(is) {};

    // Reads 'numbits' bits from the stream and returns them as a number.
    uint64_t operator()(uint64_t numbits) {
        uint64_t u = 0;
        while (numbits-- > 0) {
            u = (u << 1) | readbit();
        }
        return u;
    };

    // Returns true if the underlying stream is in a failed state.
    bool fail() const {
        return is_.fail();
    };

    // Allows using the object in boolean contexts (true if stream is OK).
    operator bool() const {
        return !fail();
    };

private:
    // Internal buffer to hold bits from the stream.
    uint8_t buffer_;
    // Number of bits left in the buffer.
    size_t n_ = 0;
    // Reference to the input stream.
    std::istream& is_;

    // Helper function to read a single bit from the buffer/stream.
    uint64_t readbit() {
        if (n_ == 0) {
            buffer_ = is_.get();
            n_ = 8;
        }
        --n_;
        return (buffer_ >> n_) & 1;
    };
};
