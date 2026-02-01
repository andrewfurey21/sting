#ifndef VALUE_HPP
#define VALUE_HPP

#include "utilities.hpp"
#include "object.hpp"

namespace sting {

enum class vtype {
    BOOLEAN,
    NIL,
    NUMBER,
    STRING,
    FUNCTION,
    NATIVE_FUNCTION,
};

class value {
public:
    value();
    value(f32 f);
    value(u8 b);
    value(object const* o, vtype t);
    ~value();

    value operator+(const value& other) const;
    value operator-(const value& other) const;
    value operator*(const value& other) const;
    value operator/(const value& other) const;
    value operator!() const;
    value operator-() const;
    value operator==(const value& other) const;
    value operator>(const value& other) const;
    value operator<(const value& other) const;

    f32 number() const { return f; }
    u8 byte() const { return b; }
    object* obj() const { return o; }

    vtype type;

private:
    union { // should really just be one (shared) object pointer.
        f32 f;
        u8 b;
        object* o; // should probably be a shared pointer
    };

    friend void check_type(const value& a, const value& b);
    friend std::ostream& operator<<(std::ostream& os, const value& v);
};


} // namespace sting

#endif
