#define _CRT_SECURE_NO_WARNINGS

#include <vector>
#include <algorithm>
#include <fstream>
#include <cstring>
#include <cstdlib>
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

	FILE* fin = fopen(filename, "r");
	// Defaults to text mode, for binary mode --> std::ifstream is(filename, std::ios::binary);
	std::ifstream is(filename);
	
	// Equivalent to if(is.good())
	if (is) {
		int val;
		
		// Take input from the stream and convert it to the type of val
		// NOT to be used for binary!
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

	for (size_t i = 0; i < v.size(); i++) {
		os << v[i] << '\n';
	}

	// Omitted os.close() for the same reason as before
	return true;
}

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

		std::vector<int> v = read_from_file(argv[1]); // Return Value Optimization
		if (v.empty()) {
			return EXIT_FAILURE;
		}

		std::sort(v.begin(), v.end());

		write_to_file(argv[2], v);
	}
#ifdef _MSC_VER
	_CrtDumpMemoryLeaks();
#endif

	return EXIT_SUCCESS;
}
