#include "value.hpp"
#include "string.hpp"

namespace sting {

value::value() {
    type = vtype::NIL;
    f = 0;
}

value::value(f32 f) {
    type = vtype::NUMBER;
    this->f = f;
}

value::value(u8 b) {
    type = vtype::BOOLEAN;
    this->b = b;
}

value::value(object const* o, vtype t) {
    type = t;
    this->o = o->clone();
}

value::~value() {
    // shouldn't we free this->o
    // if (type == vtype::STRING) {
    //
    // }
}

value value::operator+(const value& other) const {
    check_type(*this, other);
    if (this->type != vtype::NUMBER && this->type != vtype::STRING) {
        panic("Type error: cannot add type");
    }

    if (this->type == vtype::NUMBER) {
        return value(static_cast<f32>(this->number() + other.number()));
    } else if (this->type == vtype::STRING) {
        const string b = *static_cast<string*>(this->obj());
        const string a = *static_cast<string*>(other.obj());
        string c = a + b; // TODO: remove copy
        return value(static_cast<object*>(&c), vtype::STRING);
    } else {
        panic("Type error: unknown type.");
    }
    return value();
}

value value::operator-(const value& other) const {
    check_type(*this, other);
    if (this->type != vtype::NUMBER || other.type != vtype::NUMBER) {
        panic("Type error: cannot subtract non-number type");
    }
    return value(static_cast<f32>(this->number() - other.number()));
}

value value::operator*(const value& other) const {
    check_type(*this, other);
    if (this->type != vtype::NUMBER || other.type != vtype::NUMBER) {
        panic("Type error: cannot multiply non-number type");
    }
    return value(static_cast<f32>(this->number() * other.number()));
}

value value::operator/(const value& other) const {
    check_type(*this, other);
    if (this->type != vtype::NUMBER || other.type != vtype::NUMBER) {
        panic("Type error: cannot divide non-number type");
    }
    return value(static_cast<f32>(this->number() / other.number()));
}

value value::operator!() const {
    if (this->type != vtype::BOOLEAN) {
        panic("Type error: cannot logical-not non-boolean type");
    }
    return value(static_cast<u8>(this->b ^ 0x1));
}

value value::operator-() const {
    if (this->type != vtype::NUMBER) {
        panic("Type error: cannot negate non-number type");
    }
    return value(static_cast<f32>(-this->f));
}

value value::operator==(const value& other) const {
    check_type(*this, other);

    switch(this->type) {
        case vtype::NIL: {
            return value(static_cast<u8>(true));
        }
        case vtype::BOOLEAN: {
            return value(static_cast<u8>(this->b == other.b));
        }
        case vtype::NUMBER: {
            return value(static_cast<u8>(this->f == other.f));
        }
        case vtype::STRING: {
            string* b = static_cast<string*>(this->o);
            string* a = static_cast<string*>(other.o);
            return value(static_cast<u8>(*a == *b));
        }
        default:
            panic("Type error: cannot compare this type");
    }

    return value();
}

value value::operator>(const value& other) const {
    check_type(*this, other);
    if (this->type == vtype::NIL) {
        panic("Type error: > not supported for nil type");
    } else if (this->type == vtype::BOOLEAN) {
        return value(static_cast<u8>(this->b > other.b));
    }
    return value(static_cast<u8>(this->f > other.f));
}

value value::operator<(const value& other) const {
    check_type(*this, other);
    if (this->type == vtype::NIL) {
        panic("Type error: > not supported for nil type");
    } else if (this->type == vtype::BOOLEAN) {
        return value(static_cast<u8>(this->b < other.b));
    }
    return value(static_cast<u8>(this->f < other.f));
}

std::ostream& operator<<(std::ostream& os, const value& v) {
    switch (v.type) {
        case vtype::BOOLEAN: {
            os << (v.b ? "true" : "false");
            break;
        }
        case vtype::NIL: {
            os << "nil";
            break;
        }
        case vtype::NUMBER: {
            os << v.f;
            break;
        }
        case vtype::STRING: {
            u8* s = v.o->cstr();
            os << s;
            free(s);
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
