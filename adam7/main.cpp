// 17.00

#include <cstdint>
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <format>
#include <cctype>
#include <cstdlib>

class bitreader {
public:
	bitreader(std::istream& is) : is_(is) {};

	uint64_t operator()(uint64_t nbits) {
		uint64_t u = 0;
		while (nbits-- > 0) {
			u = (u << 1) | readbit();
		}
		return u;
	}
private:
	uint8_t buffer_;
	size_t n_ = 0;
	std::istream& is_;

	uint64_t readbit() {
		if (n_ == 0) {
			buffer_ = is_.get();
			n_ = 8;
		}
		--n_;
		return (buffer_ >> n_) & 1;
	}
};

class bitwriter {
private:
	uint8_t buffer_;
	size_t n_ = 0;
	std::ostream& os_;

	void writebit(uint64_t curbit) {
		buffer_ = (buffer_ << 1) | (curbit & 1);
		++n_;
		if (n_ == 8) {
			os_.put(buffer_);
			n_ = 0;
		};
	}
public:
	bitwriter(std::ostream& os) : buffer_(0), n_(0), os_(os) {}

	~bitwriter() {
		flush();
	}

	void operator()(uint64_t value, size_t nbits) {
		while (nbits-- > 0) {
			writebit(value >> nbits);
		}
	}

	void flush(int padbit = 0) {
		while (n_ > 0) {
			writebit(padbit);
		}
	}
};

class little_bitwriter {
private:
	std::vector<uint8_t> bytebuf_;
	uint8_t buffer_;
	size_t n_ = 0;
	std::ostream& os_;

	void writebit(uint64_t curbit) {
		buffer_ = (buffer_ << 1) | (curbit & 1);
		++n_;
		if (n_ == 8) {
			bytebuf_.emplace_back(buffer_);
			n_ = 0;
		};
	}

	void writebuf() {
		for (size_t i = bytebuf_.size(); i > 0; i--) {
			os_.put(bytebuf_[i-1]);
		}
		bytebuf_.clear();
	}
public:
	little_bitwriter(std::ostream& os) : buffer_(0), n_(0), os_(os) {}

	~little_bitwriter() {
		flush();
	}

	void operator()(uint64_t value, size_t nbits) {
		while (nbits-- > 0) {
			writebit(value >> nbits);
		}
		writebuf();
	}

	void flush(int padbit = 0) {
		while (n_ > 0) {
			writebit(padbit);
		}
	}
};

struct pgm {
	uint32_t width;
	uint32_t height;
	uint32_t maxval;
	std::vector<uint16_t> data;

	void skip_ws_comments(std::istream& is) {
		while (std::isspace(is.peek())) is.get();
		while (is.peek() == '#') {
			std::string tmp;
			std::getline(is, tmp, '\n');
		}
	}

	pgm() {
		width = 0;
		height = 0;
		maxval = 0;
		data = {};
	}

	pgm(std::istream& is) {
		// Check magic
		std::string magic;
		magic.resize(2);
		is >> magic;
		if (magic != "P5") return;

		// Read header
		skip_ws_comments(is);
		is >> width;
		skip_ws_comments(is);
		is >> height;
		skip_ws_comments(is);
		is >> maxval;
		if (maxval >= 65536 || maxval == 0) return;
		skip_ws_comments(is);

		// Read data
		for (size_t i = 0; i < height * width; i++) {
			uint16_t tmp;
			if (maxval > 255) {
				is >> tmp;
			}
			else {
				tmp = is.get();
			}
			data.emplace_back(tmp);
		}
	}

	void write(std::ostream& os) {
		// Write header
		os << "P5";
		os.put(' ');
		os << width;
		os.put(' ');
		os << height;
		os.put(' ');
		os << maxval;
		os.put('\n');

		bitwriter bw(os);
		size_t bits = (maxval > 255) ? 16 : 8;

		// Write data
		for (size_t i = 0; i < height * width; i++) {
			bw(data[i], bits);
		}
	}
};

struct mlt {
	const std::vector<std::vector<uint8_t>> pattern = {
		{ 1, 6, 4, 6, 2, 6, 4, 6 },
		{ 7, 7, 7, 7, 7, 7, 7, 7 },
		{ 5, 6, 5, 6, 5, 6, 5, 6 },
		{ 7, 7, 7, 7, 7, 7, 7, 7 },
		{ 3, 6, 4, 6, 3, 6, 4, 6 },
		{ 7, 7, 7, 7, 7, 7, 7, 7 },
		{ 5, 6, 5, 6, 5, 6, 5, 6 },
		{ 7, 7, 7, 7, 7, 7, 7, 7 },
	};

	uint32_t width;
	uint32_t height;
	std::vector<std::vector<uint16_t>> levels;

	mlt(std::istream& is) {
		// Check magic
		std::string magic;
		magic.resize(8);
		is.read(&magic[0], 8);
		if (magic != "MULTIRES") return;

		// Read header
		is.read(reinterpret_cast<char*>(&width), 4);
		is.read(reinterpret_cast<char*>(&height), 4);

		std::vector<uint64_t> level_sizes = { 0, 0, 0, 0, 0, 0, 0 };

		// Read data
		for (size_t r = 0; r < height; r++) {
			for (size_t c = 0; c < width; c++) {
				uint8_t ri = r % 8;
				uint8_t ci = c % 8;

				uint8_t level = pattern[ri][ci] - 1;
				level_sizes[level]++;
			}
		}

		for (size_t l = 0; l < 7; l++) {
			levels.emplace_back();
			for (size_t i = 0; i < level_sizes[l]; i++) {
				uint16_t val = is.get();
				levels[l].emplace_back(val);
			}
		}
	}

	mlt(pgm& pgm_file) {
		width = pgm_file.width;
		height = pgm_file.height;

		for (size_t i = 0; i < 7; i++) {
			levels.emplace_back();
		}

		for (size_t r = 0; r < height; r++) {
			for (size_t c = 0; c < width; c++) {
				uint8_t ri = r % 8;
				uint8_t ci = c % 8;

				uint8_t level = pattern[ri][ci] - 1;
				levels[level].emplace_back(pgm_file.data[r * width + c]);
			}
		}
	}

	void write(std::ostream& os) {
		// Write header
		os << "MULTIRES";

		little_bitwriter br(os);

		br(width, 32);
		br(height, 32);

		// Write data
		for (auto l : levels) {
			for (auto v : l) {
				os.put(static_cast<char>(v));
			}
		}
	}

	void write_pgms(std::string& prefix) {
		// Put every pixel back in its place: each level stores its pixels in raster order
		std::vector<uint16_t> image(static_cast<size_t>(width) * height);
		std::vector<size_t> next = { 0, 0, 0, 0, 0, 0, 0 };
		for (size_t r = 0; r < height; r++) {
			for (size_t c = 0; c < width; c++) {
				uint8_t level = pattern[r % 8][c % 8] - 1;
				image[r * width + c] = levels[level][next[level]++];
			}
		}

		// After level i, each known pixel covers a block_w x block_h rectangle,
		// whose top-left corner is always a pixel of level <= i
		const size_t block_w[7] = { 8, 4, 4, 2, 2, 1, 1 };
		const size_t block_h[7] = { 8, 8, 4, 4, 2, 2, 1 };

		for (size_t i = 0; i < 7; i++) {
			pgm out;
			out.width = width;
			out.height = height;
			out.maxval = 255;

			for (size_t r = 0; r < height; r++) {
				for (size_t c = 0; c < width; c++) {
					size_t src_r = r - r % block_h[i];
					size_t src_c = c - c % block_w[i];
					out.data.emplace_back(image[src_r * width + src_c]);
				}
			}

			std::string filename = prefix;
			filename.append(std::format("_{}.pgm", i+1));
			std::ofstream os(filename, std::ios::binary);
			out.write(os);
		}
	}
};

void pgm_to_mlt(std::istream& is, std::ofstream& os) {
	pgm pgm_file(is);
	
	mlt mlt_file(pgm_file);
	mlt_file.write(os);
}

void mlt_to_pgm(std::istream& is, std::string prefix) {
	mlt mlt2(is);
	mlt2.write_pgms(prefix);
}

int main(int argc, char* argv[]) {
	if (argc != 4) {
		return EXIT_FAILURE;
	}

	std::ifstream is(argv[2], std::ios::binary);
	if (!is) {
		return EXIT_FAILURE;
	}

	if (*argv[1] == 'c') {
		std::ofstream os(argv[3], std::ios::binary);
		if (!os) {
			return EXIT_FAILURE;
		}
		pgm_to_mlt(is, os);
	}
	else if (*argv[1] == 'd') {
		mlt_to_pgm(is, argv[3]);
	}
	else {
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}
