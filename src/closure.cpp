#include "closure.hpp"

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

} // namespace sting
