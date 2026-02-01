#include "native_function.hpp"
#include <ctime>

namespace sting {

native_function::native_function() : native_function("unnamed_native_function_warning", 0, nullptr) {}

native_function::native_function(const string& name, u64 arity, pfn native_fn) :
    name(name),
    arity(arity),
    native_fn(native_fn)
{
}

native_function::native_function(const native_function& other) :
    name(other.name),
    arity(other.arity),
    native_fn(other.native_fn)
{}

native_function::native_function(native_function&& other) :
    name(stealable(other.name)),
    arity(stealable(other.arity)),
    native_fn(stealable(other.native_fn))
{}

native_function& native_function::operator=(const native_function& other) {
    if (this != &other) {
        name = other.name;
        arity = other.arity;
        native_fn = other.native_fn;
    }
    return *this;
}

native_function& native_function::operator=(native_function&& other) {
    if (this != &other) {
        name = stealable(other.name);
        arity = stealable(other.arity);
        native_fn = stealable(other.native_fn);
    }
    return *this;
}

value native_function::call(const dynarray<value>& args) {
    return native_fn(args);
}

object *native_function::clone() const {
    object *func = new native_function(*this);
    object_list.push_back(func); // ew!
    return func;
}

u8 *native_function::cstr() const {
    return name.cstr();
}

std::ostream& operator<<(std::ostream& os, const native_function& func) {
    os << func.name;
    return os;
}

// Built in functions
// args are in reverse order.

value clock(const dynarray<value>& /* args */) {
    f32 ms = 1000.0f * std::clock() / CLOCKS_PER_SEC;
    return value(ms);
}

} // namespace sting
