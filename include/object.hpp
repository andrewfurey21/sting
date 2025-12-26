#include "utilities.hpp"

namespace sting {

class object {
};


// immutable string
class string : public object {
public:
    string();
    string(u64 size);
    string(const u8* other);
    string(const string& other);
    string(string&& other);
    string& operator=(const string& other);
    string& operator=(string&& other);
    ~string();

    char at (u64 index) const;
    string operator+(const string& other);
    void operator+=(const string& other);
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
