#include "pam.h"

#include <string>
#include <iostream>
#include <cstdlib>

#define EXERCISE1
#define EXERCISE2
#define EXERCISE3

// Runs the three exercises in sequence. Expects frog.pam and laptop.pam in the working directory.
int main() {

#ifdef EXERCISE1
	pam image1(256, 256, TupleType::GRAYSCALE);

	for (size_t i = 0; i < 256; i++) {
		image1.set_row(i, (uint8_t)i);
	}

	image1.write("ex1_output.pam");
#endif // EXERCISE1

#ifdef EXERCISE2
	pam image2(1, 1, TupleType::BLACKANDWHITE);

	image2.read("frog.pam");
	image2.flip_vertical();

	image2.write("ex2_output.pam");
#endif // EXERCISE2

#ifdef EXERCISE3
	pam image3(1, 1, TupleType::BLACKANDWHITE);

	image3.read("laptop.pam");
	image3.flip_horizontal();

	image3.write("ex3_output.pam");
#endif // EXERCISE3

	return EXIT_SUCCESS;
}