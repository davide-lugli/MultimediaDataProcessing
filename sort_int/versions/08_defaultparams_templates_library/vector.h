#pragma once

#include <cassert>
#include <utility>
#include <algorithm>

namespace mdp {

	template<typename T>
	class vector {
	private:
		size_t size_;
		size_t capacity_;
		T* data_;

		void resize_(size_t new_size, size_t new_capacity = 0, bool force_capacity = false) {
			if (new_capacity == 0 || new_capacity < new_size) {
				new_capacity = new_size;
			}

			if (new_size > capacity_ || force_capacity) {
				capacity_ = new_size;
				// auto is fine here: the type is already spelled out in the new-expression
				auto* tmp = new T[capacity_];
				std::copy(data_, data_ + size_, tmp);
				delete[] data_;
				data_ = tmp;
			}
			size_ = new_size;
		}
	public:
		// New constructor so we can do "mdp::vector a(3);" (also default constructor)
		vector(size_t size = 0) : size_(size), capacity_(size) {
			data_ = new T[capacity_];
		}
		vector(const vector& other) : size_(other.size_), capacity_(other.capacity_) {
			data_ = new T[capacity_];
			std::copy(other.data_, other.data_ + size_, data_);
		}
		vector(vector&& other) noexcept {
			size_ = other.size_;
			capacity_ = other.capacity_;
			data_ = other.data_;
			other.data_ = nullptr;
		}
		vector& operator=(vector rhs) {
			swap(*this, rhs);
			return *this;
		}
		~vector() {
			delete[] data_;
		}

		void resize(size_t new_size) {
			resize_(new_size);
		}

		void push_back(const T& val) {
			resize_(size_ + 1, capacity_ * 2 + 1);
			data_[size_ - 1] = val;
		}

		void shrink_to_fit() {
			resize_(size_, size_, true);
		}

		bool empty() const {
			return size_ == 0;
		}

		size_t size() const { return size_; }

		auto* data() { return data_; }

		const auto& at(size_t pos) const {
			assert(pos < size_);
			return data_[pos];
		}
		auto& at(size_t pos) {
			assert(pos < size_);
			return data_[pos];
		}

		void set(size_t pos, T value) {
			assert(pos < size_);
			data_[pos] = value;
		}

		const auto& operator[](size_t pos) const {
			return data_[pos];
		}
		auto& operator[](size_t pos) {
			return data_[pos];
		}

		friend void swap(vector& a, vector& b) {
			using std::swap;
			swap(a.size_, b.size_);
			swap(a.capacity_, b.capacity_);
			swap(a.data_, b.data_);
		}
	};

	// You can't put function definitions in the header file!
	// Because both main.cpp and vector.cpp include this header file, when the linker
	// will put them all together and you'll have the same things defined multiple times
	// ... unless it is a template!
	
	template<typename T>
	// static --> Not exported, so not available to the linker to provide to other files
	static int compare(const void* va, const void* vb) {
		const auto* a = (const T*)va;
		const auto* b = (const T*)vb;

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
	template<typename T>
	void sort(vector<T>& v) {
		qsort(v.data(), v.size(), sizeof(T), compare<T>);
	}

	bool write_to_file(const char* filename, const vector<int>& v);

	vector<int> read_from_file(char* filename);

}
