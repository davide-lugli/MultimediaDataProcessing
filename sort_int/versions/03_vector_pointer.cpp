#define _CRT_SECURE_NO_WARNINGS

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <assert.h>

struct vector {
	size_t size_;
	size_t capacity_;
	int* data_;

	vector() {
		size_ = capacity_ = 0;
		data_ = NULL;
	}
	// ~ --> Bloc Num then Alt + 126 (numeric keypad)
	~vector() {
		free(data_);
	}

	void push_back(int val) {
		// Double capacity if needed
		size_++;
		if (size_ > capacity_) {
			capacity_ = capacity_ * 2 + 1;
			data_ = (int*) realloc(data_, capacity_ * sizeof(int));
		}

		// Store in the array
		data_[size_ - 1] = val;
	}
	
	void shrink_to_fit() {
		capacity_ = size_;
		data_ = (int*) realloc(data_, capacity_ * sizeof(int));
	}
	
	// const --> This method does not modify the object
	size_t size() const { return size_;	}
	
	int* data() { return data_; }
	
	int at(size_t pos) const {
		assert(pos < size_);
		return data_[pos];
	}
};

// We define the function to take 2 const void pointers in order to conform to the
// _CoreCrtNonSecureSearchSortCompareFunction definition, as required by qsort()
int compare_int(const void* va, const void* vb) {
	// Cast the pointers to int
	const int* a = (const int*) va;
	const int* b = (const int*) vb;

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
void sort(vector* v) {
	qsort(v->data(), v->size(), sizeof(int), compare_int);
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

	// v is a pointer to v: we use ->
	vector* v = new vector;

	int val;
	// Read a 32 bit integer (%d, as most selectors, skips whitespace)
	// and check if read was successful
	while (fscanf(fin, "%d", &val) == 1) {
		// Add to vector
		v->push_back(val);
	}

	fclose(fin);

	// Shrink to fit
	v->shrink_to_fit();

	// Sort v
	sort(v);

	// Save the output to the second file
	FILE* fout = fopen(argv[2], "w");
	if (fout == NULL) {
		return EXIT_FAILURE;
	}

	for (size_t i = 0; i < v->size(); i++) {
		fprintf(fout, "%d\n", v->at(i));
	}

	fclose(fout);
	delete v;

	return EXIT_SUCCESS;
}
