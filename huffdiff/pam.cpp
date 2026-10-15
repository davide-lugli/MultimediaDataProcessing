#include "pam.h"

#include <fstream>
#include <iostream>
#include <algorithm>


pam::pam(size_t w, size_t h, TupleType t) : width(w), height(h), depth(0), maxval(0), tupltype(t), raster(nullptr) {
	set_tupltype(tupltype);
};

pam::~pam() {
	delete[] raster;
};

bool pam::write(std::string filename) {
	std::ofstream os(filename, std::ios::binary);
	if (!os) {
		std::cout << "Cannot open output file";
		return false;
	}

	// Header
	os << "P7\n";
	os << "WIDTH " << width << '\n';
	os << "HEIGHT " << height << '\n';
	os << "DEPTH " << depth << '\n';
	os << "MAXVAL " << maxval << '\n';
	os << "TUPLTYPE ";
	if (tupltype == TupleType::BLACKANDWHITE) {
		os << "BLACKANDWHITE";
	}
	else if (tupltype == TupleType::GRAYSCALE) {
		os << "GRAYSCALE";
	}
	else if (tupltype == TupleType::RGB) {
		os << "RGB";
	}
	os << '\n';
	os << "ENDHDR\n";

	// Raster 
	for (size_t i = 0; i < width * height * depth; i++) {
		os << raster[i];
	}

	return true;
}
bool pam::read(std::string filename) {
	std::ifstream is(filename, std::ios::binary);
	if (!is) {
		std::cout << "Cannot open input file";
		return false;
	}

	// Should have made copies and set at the end only if no errors
	// Set initial values
	width = height = depth = maxval = 0;
	tupltype = TupleType::RGB;
	delete[] raster;
	raster = nullptr;

	// Check magic number
	std::string s;
	is >> s;
	if (s != "P7") return false;

	// Read rest of header
	while (is) {
		is >> s;
		if (s == "WIDTH") {
			if (width > 0) return false;

			is >> width;
			if (width < 1) return false;
		}
		else if (s == "HEIGHT") {
			if (height > 0) return false;

			is >> height;
			if (height < 1) return false;
		}
		else if (s == "DEPTH") {
			if (depth > 0) return false;

			is >> depth;
			if (depth < 1) return false;
		}
		else if (s == "MAXVAL") {
			if (maxval > 0) return false;

			is >> maxval;
			if (maxval < 1) return false;
		}
		else if (s == "TUPLTYPE") {
			is >> s;
			if (s == "BLACKANDWHITE") tupltype = TupleType::BLACKANDWHITE;
			else if (s == "GRAYSCALE") tupltype = TupleType::GRAYSCALE;
			else if (s == "RGB") tupltype = TupleType::RGB;
			else return false;
		}
		else if (s == "ENDHDR") break;
		else {
			return false;
		}
	}

	// Skip newline after ENDHDR
	is.get();

	// Check TUPLTYPE, MAXVAL and DEPTH relationship
	if (tupltype == TupleType::RGB && depth != 3) return false;
	if (tupltype == TupleType::GRAYSCALE && depth != 1) return false;
	if (tupltype == TupleType::BLACKANDWHITE && (depth != 1 || maxval != 1)) return false;

	// Read raster
	raster = new uint8_t[width * height * depth];
	is.read(reinterpret_cast<char*>(raster), width * height * depth * sizeof(uint8_t));

	return true;
}

void pam::flip_vertical() {
	for (size_t r = 0; r < height / 2; r++) {
		for (size_t i = 0; i < width * depth; i++) {
			std::swap(raster[(height - 1 - r) * width * depth + i], raster[r * width * depth + i]);
		}
	}
}
void pam::flip_horizontal() {
	for (size_t c = 0; c < width / 2; c++) {
		for (size_t i = 0; i < height; i++) {
			for (size_t d = 0; d < depth; d++) {
				std::swap(raster[i * width * depth + (c * depth) + d], raster[i * width * depth + ((width - 1 - c) * depth) + d]);
			}
		}
	}
}

size_t pam::get_width() {
	return width;
}

size_t pam::get_height() {
	return height;
}

uint8_t* pam::get_raster() {
	return raster;
}

bool pam::set_width(size_t w) {
	if (w >= 1) {
		width = w;
		delete[] raster;
		raster = new uint8_t[width * height * depth];
		return true;
	}

	return false;
}
bool pam::set_height(size_t h) {
	if (h >= 1) {
		height = h;
		delete[] raster;
		raster = new uint8_t[width * height * depth];
		return true;
	}

	return false;
}
bool pam::set_depth(size_t d) {
	if (d >= 1) {
		depth = d;
		delete[] raster;
		raster = new uint8_t[width * height * depth];
		return true;
	}

	return false;
}
bool pam::set_maxval(size_t m) {
	if (m >= 1 && m <= 65535) {
		maxval = m;
		return true;
	}

	return false;
}
void pam::set_tupltype(TupleType t) {
	if (t == TupleType::BLACKANDWHITE) {
		set_depth(1);
		set_maxval(1);
	}
	else if (t == TupleType::GRAYSCALE) {
		set_depth(1);
		set_maxval(255);
		tupltype = t;
	}
	else if (t == TupleType::RGB) {
		set_depth(3);
		set_maxval(255);
	}

	tupltype = t;
}

bool pam::set_raster(std::vector<uint8_t> values) {
	for (size_t i = 0; i < values.size(); i++) {
		raster[i] = values[i];
	}
	return true;
}

bool pam::set_row(size_t row, uint8_t value) {
	if (row >= height || value > maxval) {
		return false;
	}

	for (size_t i = width * depth * row; i < width * depth * (row + 1); i++) {
		raster[i] = value;
	}

	return true;
}

bool pam::set_pixel(size_t row, size_t col, uint8_t value) {
	if (row >= height || col >= width || value > maxval) {
		return false;
	}

	raster[width * depth * row + col] = value;
	return true;
}
