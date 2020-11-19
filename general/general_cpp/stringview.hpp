#ifndef GENERAL_STRING
#define GENERAL_STRING

#include <stdexcept>

#include <cstdlib>
#include <cstdio>
#include <cstring>

class StringView {
private:
	char *buffer;
	size_t size;

public:
	String() {
		nullify();
	}

	String(char *c_string) {
		buffer = c_string;
		size   = strlen(c_string);
	}

	~String() {
		nullify();
	}

	void nullify() {
		buffer = nullptr;
		size = 0;
	}

	bool is_null() const {
		return buffer == nullptr;
	}

	size_t length() const {
		return size;
	}

	char &operator[](size_t i) const {
		if (i > size) {
			throw std::length_error("[ERR]<string>: out of length indexing");
		} else {
			return buffer[i];
		}
	}

	int read(char *c_string, const bool stop_on_space = true, const char ending_char = '\0') {
		if (c_string == nullptr) {
			return -1;
		}

		const char *c = c_string;
		size_t length = 0;
		while (*c && (!stop_on_space || !isspace(*c)) && *c != ending_char) {
			++c;
			++length;
		}

		buffer = c_string;
		size = length;

		return length;
	}

	void print(FILE *file_ptr = stdout, const int sidx = -1, const int eidx = -1) const {
		if (is_null()) {
			return;
		}
		if (sidx < 0 && eidx < 0) {
			fprintf(file_ptr, "%s", buffer);
		} else {
			for (int i = sidx; !(i >= (int) size || (eidx >= 0 && i < eidx)); ++i) {
				fprintf(file_ptr, "%c", buffer[i]);
			}
		}
	}

	char *get_buffer() {
		return buffer;
	}

	bool operator==(const String &other) const {
		if (length() != other.length()) {
			return false;
		}

		int len = length();
		for (int i = 0; i < len; ++i) {
			if ((*this)[i] != other[i]) {
				return false;
			}
		}

		return true;
	}
};

#endif // GENERAL_STRING