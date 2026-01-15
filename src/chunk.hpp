#ifndef CHUNK_HPP
#define CHUNK_HPP

#include "utilities.hpp"
#include "dynarray.hpp"
#include "value.hpp"

namespace sting {

enum class opcode {
    RETURN,
    LOAD_CONST,
    NEGATE,
    NOT,
    ADD,
    MULTIPLY,
    DIVIDE,
    SUBTRACT,
    TRUE,
    FALSE,
    NIL,
    GREATER,
    LESS,
    EQUAL,
    PRINT,
    POP,
    POPN,
    DEFINE_GLOBAL,
    GET_GLOBAL,
    SET_GLOBAL,
    GET_LOCAL,
    SET_LOCAL,
    BRANCH_FALSE,
    BRANCH,
    LOOP, // just branch but backwards
};

std::string opcode_to_string(opcode op);



struct instruction {
    opcode op;
    u32 a;

    friend std::ostream& operator<<(std::ostream& os, const instruction& instr);
};

struct chunk {
    chunk() : name("unnamed_chunk") {}
    // could just use my string
    chunk(const std::string& name) : name(name) {}
    std::string name;
    dynarray<instruction> bytecode;
    dynarray<value> constant_pool;
    dynarray<u64> lines;

    void write_instruction(const opcode op, u64 line, u32 a = 0) {
        instruction instr = {
            .op = op,
            .a = a,
        };

        lines.push_back(line);
        bytecode.push_back(instr);
    }

    u32 load_constant(const value& val) {
        u32 index = constant_pool.size();
        constant_pool.push_back(val);
        return index;
    }

    friend std::ostream& operator<<(std::ostream& os, const chunk& chk);

    ~chunk() {

    }
};

} // namespace sting

#endif
