#ifndef CLOSURE_HPP
#define CLOSURE_HPP

#include "object.hpp"
#include "function.hpp"

namespace sting {

// rtupvalues are placed on heap. so no good to call constructors directly, just use new_rtupvalue
class rtupvalue: public object {
public:
    rtupvalue();
    rtupvalue(value * const v);
    rtupvalue(const rtupvalue& other);
    rtupvalue(rtupvalue&& other);
    rtupvalue& operator=(const rtupvalue& other);
    rtupvalue& operator=(rtupvalue&& other);
    ~rtupvalue();

    object *clone() const override;
    u8 *cstr() const override;
    friend std::ostream& operator<<(std::ostream& os, const rtupvalue& c);
    static rtupvalue *new_upvalue(value * const v);
    value * data() const { return _data; }
private:
    // points to somewhere on stack or object_list when the get closed
    value *_data; // NOTE: _data not owned by upvalue
    // _data is only changed one after a variable gets closed.
};

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
    dynarray<rtupvalue*>& get_upvalues() { return _upvalues; }

    friend std::ostream& operator<<(std::ostream& os, const closure& c);
private:
    function f;
    dynarray<rtupvalue*> _upvalues;
    // NOTE: upvalues not owned by closure.
};
} // namespace sting

#endif
