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

int main(int argc, char* argv[]) {

	if (argc != 4 || (strcmp(argv[1], "r") != 0 && strcmp(argv[1], "w") != 0)) {
		error("SYNTAX:\n"
			"binary_rw [r|w] <input_file> <output_file>\n"
			"  w: text file of integers -> 32-bit binary integers\n"
			"  r: 32-bit binary integers -> text file of integers\n");
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

		int32_t num;
		while (raw_read(is, num)) {
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

		int32_t num;
		while (is >> num) {
			raw_write(os, num);
		}
	}

	return EXIT_SUCCESS;
}
