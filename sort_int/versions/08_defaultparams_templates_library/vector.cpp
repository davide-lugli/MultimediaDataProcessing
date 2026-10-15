#define _CRT_SECURE_NO_WARNINGS

#include "vector.h"

#include <cstdlib>
#include <cstdio>

namespace mdp {

	bool write_to_file(const char* filename, const vector<int>& v) {
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

	vector<int> read_from_file(char* filename) {
		vector<int> v;

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

}
