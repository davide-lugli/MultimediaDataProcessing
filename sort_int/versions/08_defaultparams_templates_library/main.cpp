#include "vector.h"

#include <string>
#include <cstdlib>
#include <cstring>
#ifdef _MSC_VER
#include <crtdbg.h>
#endif

double mean(const mdp::vector<int>& v) {
	double sum = 0.0;
	for (size_t i = 0; i < v.size(); ++i) {
		sum += v[i];
	}
	return sum / v.size();
}

void double_all_elements(mdp::vector<int>& v) {
	for (size_t i = 0; i < v.size(); ++i) {
		v[i] *= 2;
	}
}

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

int main(int argc, char* argv[]) {
	
	{
		mdp::vector<person> people;
		people.push_back({ "Davide", 24 });
		people.push_back({ "Baby", 2 });
		people.push_back({ "OldMan", 92 });

		sort(people);

		if (argc != 3) {
			return EXIT_FAILURE;
		}

		mdp::vector<int> v = mdp::read_from_file(argv[1]); // Return Value Optimization
		if (v.empty()) {
			return EXIT_FAILURE;
		}

		double d = mean(v);
		double_all_elements(v);
		d = mean(v);

		sort(v);

		write_to_file(argv[2], v);
	}
#ifdef _MSC_VER
	_CrtDumpMemoryLeaks();
#endif

	return EXIT_SUCCESS;
}
