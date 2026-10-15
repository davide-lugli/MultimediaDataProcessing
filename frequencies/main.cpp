#include <fstream>
#include <iostream>
#include <algorithm>
#include <iomanip>
#include <cstdint>
#include <cstdlib>
#include <cmath>
#include <iterator>

void error(const char* msg) {
	std::cout << msg;
	exit(EXIT_FAILURE);
}

struct frequency_counter {
	size_t occurrencies[256];

	frequency_counter() : occurrencies{ 0 } {}

	void operator()(uint8_t val) {
		++occurrencies[val];
	}

	const size_t& operator[](uint8_t pos) const {
		return occurrencies[pos];
	}
	size_t& operator[](uint8_t pos) {
		return occurrencies[pos];
	}

	size_t total() {
		size_t tot = 0;
		for (const auto& x : occurrencies) {
			tot += x;
		}
		return tot;
	}

	double entropy() {
		double tot = (double) total();
		double h = 0.0;
		for (const auto& x : occurrencies) {
			if (x > 0) {
				double px = x / tot;
				h += px * log2(px);
			}
		}
		return -h;
	}
};

int main(int argc, char* argv[]) {

	if (argc != 3) {
		error("SYNTAX:\n"
			"frequencies <input_file> <output_file>");
	}

	std::ifstream is(argv[1], std::ios::binary);
	if (!is) {
		error("Cannot open input file");
	}
	is.unsetf(std::ios::skipws);

	// We do f = because for_each returns a copy of the object instead of the object itself
	// This works because we create a new, temporary frequency_counter, the for_each fills it and finally 
	// the result gets copied to f
	frequency_counter f = std::for_each(
		std::istream_iterator<uint8_t>(is),
		std::istream_iterator<uint8_t>(),
		frequency_counter()
	);
	/* Equivalent to
		for (std::istream_iterator<uint8_t> it(is); it != std::istream_iterator<uint8_t>(); ++it) {
			f(*it);
		}
	*/

	std::ofstream os(argv[2]);
	if (!os) {
		error("Cannot open output file");
	}

	for (size_t i = 0; i < 256; i++) {
		if (f[i] > 0) {
			// Careful: std::ios::hex and std::hex behave very differently
			os << std::setfill('0') << std::setw(2) << std::hex << i;
			os << '\t' << f[i] << '\n';
		}
	}

	std::cout << "Entropy: " << f.entropy() << '\n';

	return EXIT_SUCCESS;
}
