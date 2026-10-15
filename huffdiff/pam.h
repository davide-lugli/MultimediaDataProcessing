#pragma once

#include <cstdint>
#include <string>
#include <vector>

enum class TupleType {
	BLACKANDWHITE,
	GRAYSCALE,
	RGB
};

class pam {
private:
	size_t width;
	size_t height;
	size_t depth;
	size_t maxval;
	TupleType tupltype;
	uint8_t* raster;
public:
	pam(size_t w, size_t h, TupleType t);
	~pam();
	pam(const pam&) = delete;
	pam& operator=(const pam&) = delete;
	bool write(std::string filename);
	bool read(std::string filename);
	void flip_vertical();
	void flip_horizontal();
	size_t get_width();
	size_t get_height();
	uint8_t* get_raster();
	bool set_width(size_t w);
	bool set_height(size_t h);
	bool set_depth(size_t d);
	bool set_maxval(size_t m);
	void set_tupltype(TupleType t);
	bool set_raster(std::vector<uint8_t> values);
	bool set_row(size_t row, uint8_t value);
	bool set_pixel(size_t row, size_t col, uint8_t value);
};
