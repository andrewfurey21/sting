#ifndef CHUNK_HPP
#define CHUNK_HPP

// #include "function.hpp"
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
    CALL,
};

std::string opcode_to_string(opcode op);

// when rewriting, needs to be a stream of bytes.
struct instruction {
    opcode op;
    dynarray<u32> operands;

    friend std::ostream& operator<<(std::ostream& os, const instruction& instr);
};

struct chunk {
    chunk() : name("unnamed_chunk") {}
    // could just use my string
    chunk(const std::string& name) : name(name) {}
    std::string name; // should be sting::string
    dynarray<instruction> bytecode;
    // globals (functions, strs, global names)
    dynarray<value> constant_pool;
    dynarray<u64> lines;

    void write_instruction(const opcode op, u64 line, u32 a = 0) {
        instruction instr = {
            .op = op,
            .operands = dynarray<u32>({ a }),
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

    ~chunk() {}
};

inline std::ostream& operator<<(std::ostream& os, const instruction& instr) {
    os << opcode_to_string(instr.op) << ": " << instr.operands;
    return os;
}

inline std::ostream& operator<<(std::ostream& os, const chunk& chk) {
    os << "---- CHUNK: " << chk.name << " ---- \n";
    for (u64 i{0}; i < chk.bytecode.size(); i++) {
        os << std::setw(3) << std::setfill(' ') << i + 1 << ": ";
        const instruction& instr = chk.bytecode.at(i);
        os << instr << "\t";
        switch (instr.op) {
            case opcode::LOAD_CONST: {
                const value& data = chk.constant_pool.at(instr.operands.at(0));
                os << "Value(" << data << ")";
                os << "\t";
            }
            default: {
            }
        }
        os << "\tline: " << chk.lines.at(i) << "\n";
    }
    return os;
}

} // namespace sting

#endif
