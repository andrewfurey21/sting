#ifndef CLOSURE_HPP
#define CLOSURE_HPP

#include "object.hpp"
#include "function.hpp"

namespace sting {

class closure : public object {
public:
    closure();
    closure(const function& f);
    closure(const closure& other);
    closure(closure&& other);
    closure& operator=(const closure& other);
    closure& operator=(closure&& other);

    function& get_function() { return f; }
    object *clone() const override;
    u8 *cstr() const override;

    friend std::ostream& operator<<(std::ostream& os, const closure& c);
private:
    function f;
};

} // namespace sting

#endif
