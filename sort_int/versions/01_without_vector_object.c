#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

// #define INEFFICIENT_VERSION

// We define the function to take 2 const void pointers in order to conform to the
// _CoreCrtNonSecureSearchSortCompareFunction definition, as required by qsort()
int compare_int(const void* va, const void* vb) {
	// Cast the pointers to int
	const int* a = va;
	const int* b = vb;

	if (*a < *b) {
		return -1;
	}
	else if (*a > *b) {
		return 1;
	}
	else {
		return 0;
	}

	// We can't do "return *a - *b" because if a is the lowest possible number it overflows
}

int main(int argc, char* argv[]) {
	if (argc != 3) {
		return EXIT_FAILURE;
	}

	// Opening in text mode, so it translates new lines
	// 0x0d 0x0a becomes 0x0a
	FILE* fin = fopen(argv[1], "r");
	if (fin == NULL) {
		return EXIT_FAILURE;
	}

#ifndef INEFFICIENT_VERSION
	int* v = NULL;
	size_t size = 0;
	size_t capacity = 0;

	while (true) {
		// Read a 32 bit integer (%d, as most selectors, skips whitespace)
		int val;
		int res = fscanf(fin, "%d", &val);

		// Check if read was successful
		if (res != 1) {
			break;
		}

		// Double capacity if needed
		++size;
		if (size > capacity) {
			capacity = capacity * 2 + 1;
			v = realloc(v, capacity * sizeof(int));
		}

		// Store in the array
		v[size - 1] = val;
	}
#else
	// This version is slow because it reallocates memory every single time
	// A better way is to simply double the array size every time we fill it up
	
	int* v = NULL;
	size_t size = 0;
	
	while(true) {
		// Read a 32 bit integer (%d, as most selectors, skips whitespace)
		int val;
		int res = fscanf(f, "%d", &val);
		
		// Check if read was successful
		if (res != 1) {
			break;
		}

		// Store in the array
		++size;
		v = realloc(v, size * sizeof(int));
		v[size - 1] = val;
	}
#endif // !INEFFICIENT_VERSION

	fclose(fin);

	// Shrink to fit
	capacity = size;
	v = realloc(v, capacity * sizeof(int));

	// Sort v
	qsort(v, size, sizeof(int), compare_int);

	// Save the output to the second file
	FILE* fout = fopen(argv[2], "w");
	if (fout == NULL) {
		return EXIT_FAILURE;
	}

	for (size_t i = 0; i < size; i++) {
		fprintf(fout, "%d\n", v[i]);
	}

	fclose(fout);
	free(v);

	return EXIT_SUCCESS;
}
