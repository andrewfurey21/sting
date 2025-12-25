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
    if (this->type == NIL || other.type == NIL) {
        panic("Type error: cannot add nil type");
    }

    switch (this->type) {
        case NUMBER: {
            return value(static_cast<f32>(this->number() + other.number()));
        }
        case BOOLEAN: {
            return value(static_cast<u8>(this->byte() + other.byte()));
        }
        default:
            panic("Type error: adding unknown types");
    }

    return value();
}

value value::operator-(const value& other) const {
    check_type(*this, other);
    if (this->type == NIL || other.type == NIL) {
        panic("Type error: cannot subtract nil type");
    }

    switch (this->type) {
        case NUMBER: {
            return value(static_cast<f32>(this->number() - other.number()));
        }
        case BOOLEAN: {
            return value(static_cast<u8>(this->byte() - other.byte()));
        }
        default:
            panic("Type error: subtracting unknown types");
    }

    return value();
}

value value::operator*(const value& other) const {
    check_type(*this, other);
    if (this->type == NIL || other.type == NIL) {
        panic("Type error: cannot multiply nil type");
    }

    switch (this->type) {
        case NUMBER: {
            return value(static_cast<f32>(this->number() * other.number()));
        }
        case BOOLEAN: {
            return value(static_cast<u8>(this->byte() * other.byte()));
        }
        default:
            panic("Type error: multiplying unknown types");
    }

    return value();
}

value value::operator/(const value& other) const {
    check_type(*this, other);
    if (this->type == NIL || other.type == NIL) {
        panic("Type error: cannot divide nil type");
    }

    switch (this->type) {
        case NUMBER: {
            return value(static_cast<f32>(this->number() / other.number()));
        }
        case BOOLEAN: {
            return value(static_cast<u8>(this->byte() / other.byte()));
        }
        default:
            panic("Type error: dividing unknown types");
    }

    return value();
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
