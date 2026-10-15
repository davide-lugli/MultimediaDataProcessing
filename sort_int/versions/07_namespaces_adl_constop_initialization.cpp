#define _CRT_SECURE_NO_WARNINGS

#include <cstdlib>
#include <cstdio>
#include <cassert>
#include <utility>
#include <algorithm>
#ifdef _MSC_VER
#include <crtdbg.h>
#endif

namespace mdp {

	// class --> same as struct, but we're using C++ now so no reason to keep using struct
	// The difference is that class requires methods to be declared as public, because by default everything is private
	class vector {
	private:
		size_t size_; // Initialized first
		size_t capacity_; // Initialized second
		int* data_; // Initialized third
	public:
		// Every method defined inside a class is automatically suggested to be inlined to the compiler
		// This means that "bool empty()" becomes "inline bool empty()" automatically
		// inline is used to suggest to the compiler to inline the function, meaning that
		// in release mode, it will try to replace, for example:
		//		if (v.empty())
		// with
		//		if (v.size_ == 0)
		// avoiding an useless function call
 
		vector() : size_(0), capacity_(0),  data_(nullptr) {
			puts("vector()");
		}

		// We could put "data_(new int[capacity_])" here, it would work because of the order of initialization (capacity_ is initialized before data_)
		// but for the sake of clarity and simplicity it's just better not to
		vector(const vector& other) : size_(other.size_), capacity_(other.capacity_) {
			puts("vector(const vector& other)");
			data_ = new int[capacity_];
			// Using std copy, optimized
			std::copy(other.data_, other.data_ + size_, data_);
		}
		vector(vector&& other) {
			puts("vector(vector&& other)");
			size_ = other.size_;
			capacity_ = other.capacity_;
			data_ = other.data_;
			other.data_ = nullptr;
		}
		vector& operator=(vector rhs) {
			puts("vector& operator=(vector rhs)");
			swap(*this, rhs);
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
				std::copy(data_, data_ + size_ - 1, tmp);
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

		void set(size_t pos, int value) {
			assert(pos < size_);
			data_[pos] = value;
		}

		// If we don't return a reference it does not work as a setter but only getter!
		int& operator[](size_t pos) {
			return data_[pos];
		}
		// This is the const version that works only as a getter
		// The first const tells us that we can't do v[i] = something
		const int& operator[](size_t pos) const {
			return data_[pos];
		}

		// friend allows the function to use private methods of the class
		// it also allows us to avoid declaring the function before the class!
		// Note that the name of the function is still "swap" and NOT "vector::swap", so it can be used outside of the class
		friend void swap(vector& a, vector& b) {
			using std::swap;
			swap(a.size_, b.size_);
			swap(a.capacity_, b.capacity_);
			swap(a.data_, b.data_);
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

}

double mean(const mdp::vector& v) {
	double sum = 0.0;
	for (size_t i = 0; i < v.size(); ++i) {
		sum += v[i];
	}
	return sum / v.size();
}

void double_all_elements(mdp::vector& v) {
	for (size_t i = 0; i < v.size(); ++i) {
		v[i] *= 2;
	}
}

struct integer_printer {
	int& value_;

	// : value_(p) is used to INITIALIZE value_ to p, remember that INITIALIZATION != ASSIGNMENT (which we did in the vector class)
	integer_printer(int& p) : value_(p) {
		// You can then do other things (such as ASSIGNMENTS, different from initializations) here
	}

	void print() {
		printf("%d", value_);
	}
};

int main(int argc, char* argv[]) {
	
	// Declaration/definition of x and y and initialization
	int x = 5;
	int y = 2;
	// Expression statement (an expression followed by ;) --> the expression has 1 operator --> the assignment operator
	x = 6;

	// Why are those two different? Here's a practical example

	// Here we're defining a reference to an int, which gets automatically dereferenced to point to x during the initialization
	int& r = x; // int *r = &x;
	// Here we are instead changing the value of r (which points to x, so the value of x as well) to be equal to y
	r = y; // *r = y;
	// And here again we are changing the value of r (so x as well) to 3
	r = 3;
	// The final result is x = 3, y = 2 !!!!!
	
	// THERE IS NO WAY TO ASSIGN A NEW POSITION TO A REFERENCE
	// It is done during initalization (mandatory) and then never again
	// Meaning that "int &r;" can't be done (no initialization)

	// But what if we have a class with a reference inside, and we want to initialize it?
	// Look at the integer_printer struct
	int q = 5;
	integer_printer t(q);
	t.print();

	// After learning this, we also changed the default and copy constructors of mdp::vector to initialize instead of assign

	{
		if (argc != 3) {
			return EXIT_FAILURE;
		}

		mdp::vector v;
		v = mdp::read_from_file(argv[1]); // Return Value Optimization
		if (v.empty()) {
			return EXIT_FAILURE;
		}

		double d = mean(v);
		double_all_elements(v);
		d = mean(v);

		// Works without namespace because of ADL - Argument Dependent Lookup
		// Since your passing an object from the mdp namespace to the function, it looks for the
		// function also in the mdp namespace by default
		sort(v);

		// Same as above
		write_to_file(argv[2], v);
	}
#ifdef _MSC_VER
	_CrtDumpMemoryLeaks();
#endif

	return EXIT_SUCCESS;
}
