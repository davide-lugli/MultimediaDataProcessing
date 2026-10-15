#pragma once

#include <cstdio>

struct canvas {
private:
	int width_, height_;
	char* ptr_;
public:
	canvas(int width, int height);
	~canvas();
	void set(int x, int y, char c);
	void line(int x0, int y0, int x1, int y1, char c);
	void rectangle(int x0, int y0, int x1, int y1, char c);
	void circle(int xm, int ym, int r, char c);
	void out(FILE *f);
};
