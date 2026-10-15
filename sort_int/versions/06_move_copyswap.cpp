#define _CRT_SECURE_NO_WARNINGS

#include <cstdlib>
#include <cstdio>
#include <cstdbool>
#include <cassert>
#include <cstring>

// Forward declate to avoid compilation errors because I'm using swap before declaring it
struct vector;
void swap_vectors(vector& a, vector& b);

struct vector {
	size_t size_;
	size_t capacity_;
	int* data_;

	vector() {
		puts("vector()");
		size_ = capacity_ = 0;
		data_ = NULL;
	}
	vector(const vector& other) {
		puts("vector(const vector& other)");
		size_ = other.size_;
		capacity_ = other.capacity_;
		data_ = new int[capacity_];
		for (size_t i = 0; i < size_; ++i) {
			data_[i] = other.data_[i];
		}
	}
	// Move constructor
	// By using r-value reference, we specify that this constructor is to be called only if we're
	// sure that the other object will not be used anymore
	vector(vector&& other) { // r-value reference
		puts("vector(vector&& other)");
		size_ = other.size_;
		capacity_ = other.capacity_;
		data_ = other.data_;
		other.data_ = nullptr; // I steal other.data_
	}
	// Assign operator with copy and swap idiom
	// It removes the need for the move assignment operator as well!
	vector& operator=(vector rhs) { // right hand side
		puts("vector& operator=(vector rhs)");
		swap_vectors(*this, rhs);
		return *this;
	}
	~vector() {
		puts("~vector()");
		delete[] data_;
	}

	void push_back(int val) {
		size_++;
		if (size_ > capacity_) {
			capacity_ = capacity_ * 2 + 1;
			// C++ way of doing realloc
			int* tmp = new int[capacity_];
			for (size_t i = 0; i < size_ - 1; ++i) {
				tmp[i] = data_[i];
			}
			delete[] data_;
			data_ = tmp;
		}

		data_[size_ - 1] = val;
	}
	
	void shrink_to_fit() {
		capacity_ = size_;
		// C++ way of doing realloc
		int* tmp = new int[capacity_];
		for (size_t i = 0; i < size_; ++i) {
			tmp[i] = data_[i];
		}
		delete[] data_;
		data_ = tmp;
	}
	
	bool empty() const {
		return size_ == 0;
	}

	size_t size() const { return size_;	}
	
	int* data() { return data_; }
	
	int at(size_t pos) const {
		assert(pos < size_);
		return data_[pos];
	}

};

bool write_to_file(const char* filename, const vector& v) {
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

vector read_from_file(char* filename) {
	vector v;

	FILE* fin = fopen(filename, "r");
	if (fin != NULL) {
		int val;
		while (fscanf(fin, "%d", &val) == 1) {
			v.push_back(val);
		}

		v.shrink_to_fit();
		fclose(fin);
	}
	
	return v;
}

// We define the function to take 2 const void pointers in order to conform to the
// _CoreCrtNonSecureSearchSortCompareFunction definition, as required by qsort()
int compare_int(const void* va, const void* vb) {
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
}
void sort(vector& v) {
	qsort(v.data(), v.size(), sizeof(int), compare_int);
}
// Here we use a pointer
// void sort(vector* v) {
// 	qsort(v->data(), v->size(), sizeof(int), compare_int);
// }

void swap_vectors(vector& a, vector& b) {
	size_t tmp_size = a.size_;
	a.size_ = b.size_;
	b.size_ = tmp_size;
	size_t tmp_capacity = a.capacity_;
	a.capacity_ = b.capacity_;
	b.capacity_ = tmp_capacity;
	int* tmp_data = a.data_;
	a.data_ = b.data_;
	b.data_ = tmp_data;
}

int main(int argc, char* argv[]) {
	if (argc != 3) {
		return EXIT_FAILURE;
	}

	vector v;
	v = read_from_file(argv[1]); // Return Value Optimization
	if (v.empty()) {
		return EXIT_FAILURE;
	}

	sort(v);
	
	write_to_file(argv[2], v);

	return EXIT_SUCCESS;
}
