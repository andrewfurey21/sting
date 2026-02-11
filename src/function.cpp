#include "function.hpp"

namespace sting {

function::function() : function("unnamed_function_warning", 0) {}

function::function(const string& name, u64 arity) :
    name(name),
    arity(arity),
    chk(std::string(name.data(), name.size())) // hack
{
}

function::function(const function& other) :
    name(other.name),
    arity(other.arity),
    chk(other.chk)
{}

function::function(function&& other) :
    name(stealable(other.name)),
    arity(stealable(other.arity)),
    chk(stealable(other.chk))
{}

function& function::operator=(const function& other) {
    if (this != &other) {
        name = other.name;
        arity = other.arity;
        chk = other.chk;
    }
    return *this;
}

function& function::operator=(function&& other) {
    if (this != &other) {
        name = stealable(other.name);
        arity = stealable(other.arity);
        chk = stealable(other.chk);
    }
    return *this;
}

void function::write_instruction(const opcode op, u64 line, u32 a) {
    chk.write_instruction(op, line, a);
}

u32 function::load_constant(const value& val) {
    return chk.load_constant(val);
}

object *function::clone() const {
    object *func = new function(*this);
    object_list.push_back(func);
    return func;
}

u8 *function::cstr() const {
    return name.cstr();
}

std::ostream& operator<<(std::ostream& os, const function& func) {
    os << func.name;
    return os;
}

} // namespace sting
