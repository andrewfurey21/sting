#ifndef NATIVE_FUNCTION_HPP
#define NATIVE_FUNCTION_HPP

#include "object.hpp"
#include "string.hpp"
#include "value.hpp"

namespace sting {

class native_function : public object {
public:
    using pfn = value (*)(const dynarray<value>& args);
    native_function();
    native_function(const string& name, u64 arity, pfn native_fn);
    native_function(const native_function& other);
    native_function(native_function&& other);
    native_function& operator=(const native_function& other);
    native_function& operator=(native_function&& other);

    u64 get_arity() { return arity; }
    value call(const dynarray<value>& args);
    object *clone() const override;
    u8 *cstr() const override;

    friend std::ostream& operator<<(std::ostream& os, const native_function& func);
private:
    string name;
    u64 arity;
    pfn native_fn;
};

// Native function definitions

value clock(const dynarray<value>& args);

} // namespace sting

#endif
