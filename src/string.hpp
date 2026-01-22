#ifndef STRING_HPP
#define STRING_HPP

#include "object.hpp"
#include "hash.hpp"

namespace sting {

class string : public object {
public:
    string();
    string(u64 size);
    string(const u8 *other);
    string(const u8 *other, const u64 size);
    string(const string& other);
    string(string&& other);
    string& operator=(const string& other);
    string& operator=(string&& other);
    ~string();

    object *clone() const override;
    u8* cstr() const override;

    u8 at (u64 index) const;
    bool compare(const string& other) const;
    string operator+(const string& other);
    void operator+=(const string& other);
    bool operator==(const string& other);
    bool operator!=(const string& other);
    u64 size() const { return _size; }
    u8 *data() const { return _data; } // not good that it's const.

    friend std::ostream& operator<<(std::ostream& os, const string& str);

private:
    void allocate_size();
    void copy(u8* dest, const u8* src, const u64 size) const;
    u8 *_data;
    u64 _size;
};


} // namespace sting

#endif
