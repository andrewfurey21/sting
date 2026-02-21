#ifndef CLOSURE_HPP
#define CLOSURE_HPP

#include "object.hpp"
#include "function.hpp"

namespace sting {

// rtupvalues are placed on heap. so no good to call constructors directly, just use new_rtupvalue
class rtupvalue: public object {
public:
    rtupvalue();
    rtupvalue(const u64 v);
    rtupvalue(rtupvalue * next);
    rtupvalue(const rtupvalue& other);
    rtupvalue(rtupvalue&& other);
    rtupvalue& operator=(const rtupvalue& other);
    rtupvalue& operator=(rtupvalue&& other);
    ~rtupvalue();

    object *clone() const override;
    u8 *cstr() const override;
    friend std::ostream& operator<<(std::ostream& os, const rtupvalue& c);
    static rtupvalue *new_upvalue(const u64 v);
    u64 value_stack_index() const { return _value_stack_index; }
    rtupvalue * next() const { return _next; }
    rtupvalue *& next() { return _next; } // TODO: this compiles? how different signature?
    value closed;
    bool is_closed = false;
private:
    u64 _value_stack_index;
    rtupvalue * _next;
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
