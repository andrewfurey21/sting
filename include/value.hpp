#include "utilities.hpp"
#include "object.hpp"

namespace sting {

enum class vtype {
    BOOLEAN,
    NIL,
    NUMBER,
    STRING
};

class value {
public:
    value();
    value(f32 f);
    value(u8 b);
    value(object* o, vtype t);

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
    union {
        f32 f;
        u8 b;
        object* o;
    };

    friend void check_type(const value& a, const value& b);
    friend std::ostream& operator<<(std::ostream& os, const value& v);
};


} // namespace sting
