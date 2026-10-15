#define _CRT_SECURE_NO_WARNINGS

#include <cstdlib>
#include <cstdio>
#include <cstdbool>
#include <cassert>
#include <cstring>

struct vector {
	size_t size_;
	size_t capacity_;
	int* data_;

	// Rule of 4: We can either define only the default constructor and nothing else or all 4 of them
	// constructor, copy const, assignment const and destructor

	// Default constructor
	vector() {
		puts("vector()");
		size_ = capacity_ = 0;
		data_ = NULL;
	}
	// Copy constructor
	vector(const vector& other) {
		puts("vector(const vector& other)");
		size_ = other.size_;
		capacity_ = other.capacity_;
		data_ = (int*) malloc(capacity_ * sizeof(int));
		for (size_t i = 0; i < size_; ++i) {
			data_[i] = other.data_[i];
		}
	}
	// Assignment constructor
	vector& operator=(const vector& rhs) { // right hand side
		puts("vector& operator=(const vector& rhs)");
		// Check for self-assignment
		if (this != &rhs) {
			size_ = rhs.size_;
			capacity_ = rhs.capacity_;
			free(data_);
			data_ = (int*)malloc(capacity_ * sizeof(int));
			for (size_t i = 0; i < size_; ++i) {
				data_[i] = rhs.data_[i];
			}
		}
		// We return a reference to our object to allow for "a = b = c", etc
		return *this;
	}
	// Destructor
	// ~ --> Bloc Num then Alt + 126 (numeric keypad)
	~vector() {
		puts("~vector()");
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

// If we pass "vector v" (copy) instead of "vector& v" without defining an appropriate copy constructor for vector, the following happens:
// 1. The object is copied byte by byte by the compiler, so we get a new object (without calling constructor)
// 2. The new object is a SHALLOW COPY, so it will have the same size_, the same capacity_ and THE SAME data_ POINTER
// 3. This means that at the end of this function, when the destructor for v is called, we are effectively freeing the data_ also pointed by the original object
// 4. This causes the destructor called at the end of the main to throw an exception, because the data_ memory was already freed
bool write_to_file(const char* filename, const vector& v) {
	// Save the output to a file
	FILE* fout = fopen(filename, "w");
	if (fout == NULL) {
		return false;
	}

	for (size_t i = 0; i < v.size(); i++) {
		fprintf(fout, "%d\n", v.at(i));
	}

	fclose(fout);
	return true;
}

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
// Here we use a reference, so we treat v as itf it was an object
void sort(vector& v) {
	qsort(v.data(), v.size(), sizeof(int), compare_int);
}
// Here we use a pointer
// void sort(vector* v) {
// 	qsort(v->data(), v->size(), sizeof(int), compare_int);
// }

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

	// v is a vector, constructor called
	vector v;

	int val;
	// Read a 32 bit integer (%d, as most selectors, skips whitespace)
	// and check if read was successful
	while (fscanf(fin, "%d", &val) == 1) {
		// Add to vector
		v.push_back(val);
	}

	fclose(fin);

	// Shrink to fit
	v.shrink_to_fit();

	// Sort v
	sort(v);
	//sort(&v);

	write_to_file(argv[2], v);

	// Defining declaration + initialization
	// Equivalent to "vector x = v" and "vector x{v}". Calls the copy constructor
	vector x(v);

	// Assignment
	// Not the same! There are thing you can do in initialization but not in assignment
	x = v;

	// No need to delete v because it is not a pointer
	// v destructor called

	return EXIT_SUCCESS;
}
