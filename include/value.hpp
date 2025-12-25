#include "utilities.hpp"

namespace sting {

class value {
public:
    value();
    value(f32 f);
    value(u8 b);

    value operator+(const value& other) const;
    value operator-(const value& other) const;
    value operator*(const value& other) const;
    value operator/(const value& other) const;

    f32 number() const { return f; }
    u8 byte() const { return b; }
private:
    enum {
        BOOLEAN,
        NIL,
        NUMBER,
    } type;

    union {
        f32 f;
        u8 b;
    };

    friend void check_type(const value& a, const value& b);
    friend std::ostream& operator<<(std::ostream& os, const value& v);
};


} // namespace sting
