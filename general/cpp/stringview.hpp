#ifndef GENERAL_STRINGVIEW
#define GENERAL_STRINGVIEW

#include <cstdlib>
#include <cstdio>
#include <cstring>

class StringView {
private:
	char *buffer;
	char null_char;
	size_t size;
	bool own_buffer;

public:
	StringView            (const StringView& other) = delete;
	StringView& operator= (const StringView& other) = delete;

	StringView() :
	buffer(nullptr),
	null_char('\0'),
	size(0),
	own_buffer(false)
	{}

	~StringView() {}

	void ctor() {dtor();}

	static StringView *NEW() {
		StringView *cake = (StringView*) calloc(1, sizeof(StringView));
		if (!cake) {
			return nullptr;
		}

		cake->ctor();
		return cake;
	}

	void ctor(char *c_string, bool own_buffer_ = false) {
		if (c_string == nullptr) {
			dtor();
			return;
		}

		size = strlen(c_string);
		buffer = c_string;
		own_buffer = own_buffer_;
	}

	static StringView *NEW(char *c_string, bool own_buffer_) {
		StringView *cake = (StringView*) calloc(1, sizeof(StringView));
		if (!cake) {
			return nullptr;
		}

		cake->ctor(c_string, own_buffer_);
		return cake;
	}

	void dtor() {
		if (own_buffer && buffer) {
			free(buffer);
		}

		buffer = nullptr;
		own_buffer = false;
		null_char = '\0';
		size = 0;
	}

	static void DELETE(StringView *stringview) {
		if (!stringview) {
			return;
		}

		stringview->dtor();
		free(stringview);
	}

	bool is_null() const {
		return buffer == nullptr;
	}

	size_t length() const {
		return size;
	}

	const char &operator[](size_t i) const {
		if (i >= size) {
			return null_char;
		} else {
			return buffer[i];
		}
	}

	char &break_index(size_t i) {
		if (i >= size) {
			return null_char;
		} else {
			return buffer[i];
		}
	}

	int read(char *c_string, const bool stop_on_space = true, const char ending_char = '\0') {
		if (c_string == nullptr) {
			return -1;
		}

		char *c = c_string;
		size_t length = 0;
		while (*c && (!stop_on_space || !isspace(*c)) && *c != ending_char) {
			++c;
			++length;
		}

		dtor();

		size = length;
		buffer = c_string;

		return (int) length;
	}

	void generate_length_format(char *string) {
		char format[24]; // to simulate the certain lenth of out stringview
		sprintf(format, "%%.%lus", size);

		size_t length = strlen(string);

		char *string_copy = (char*) calloc(length + 1, sizeof(char));
		memcpy(string_copy, string, length);

		sprintf(string, string_copy, format);

		free(string_copy);
	}

	void print(FILE *file_ptr = stdout, const int sidx = -1, const int eidx = -1) const {
		if (is_null()) {
			return;
		}

		if (sidx < 0 && eidx < 0) {
			char format[24]; // to simulate the certain lenth of out stringview
			sprintf(format, "%%.%lus", size);
			fprintf(file_ptr, format, buffer);
		} else {
			for (int i = sidx; !(i >= (int) size || (eidx >= 0 && i < eidx)); ++i) {
				fprintf(file_ptr, "%c", buffer[i]);
			}
		}
	}

	char *get_buffer() const {
		return buffer;
	}

	bool operator==(const StringView &other) const {
		if (length() != other.length()) {
			return false;
		}

		size_t len = length();
		for (size_t i = 0; i < len; ++i) {
			if ((*this)[i] != other[i]) {
				return false;
			}
		}

		return true;
	}
};

#endif // GENERAL_STRINGVIEW