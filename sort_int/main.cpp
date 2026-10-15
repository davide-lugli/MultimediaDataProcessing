#define _CRT_SECURE_NO_WARNINGS

#include <vector>
#include <algorithm>
#include <fstream>
#include <cstring>
#include <cstdlib>
#include <iterator>
#ifdef _MSC_VER
#include <crtdbg.h>
#endif

struct person {
	char name[100];
	int age;

	const bool operator>(const person& rhs) const {
		//return age > rhs.age;
		return strcmp(name, rhs.name) > 0;
	}
	const bool operator<(const person& rhs) const {
		//return age < rhs.age;
		return strcmp(name, rhs.name) < 0;
	}
	const bool operator==(const person& rhs) const {
		//return age == rhs.age;
		return strcmp(name, rhs.name) == 0;
	}
};

std::vector<int> read_from_file(char* filename) {
	std::vector<int> v;

	// Defaults to text mode, for binary mode --> std::ifstream is(filename, std::ios::binary);
	std::ifstream is(filename);
	
	// Equivalent to if(is.good())
	if (is) {
		int val;
		
		// Take input from the stream and convert it to the type of val
		// NOT to be used for binary file mode!
		while (is >> val) {
			v.push_back(val);
		}

		v.shrink_to_fit();

		// Not needed here because the destructor gets automatically called when is goes out of scope
		is.close();
	}

	return v;
}

bool write_to_file(const char* filename, const std::vector<int>& v) {
	std::ofstream os(filename);

	if (!os) {
		return false;
	}

	// While doing v[i] on a vector has a complexity O(1)
	// doing it on a list (linked list) has a complexity O(n)
	// It is a bad idea to traverse a list in order by using [], it's way better
	// to traverse it from beginning to end using an increasing pointer
	// To generalize traversing data for different data structures, iterators were created

	/* Vector version which is ok, but not possible or too complex for other containers
	for (size_t i = 0; i < v.size(); i++) {
		os << v[i] << '\n';
	}*/

	/* This is the new way of doing things with iterators
	for (auto it = v.begin(); it < v.end(); ++it) {
		const auto& x = *it; // Optional, just to have an alias
		os << x << '\n';
	}*/

	// No way of getting the index from the iterator!
	// That was so standard that people decided to shorten it even more:
	for (const auto& x : v) { // Works also with "const auto", "auto&", "auto" ...
		os << x << '\n';
	}

	/* Short way
	copy(v.begin(), v.end(), std::ostream_iterator<int>(os, "\n"));
	*/

	// Omitted os.close() for the same reason as before
	return true;
}

// This is a function object or functor
struct push_into_vector {
	std::vector<int>& v_;

	push_into_vector(std::vector<int>& v) : v_(v) {}

	void operator() (int x) {
		v_.push_back(x);
	}
};

int main(int argc, char* argv[]) {
	
	{
		std::vector<person> people;
		people.push_back({ "Davide", 24 });
		people.push_back({ "Baby", 2 });
		people.push_back({ "OldMan", 92 });

		std::sort(people.begin(), people.end());

		if (argc != 3) {
			return EXIT_FAILURE;
		}

		//std::vector<int> v = read_from_file(argv[1]);

		std::vector<int> v;
		// std::vector has lots of constructors, type "std::vector<int> v(" and hover with mouse
		// You can also construct from iterators like "std::vector<int> v(it_start, it_end)"

		// Let's see 4 different ways of reading ints from file without using our function

		{
			// Different way to read from file
			std::ifstream is(argv[1]);
			std::istream_iterator<int> it_start = is;
			std::istream_iterator<int> it_stop;

			// Lambda functions, they create a "closure" which is the information used by it
			// The [] are the "name" of the function
			// If we leave them empty the function doesn't know anything about the external world
			// If we put & then it gets references to all available elements outside
			// If we put &v only the reference to v
			std::for_each(it_start, it_stop, [&](int x) { v.push_back(x); });
		}

		{
			// Copying version
			std::ifstream is(argv[1]);
			std::istream_iterator<int> it_start = is;
			std::istream_iterator<int> it_stop;
			std::copy(it_start, it_stop, std::back_inserter(v));
		}

		{
			std::ifstream is(argv[1]);
			std::istream_iterator<int> it_start = is;
			std::istream_iterator<int> it_stop;

			// Using the function object
			push_into_vector pivo(v);
			pivo(3);
			pivo(4);
			pivo(5);
			// This also changes v as it's a reference to it
			// Useful for doing things like
			std::for_each(it_start, it_stop, push_into_vector(v));
		}

		{
			std::ifstream is(argv[1]);
			std::istream_iterator<int> it_start = is;
			std::istream_iterator<int> it_stop;

			// std::vector<int> v(std::istream_iterator<int>(is), std::istream_iterator<int>());
			// This does not work, it's called the Most Vexing Parsing problem
			
			// It doesn't work because: 
			int a; // int
			int* b; // pointer to int
			int c[3]; // array of 3 int
			int* d[3]; // array of 3 pointer to int
			int (*e)[3]; // pointer to array of 3 int
			int f(double k); // function with 1 parameter which returns an int (declaration)
			int g(double k); // function with 1 parameter which returns an int (declaration)
			int h(); // function without parameter which returns an int (declaration)

			// Meaning that the compiler is not sure wheter we're calling a constructor or prototyping a function
			// So we can use another initialization technique
			int i1 = 5;
			int i2(5);
			int i3{ 5 };

			// The third one, with curly braces, was introduced to solve the MVP problem
			// So this will work:
			std::vector<int> v{ std::istream_iterator<int>(is), std::istream_iterator<int>() };
		}

		std::sort(v.begin(), v.end());

		write_to_file(argv[2], v);
	}
#ifdef _MSC_VER
	_CrtDumpMemoryLeaks();
#endif

	return EXIT_SUCCESS;
}
