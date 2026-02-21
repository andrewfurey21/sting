#include "closure.hpp"
#include "object.hpp"

namespace sting {

rtupvalue::rtupvalue() : _value_stack_index(0), _next(nullptr), closed() {}
rtupvalue::rtupvalue(const u64 v) : _value_stack_index(v), _next(nullptr), closed() {}
rtupvalue::rtupvalue(rtupvalue * next) : _value_stack_index(0), _next(next), closed() {}
rtupvalue::rtupvalue(const rtupvalue& other) : _value_stack_index(other._value_stack_index), _next(other._next), closed(other.closed) {}
rtupvalue::rtupvalue(rtupvalue&& other) :
    _value_stack_index(exchange(other._value_stack_index, 0)),
    _next(exchange(other._next, nullptr)),
    closed(exchange(other.closed, value())) {}

rtupvalue& rtupvalue::operator=(const rtupvalue& other) {
    if (this != &other) {
        _value_stack_index = other._value_stack_index;
        _next = other._next;
        closed = other.closed;
    }
    return *this;
}

rtupvalue& rtupvalue::operator=(rtupvalue&& other) {
    if (this != &other) {
        _value_stack_index = exchange(other._value_stack_index, 0);
        _next = exchange(other._next, nullptr);
        closed = exchange(other.closed, value());
    }
    return *this;
}

rtupvalue::~rtupvalue() {
    // _value_stack_index or _next is not owned. should be by object_list
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
    os << u._value_stack_index;
    return os;
}

rtupvalue *rtupvalue::new_upvalue(const u64 v) {
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
