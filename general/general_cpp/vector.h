#ifndef GENERAL_VECTOR
#define GENERAL_VECTOR

#include <stdexcept>

template <typename T>
class Vector {
private:
	T *buffer;
	size_t cur_size;
	size_t capacity;

	void realloc_bufer(const size_t new_capacity) {
		capacity = new_capacity;
		T* ptr = (T*) realloc(buffer, capacity * sizeof(T));
		if (!ptr) {
			throw std::length_error("[ERR]<vector>: realloc fail");
		}
		buffer= ptr;
	}

public:
	Vector() {
		capacity = 32;
		buffer = (T*) malloc(capacity * sizeof(T));
		cur_size = 0;
	}

	Vector(const size_t size_) {
		capacity = cur_size * 2;
		buffer = (T*) calloc(capacity, sizeof(T));
		cur_size = size_;
	}

	~Vector() {
		cur_size = 0;
		free(buffer);
	}

	T &operator[](const size_t i) const {
		if (i >= cur_size) {
			throw std::length_error("[ERR]<vector>: index overflow");
		}
		return buffer[i];
	}

	T &push_back(const T &val) {
		if (cur_size + 1 == capacity) {
			realloc_bufer(capacity * 2);
		}
		return buffer[cur_size++] = val;
	}

	T &pop_back() {
		if (cur_size == 0) {
			throw std::length_error("[ERR]<vector>: pop underflow");
		}
		return buffer[--cur_size];
	}

	size_t size() const {
		return cur_size;
	}

	void print_as_ints() const {
		if (!size()) {
			return;
		}

		for (size_t i = 0; i < size() - 1; ++i) {
			printf("%d ", (*this)[i]);
		}
		printf("%d", (*this)[size() - 1]);
	}
};

#endif // GENERAL_VECTOR