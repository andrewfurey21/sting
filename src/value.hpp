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
    CLOSURE,
};

// TODO: shouldn't this be an object too? for gc.
// or could implement custom new.
class value : public object {
public:
    value();
    value(f32 f);
    value(u8 b);
    value(object const* o, vtype t);
    // TODO: implement copy+move.
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

    u8 * cstr() const override { return strdup("<value>"); }
    object * clone () const override {
        object * v = new value(*this);
        object_list.push_back(v);
        return v;
    }

    vtype type;

    friend void check_type(const value& a, const value& b);
    friend std::ostream& operator<<(std::ostream& os, const value& v);
private:
    union {
        f32 f;
        u8 b;
        object* o; // shared pointer?
    };
};


} // namespace sting

#endif
