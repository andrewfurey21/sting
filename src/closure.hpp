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

    object *clone() const override;
    u8 *cstr() const override;

    u64& get_arity() { return f.get_arity(); }
    chunk& get_chunk() { return f.get_chunk(); }
    friend std::ostream& operator<<(std::ostream& os, const closure& c);
private:
    function f;
};

class rtupvalue: public object {
public:
    rtupvalue();
    rtupvalue(const rtupvalue& other);
    rtupvalue(rtupvalue&& other);
    rtupvalue& operator=(const rtupvalue& other);
    rtupvalue& operator=(rtupvalue&& other);
    ~rtupvalue();

    object *clone() const override;
    u8 *cstr() const override;
    friend std::ostream& operator<<(std::ostream& os, const rtupvalue& c);
private:
    value *_data; // TODO: custom shared pointer?
};

} // namespace sting

#endif
