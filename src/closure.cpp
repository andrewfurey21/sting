#include "closure.hpp"
#include <cstring>
#include <string>

namespace sting {

closure::closure() : f() {}
closure::closure(const function& f) : f(f) {}
closure::closure(const closure& other) : f(other.f) {}
closure::closure(closure&& other) : f(stealable(other.f)) {}

closure& closure::operator=(const closure& other) {
    if (this != &other) {
        f = other.f;
    }
    return *this;
}

closure& closure::operator=(closure&& other) {
    if (this != &other) {
        f = stealable(other.f);
    }
    return *this;
}

object *closure::clone() const {
    object *c = new closure(*this);
    object_list.push_back(c);
    return c;
}

u8 *closure::cstr() const {
    return f.cstr();
}

std::ostream& operator<<(std::ostream& os, const closure& c) {
    os << c.f;
    return os;
}

rtupvalue::rtupvalue() : _data(nullptr) {}
rtupvalue::rtupvalue(const rtupvalue& other) : _data(other._data) {}
rtupvalue::rtupvalue(rtupvalue&& other) : _data(exchange(other._data, nullptr)) {}

rtupvalue& rtupvalue::operator=(const rtupvalue& other) {
    if (this != &other) {
        _data = other._data; // guess we need rc
    }
    return *this;
}

rtupvalue& rtupvalue::operator=(rtupvalue&& other) {
    if (this != &other) {
        _data = exchange(other._data, nullptr);
    }
    return *this;
}

rtupvalue::~rtupvalue() {
    // TODO: may have to destroy stuff. may need to impl shared_ptr.
    // maybe not though. just add *_data to object_list. dont free _data.
}

object *rtupvalue::clone() const {
    object *c = new rtupvalue(*this);
    object_list.push_back(c);
    return c;
}

u8 *rtupvalue::cstr() const {
    if (_data == nullptr) {
        return strdup("<VALUE::NULLPTR>");
    }

    const value v = *_data;
    switch (v.type) {
        case vtype::BOOLEAN: {
            return v.byte() ? strdup("true") : strdup("false");
        }
        case vtype::NIL: {
            return strdup("NIL");
        }
        case vtype::NUMBER: {
            return strdup(std::to_string(v.number()).data());
        }
        case vtype::STRING:
        case vtype::NATIVE_FUNCTION:
        case vtype::FUNCTION:
        case vtype::CLOSURE: {
            return v.obj()->cstr();
        }
        default:
            panic("Unknown vtype in upvalue");
    }
}

std::ostream& operator<<(std::ostream& os, const rtupvalue& u) {
    os << *(u._data);
    return os;
}

} // namespace sting
