#pragma once

#include <cstdint>
#include <string>

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
	bool set_width(size_t w);
	bool set_height(size_t h);
	bool set_depth(size_t d);
	bool set_maxval(size_t m);
	void set_tupltype(TupleType t);
	bool set_row(size_t row, uint8_t value);
};
