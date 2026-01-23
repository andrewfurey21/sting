#ifndef FUNCTION_HPP
#define FUNCTION_HPP

#include "object.hpp"
#include "string.hpp"
#include "chunk.hpp"

namespace sting {

class function : public object {
public:
    function();
    function(const string& name, u64 arity);
    function(const function& other);
    function(function&& other);
    function& operator=(const function& other);
    function& operator=(function&& other);

    chunk& get_chunk() { return chk; }
    u64 get_arity() { return arity; }
    void write_instruction(const opcode op, u64 line, u32 a = 0);
    u32 load_constant(const value& val);
    object *clone() const override;
    u8 *cstr() const override;

    friend std::ostream& operator<<(std::ostream& os, const function& func);
private:
    string name;
    u64 arity;
    chunk chk;
};

} // namespace sting

#endif
