#include <fstream>
#include <iostream>
#include <iomanip>
#include <cstdint>
#include <cstdlib>

using std::cout;
using std::ifstream;
using std::ofstream;
using std::ios;
using std::hex;
using std::setfill;
using std::setw;

void error(const char* msg) {
	cout << msg;
	exit(EXIT_FAILURE);
}

void casting_examples() {
	typedef char* charptr;
	uint8_t my_byte;
	// The type of &my_byte is uint8_t* meaning a pointer to uint8_t
	// read() expects a char pointer, so we need to cast
	// C-style cast: (char*)&my_byte
	// Function-style cast: char(my_byte) <-- doesn't work with pointers!
	// We can overcome the problem with a typedef
	char* c = charptr(&my_byte); // Function-style cast

	// Static cast, almost never used
	(double)5;
	const int x = 5;
	//	int* p = &x; <-- Works only if x is not a constant
	int* p = (int*)&x; // Removing constness == = const_cast
	float f = 5.4;
	int* q = (int*)&f; // Chnaging the interpretation of memory address === reinterpret_cast

	reinterpret_cast<char*>(&my_byte); // reinterpret_cast
}

void reading_byte_examples() {
	// METHOD 1
	ifstream is1("file.txt", ios::binary);

	// All extractors (<<, >>) skip whitespace by default!
	// A solution is to use get()
	char c1;
	while (is1.get(c1)) {}

	// METHOD 2
	ifstream is2("file.txt", ios::binary);

	// We could also use the extractor >>, but we need to disable a flag to read also whitespaces
	uint8_t c2;
	while (is2 >> std::noskipws >> c2) {}
	
	// METHOD 3
	ifstream is3("file.txt", ios::binary);

	uint8_t c3;
	// You can also set it for all iterations of is with:
	is3.unsetf(std::ios::skipws);
	while (is3 >> c3) {}

	// METHOD 4
	ifstream is4("file.txt", ios::binary);

	// Another way of reading bytes is
	uint8_t my_byte;
	is4.read(reinterpret_cast<char*>(&my_byte), 1); 
	// 1 is the number of bytes to read
	// The casting is needed because read requires a char pointer
}

int main(int argc, char* argv[]) {

	if (argc != 3) {
		error("SYNTAX:\n"
			"frequencies <input_file> <output_file>");
	}

	ifstream is(argv[1], ios::binary);
	if (!is) {
		error("Cannot open input file");
	}
	is.unsetf(ios::skipws);
	
	size_t occurrences[256] = { 0 };

	uint8_t c;
	while (is >> c) {
		++occurrences[c];
	}

	ofstream os(argv[2]);
	if (!os) {
		error("Cannot open output file");
	}

	for (size_t i = 0; i < 256; i++) {
		if (occurrences[i] > 0) {
			// Careful: std::ios::hex and std::hex behave very differently
			os << setfill('0') << setw(2) << hex << i;
			os << '\t' << occurrences[i] << '\n';
		}
	}

	return EXIT_SUCCESS;
}
