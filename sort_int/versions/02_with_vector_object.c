#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <assert.h>

// "Objects" in C
typedef struct vector {
	size_t size_;
	size_t capacity_;
	int* data_;
} vector;
vector* vector_constructor(vector* this) {
	this->size_ = this->capacity_ = 0;
	this->data_ = NULL;

	// Useful to return the pointer so it can be passed to other functions
	return this;
}
vector* vector_destructor(vector* this) {
	free(this->data_);

	return this;
}
void vector_push_back(vector* this, int val) {
	// Double capacity if needed
	this->size_++;
	if (this->size_ > this->capacity_) {
		this->capacity_ = this->capacity_ * 2 + 1;
		this->data_ = realloc(this->data_, this->capacity_ * sizeof(int));
	}

	// Store in the array
	this->data_[this->size_ - 1] = val;
}
void vector_shrink_to_fit(vector* this) {
	this->capacity_ = this->size_;
	this->data_ = realloc(this->data_, this->capacity_ * sizeof(int));
}
size_t vector_size(const vector* this) {
	return this->size_;
}
int* vector_data(vector* this) {
	return this->data_;
}
int vector_at(const vector* this, size_t pos) {
	assert(pos < this->size_);
	return this->data_[pos];
}

vector* new_vector(void) {
	vector* v = malloc(sizeof(vector));
	return vector_constructor(v);
}
void delete_vector(vector* this) {
	free(vector_destructor(this));
}

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
void sort(vector* v) {
	qsort(vector_data(v), vector_size(v), sizeof(int), compare_int);
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

	vector* v = new_vector();

	int val;
	// Read a 32 bit integer (%d, as most selectors, skips whitespace)
	// and check if read was successful
	while (fscanf(fin, "%d", &val) == 1) {
		// Add to vector
		vector_push_back(v, val);
	}

	fclose(fin);

	// Shrink to fit
	vector_shrink_to_fit(v);

	// Sort v
	sort(v);

	// Save the output to the second file
	FILE* fout = fopen(argv[2], "w");
	if (fout == NULL) {
		return EXIT_FAILURE;
	}

	for (size_t i = 0; i < vector_size(v); i++) {
		fprintf(fout, "%d\n", vector_at(v, i));
	}

	fclose(fout);
	delete_vector(v);

	return EXIT_SUCCESS;
}
