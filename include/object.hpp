#ifndef OBJECT_HPP
#define OBJECT_HPP

#include "utilities.hpp"
#include "dynarray.hpp"

namespace sting {

class object {
public:
    // we can copy derived from base pointer
    virtual object* clone() const = 0;
    virtual u8* cstr() = 0;
    virtual ~object() = default;
};

static inline dynarray<object*> object_list;

class string : public object {
public:
    string();
    string(u64 size);
    string(const u8* other);
    string(const u8* other, const u64 size);
    string(const string& other);
    string(string&& other);
    string& operator=(const string& other);
    string& operator=(string&& other);
    ~string();

    object* clone() const override;
    u8* cstr() override;

    u8 at (u64 index) const;
    bool compare(const string& other) const;
    string operator+(const string& other);
    void operator+=(const string& other);
    bool operator==(const string& other);
    bool operator!=(const string& other);
    u64 size() const { return _size; }
    u8* data() const { return _data; }

    friend std::ostream& operator<<(std::ostream& os, const string& str);

private:
    void allocate_size();
    void copy(u8* dest, const u8* src, const u64 size);
    u8* _data;
    u64 _size;
};

} // namespace sting

#endif
