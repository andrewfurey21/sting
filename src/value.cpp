#include "../include/value.hpp"

namespace sting {

value::value() {
    type = NIL;
    f = 0;
}

value::value(f32 f) {
    type = NUMBER;
    this->f = f;
}

value::value(u8 b) {
    type = BOOLEAN;
    this->b = b;
}

value value::operator+(const value& other) const {
    check_type(*this, other);
    if (this->type != NUMBER || other.type != NUMBER) {
        panic("Type error: cannot add non-number type");
    }
    return value(static_cast<f32>(this->number() + other.number()));
}

value value::operator-(const value& other) const {
    check_type(*this, other);
    if (this->type != NUMBER || other.type != NUMBER) {
        panic("Type error: cannot subtract non-number type");
    }
    return value(static_cast<f32>(this->number() - other.number()));
}

value value::operator*(const value& other) const {
    check_type(*this, other);
    if (this->type != NUMBER || other.type != NUMBER) {
        panic("Type error: cannot multiply non-number type");
    }
    return value(static_cast<f32>(this->number() * other.number()));
}

value value::operator/(const value& other) const {
    check_type(*this, other);
    if (this->type != NUMBER || other.type != NUMBER) {
        panic("Type error: cannot divide non-number type");
    }
    return value(static_cast<f32>(this->number() / other.number()));
}

value value::operator!() const {
    if (this->type != BOOLEAN) {
        panic("Type error: cannot logical not non-boolean type");
    }
    return value(static_cast<u8>(this->b ^ 0x1));
}

value value::operator==(const value& other) const {
    check_type(*this, other);
    if (this->type == NIL) {
        return value(static_cast<u8>(true));
    } else if (this->type == BOOLEAN) {
        return value(static_cast<u8>(this->b == other.b));
    } else {
        return value(static_cast<f32>(this->f == other.f));
    }
}

value value::operator>(const value& other) const {
    check_type(*this, other);
    if (this->type == NIL) {
        panic("Type error: > not supported for nil type");
    } else if (this->type == BOOLEAN) {
        return value(static_cast<u8>(this->b > other.b));
    }
    return value(static_cast<f32>(this->f > other.f));
}

std::ostream& operator<<(std::ostream& os, const value& v) {
    switch (v.type) {
        case value::BOOLEAN: {
            os << (v.b ? "true" : "false");
            break;
        }
        case value::NIL: {
            os << "nil";
            break;
        }
        case value::NUMBER: {
            os << v.f;
            break;
        }

        default:
            panic("Unknown value type");
    }

    return os;
}

void check_type(const value &a, const value &b) {
    panic_if(a.type != b.type, "Type Error", -1);
}

} // namespace sting
