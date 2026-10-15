#include <fstream>
#include <iostream>
#include <string>
#include <cstdint>
#include <iomanip>

void tabs(size_t num) {
	for (int t = 0; t < num; t++) std::cout << '\t';
}

class bencode {
	std::ifstream& ifile;

public:
	bencode(std::ifstream& is) : ifile(is) {};

	void read_int(size_t tab = 0) {
		tabs(tab);

		char c = ifile.get();
		while (c != 'e') {
			std::cout << c;
			c = ifile.get();
		}
	}

	void read_list(size_t tab = 0) {
		std::cout << "[\n";

		while (true) {
			char c = ifile.get();
			if (c == 'e') {
				break;
			}

			if (c == 'd') {
				tabs(tab + 1);
				read_dict(tab + 1);
			}
			else if (c == 'l') {
				tabs(tab + 1);
				read_list(tab + 1);
			}
			else if (c == 'i') {
				read_int(tab + 1);
			}
			else {
				read_word(tab + 1, &c);
			}

			std::cout << '\n';
		}

		tabs(tab);
		std::cout << ']';
		return;
	}

	std::string read_word(size_t tab = 0, char* startchar = nullptr, bool pieces = false) {
		if (pieces) std::cout << '\n';
		
		tabs(tab);
		if (!pieces) std::cout << '\"';
		
		int length = 0;
		char c;
		if (startchar == nullptr) c = 48;
		else c = *startchar;

		std::string word;

		while (c != ':') {
			length *= 10;
			length += c - 48;
			c = ifile.get();
		}

		if (pieces) {
			std::uint8_t x = 0;
			for (size_t i = 0; i < length; i++) {
				if (i > 0 && i % 20 == 0) {
					std::cout << '\n';
					tabs(tab);
				}

				ifile.read(reinterpret_cast<char*>(&x), 1);
				std::cout << std::setfill('0')  << std::setw(2) << std::hex << static_cast<int>(x);
			}
			return "";
		}

		for (size_t i = 0; i < length; i++) {
			c = ifile.get();
			if (c < 32 || c > 126) {
				std::cout << '.';
			}
			else {
				std::cout.write(&c, 1);
			}
			word.append(1, c);
		}

		std::cout << '\"';
		return word;
	}

	void read_dict(size_t tab = 0, bool is_info = false) {
		std::cout << "{\n";
		bool info_found = false;
		bool pieces_found = false;

		while (true) {
			char c = ifile.get();
			if (c == 'e') {
				break;
			}

			std::string key = read_word(tab + 1, &c, false);
			if (key == "info") info_found = true;
			if (key == "pieces") pieces_found = true;
			std::cout << " => ";

			c = ifile.get();
			if (c == 'd') {
				read_dict(tab + 1, info_found);
				info_found = false;
			}
			else if (c == 'l') {
				read_list(tab + 1);
			}
			else if (c == 'i') {
				read_int();
			}
			else {
				read_word(is_info && pieces_found ? tab + 2 : 0, &c, is_info && pieces_found);
				pieces_found = false;
			}

			std::cout << '\n';
		}
		
		tabs(tab);
		std::cout << '}';
		return;
	}

	void read() {
		ifile.get();
		read_dict();
	}
};

int main(int argc, char* argv[]) {

	if (argc != 2) {
		std::cout << "SYNTAX:  torrent_dump <file .torrent>";
		return -1;
	}

	std::ifstream is = std::ifstream(argv[1], std::ios::binary);
	if (!is) {
		std::cout << "Could not open input file";
		return -1;
	}

	bencode ben = bencode(is);
	ben.read();
}
