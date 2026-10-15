#include <fstream>
#include <iostream>
#include <cstdint>
#include <cstdlib>
#include <cstring>

void error(const char* msg) {
	std::cout << msg;
	exit(EXIT_FAILURE);
}

template<typename T>
// When taking a stream as input (always as a reference) it is good practise to return it in order to chain operations
std::ostream& raw_write(std::ostream& os, const T& num, size_t size = sizeof(T)) {
	return os.write(reinterpret_cast<const char*>(&num), size);
}

template<typename T>
std::istream& raw_read(std::istream& is, T& num, size_t size = sizeof(T)) {
	return is.read(reinterpret_cast<char*>(&num), size);
}

class bitreader {
private:
	std::istream& is_;
	uint8_t buffer_;
	size_t nbits_;

	uint32_t read_bit() {
		if (nbits_ == 0) {
			raw_read(is_, buffer_);
			nbits_ = 8;
		}
		--nbits_;
		return (buffer_ >> nbits_) & 1;
	}

public:
	bitreader(std::istream& is) : is_(is), nbits_(0) {}

	std::istream& read(uint32_t& u, size_t n) {
		u = 0;
		while (n-- > 0) {
			u = (u << 1) | read_bit();
		}
		return is_;
	}
};

class bitwriter {
private:
	std::ostream& os_;
	uint8_t buffer_;
	size_t nbits_;

	// Start
	// +---++---++---++---++---++---++---++---+
	// | a || b || c || d || e || f || g || h |
	// +---++---++---++---++---++---++---++---+
	// Shift to the left === multiplying by two
	// +---++---++---++---++---++---++---++---+
	// | b || c || d || e || f || g || h || 0 |
	// +---++---++---++---++---++---++---++---+

	std::ostream& write_bit(uint32_t u) {
		// u = 0000 0000 0000 0000 0000 0000 0000 0000 0001 (we want only the last bit, the 1)
		buffer_ = (buffer_ << 1) | (u & 1);
		// Equivalent to "(buffer_ * 2) + (u % 2)"
		++nbits_;
		if (nbits_ == 8) {
			raw_write(os_, buffer_);
			nbits_ = 0;
		}
		return os_;
	}

public:
	bitwriter(std::ostream& os) : os_(os), nbits_(0) {}
	~bitwriter() {
		flush();
	}

	// Writes the n least significant bits of u, from the most to least significant
	// i.e. from bit #n-1 to bit #0
	std::ostream& write(uint32_t u, size_t n) {
		// We can't do for(i = n-1; i >= 0; i--) because size_t is defined positive so always >= 0
		
		// Solution 1:
		// for (size_t i = n; i > 0;) { i--; write_bit(u >> i); }

		// Solution 2:
		// for (size_t i = n - 1; i < n; --i) { write_bit(u >> i); }

		// Solution 3:
		// while (n > 0) { n--; write_bit(u >> n); }

		// Best solution:
		while (n-- > 0) {
			write_bit(u >> n);
		}
		return os_;
	}

	void flush(uint32_t u = 0) {
		while (nbits_ > 0) {
			write_bit(u);
		}
	}
};

int main(int argc, char* argv[]) {

	if (argc != 4 || (strcmp(argv[1], "r") != 0 && strcmp(argv[1], "w") != 0)) {
		error("SYNTAX:\n"
			"bit_rw [r|w] <input_file> <output_file>\n"
			"  w: text file of integers -> packed 11-bit two's complement values\n"
			"  r: packed 11-bit two's complement values -> text file of integers\n");
	}

	if (argv[1][0] == 'r') {
		std::ifstream is(argv[2], std::ios::binary);
		if (!is) {
			error("Cannot open input file");
		}

		std::ofstream os(argv[3]);
		if (!os) {
			error("Cannot open output file");
		}

		bitreader br(is);

		uint32_t unum;
		while (br.read(unum, 11)) {
			int32_t num = unum;
			if (num & 0x400) {
				num -= 0x800;
			}
			os << num << '\n';
		}
	}
	else {
		std::ifstream is(argv[2]);
		if (!is) {
			error("Cannot open input file");
		}

		std::ofstream os(argv[3], std::ios::binary);
		if (!os) {
			error("Cannot open output file");
		}

		bitwriter bw(os);

		int32_t num;
		while (is >> num) {
			bw.write(num, 11);
		}
	}

	return EXIT_SUCCESS;
}
