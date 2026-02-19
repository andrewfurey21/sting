#include "closure.hpp"
#include "object.hpp"

namespace sting {

rtupvalue::rtupvalue() : _data(nullptr) {}
rtupvalue::rtupvalue(value * const v) : _data(v) {}
rtupvalue::rtupvalue(const rtupvalue& other) : _data(other._data) {}
rtupvalue::rtupvalue(rtupvalue&& other) : _data(exchange(other._data, nullptr)) {}

rtupvalue& rtupvalue::operator=(const rtupvalue& other) {
    if (this != &other) {
        _data = other._data;
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
    return strdup("<upvalue>");
}

std::ostream& operator<<(std::ostream& os, const rtupvalue& u) {
    os << *(u._data);
    return os;
}

rtupvalue *rtupvalue::new_upvalue(value * const v) {
    rtupvalue* uv = new rtupvalue(v);
    object_list.push_back(uv);
    return uv;
}

closure::closure() : f(), _upvalues() {}
closure::closure(const function& f) : f(f), _upvalues() {}
closure::closure(const closure& other) : f(other.f), _upvalues(other._upvalues) {}
closure::closure(closure&& other) : f(stealable(other.f)), _upvalues(stealable(other._upvalues)) {}

closure& closure::operator=(const closure& other) {
    if (this != &other) {
        f = other.f;
        _upvalues = other._upvalues;
    }
    return *this;
}

closure& closure::operator=(closure&& other) {
    if (this != &other) {
        f = stealable(other.f);
        _upvalues = stealable(other._upvalues);
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

} // namespace sting
