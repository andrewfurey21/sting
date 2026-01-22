#include "string.hpp"

namespace sting {

void string::allocate_size() {
    free(_data);
    _data = static_cast<u8*>(malloc(_size * sizeof(u8)));
}

void string::copy(u8* dest, const u8* src, const u64 size) const {
    for (u64 i{}; i < size; i++)
        dest[i] = src[i];
}

string::string() : _data(nullptr), _size(0) {}

string::string(u64 size) : string() {
    _size = size;
    allocate_size();
}

string::string(const u8* other) : string() {
    _size = strlen(other);
    allocate_size();
    copy(_data, other, _size);
}

string::string(const u8* other, u64 size) : string(size) {
    allocate_size();
    copy(_data, other, size);
}

string::~string() {
    free(_data);
    _data = nullptr;
}

string::string(const string& other) : _size(other._size), _data(nullptr) {
    allocate_size();
    copy(_data, other._data, _size);
}

string::string(string&& other) {
    _size = exchange(other._size, 0);
    _data = exchange(other._data, nullptr);
}

string& string::operator=(const string& other) {
    if (this != &other) {
        _size = other._size;
        allocate_size();
        copy(_data, other._data, other.size());
    }
    return *this;
}

string& string::operator=(string&& other) {
    if (this != &other) {
        free(_data);
        _size = exchange(other._size, 0);
        _data = exchange(other._data, nullptr);
    }
    return *this;
}

object* string::clone() const {
    object *str = new string(*this);
    object_list.push_back(str); // for basic garbage collection
    return str;
}

u8* string::cstr() const {
    u8* ret = reinterpret_cast<char*>(calloc(_size + 1, sizeof(u8)));
    copy(ret, _data, _size);
    return ret;
}

u8 string::at(u64 index) const {
    panic_if(index >= _size, "string::at(): index out of bounds", -1);
    return _data[index];
}

string string::operator+(const string& other) {
    string concat(_size + other.size());
    copy(concat._data, _data, _size);
    copy(concat._data + _size, other.data(), other.size());
    return concat;
}

void string::operator+=(const string& other) {
    string concat(_size + other._size);
    copy(concat._data, _data, _size);
    copy(concat._data + _size, other.data(), other.size());

    _size = exchange(concat._size, _size);
    _data = exchange(concat._data, _data);
}

bool string::operator==(const string& other) {
    return this->compare(other);
}

bool string::operator!=(const string& other) {
    return !this->compare(other);
}

bool string::compare(const string& other) const {
    if (this->size() != other.size())
        return false;
    for (u64 i{}; i < this->size(); i++) {
        if (this->at(i) != other.at(i))
            return false;
    }
    return true;
}

std::ostream& operator<<(std::ostream& os, const string& str) {
    for (u64 i{}; i < str._size; i++) {
        os << str.at(i);
    }
    return os;
}

template <>
u64 fnv_1a_hash(const string& key) {
    u64 hash = DEFAULT_FNV_OFFSET;
    for (u64 i{}; i < key.size(); i++) {
        hash ^= key.at(i);
        hash *= DEFAULT_FNV_PRIME;
    }
    return hash;
}

};
