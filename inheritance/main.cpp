#define _CRT_SECURE_NO_WARNINGS

#include "canvas.h"

#include "string.h"
#ifdef _MSC_VER
#include <crtdbg.h>
#endif


struct shape {
	int x_, y_;
	char style_;
	char* name_;

	shape(int x, int y, char style) : x_(x), y_(y), style_(style), name_(nullptr) {}
	// IMPORTANT: IF WE HAVE AT LEAST ONE VIRTUAL THE DESTRUCTOR SHOULD BE VIRTUAL
	virtual ~shape() {
		delete[] name_;
	}

	void set_name(const char* name) {
		delete[] name_;
		name_ = new char[strlen(name) + 1];
		strcpy(name_, name);
	}
	const char* name() const {
		return name_;
	}

	// Virtual function --> can be overridden by derived classes
	// When using draw(), even if we're treating the object as if it was base class
	// the derived version of draw() will be used
	virtual void draw(canvas& c) {
		c.set(x_, y_, style_);
	}
};

struct point : public shape {
	point(int x, int y, char style) : shape(x, y, style) {}

	// No override draw() because it's already fine in the base class
};

struct rectangle : public shape {
	int x1_, y1_;

	rectangle(int x0, int y0, int x1, int y1, char style) : shape(x0, y0, style), x1_(x1), y1_(y1) {}

	void draw(canvas& c) override {
		c.rectangle(x_, y_, x1_, y1_, style_);
	}
};

struct line : public shape {
	int x1_, y1_;

	line(int x0, int y0, int x1, int y1, char style) : shape(x0, y0, style), x1_(x1), y1_(y1) {}

	void draw(canvas& c) override {
		c.line(x_, y_, x1_, y1_, style_);
	}
};

struct circle : public shape {
	int r_;

	circle(int xm, int ym, int r, char style) : shape(xm, ym, style), r_(r) {}

	void draw(canvas& c) override {
		c.circle(x_, y_, r_, style_);
	}
};

int main(int argc, char* argv[]) {
	canvas c(80, 25);

	rectangle* r = new rectangle(0, 0, 79, 24, '*');
	point* p = new point(5, 15, '?');
	circle* c1 = new circle(10, 10, 4, '@');
	circle* c2 = new circle(70, 10, 4, '@');
	line* l = new line(40, 15, 40, 20, '|');

	r->set_name("my_rectangle");
	p->set_name("my_point");
	c1->set_name("my_first_cirle");
	c2->set_name("my_second_circle");
	l->set_name("my_line");

	shape* arr[5] = { r, p, c1, c2, l };
	for (size_t i = 0; i < 5; i++) {
		arr[i]->draw(c);
	}

	c.out(stdout);

	for (size_t i = 0; i < 5; i++) {
		puts(arr[i]->name());
	}

	for (size_t i = 0; i < 5; i++) {
		delete arr[i];
	}

#ifdef _MSC_VER
	_CrtDumpMemoryLeaks();
#endif
}
