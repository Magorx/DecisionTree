#include "string.h"

String::String() {
	nullify();
}

String::String(const char *c_string) {
	if (c_string == nullptr) {
		nullify();
		return;
	}

	size = strlen(c_string);
	capacity = size + 1;
	
	buffer = (char*) malloc(sizeof(char) * (size + 1));
	if (!buffer) {
		nullify();
		return;
	}

	memcpy(buffer, c_string, size);
	buffer[size] = '\0';
}

String::~String() {
	if (!is_null()) {
		free(buffer);
		nullify();
	}
}

void String::nullify(const bool to_free) {
	if (to_free) {
		free(buffer);
	}

	buffer = nullptr;
	size = 0;
	capacity = 0;
}

bool String::is_null() const {
	return buffer == nullptr;
}

size_t String::length() const {
	return size;
}

char &String::operator[](size_t i) const {
	if (i > size) {
		throw std::length_error("[ERR]<string>: out of length indexing");
	} else {
		return buffer[i];
	}
}

int String::read(const unsigned char *c_string, const bool stop_on_space, const char ending_char) {
	if (c_string == nullptr) {
		return -1;
	}

	const unsigned char *c = c_string;
	size_t length = 0;
	while (*c && (!stop_on_space || !isspace(*c)) && *c != ending_char) {
		++c;
		++length;
	}

	nullify(buffer != nullptr);

	size = length;
	capacity = length + 1;

	buffer = (char*) malloc(sizeof(char) * (size + 1));
	if (!buffer) {
		nullify();
		return -1;
	}

	memcpy(buffer, c_string, size);
	buffer[size] = '\0';

	return length;
}

void String::print(FILE *file_ptr) const {
	if (is_null()) {
		return;
	}

	if (file_ptr) {
		fprintf(file_ptr, "%s", buffer);
	} else {
		printf("%s", buffer);
	}
}

char *String::get_buffer() {
	return buffer;
}

bool String::operator==(const String &other) {
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