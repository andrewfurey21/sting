#ifndef VMACHINE_HPP
#define VMACHINE_HPP

#include "utilities.hpp"
#include "dynarray.hpp"
#include "value.hpp"

namespace sting {

enum class opcode {
    RETURN,
    LOAD_CONST,
    NEGATE,
    ADD,
    MULTIPLY,
    DIVIDE,
    SUBTRACT,
};

std::string opcode_to_string(opcode op);

struct instruction {
    opcode op;
    u32 a;
    u32 b;
    u32 c;

    friend std::ostream& operator<<(std::ostream& os, const instruction& instr);
};

struct chunk {
    chunk() : name("unnamed_chunk") {}
    chunk(const std::string& name) : name(name) {}
    std::string name;
    dynarray<instruction> bytecode;
    dynarray<value> constant_pool;
    dynarray<u64> lines;

    void write_instruction(const opcode op, u64 line, u32 a = 0, u32 b = 0, u32 c = 0) {
        instruction instr = {
            .op = op,
            .a = a,
            .b = b,
            .c = c
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
};

enum class vm_result { // just result?
    OK,
    COMPILE_ERROR,
    RUNTIME_ERROR
};

struct vmachine {
    vmachine(const chunk& chk) : chk(chk) {
        pc = chk.bytecode.data();
    }

    vm_result run_chunk() {
        for (;;) {
            const instruction& current = *pc;
            pc++;

            switch(current.op) {
                case opcode::RETURN: {
                    if (value_stack.size() > 0) {
                        const value& val = value_stack.at(value_stack.size() - 1);
                        std::cerr << val.number() << "\n";
                    }
                    return vm_result::OK;
                }

                case opcode::LOAD_CONST: {
                    u64 index = current.a;
                    const value& val = chk.constant_pool.at(index);
                    value_stack.push_back(val);
                    break;
                }

                case opcode::NEGATE: {
                    value a = value_stack.pop_back();
                    value b = a * -1.0f;
                    value_stack.push_back(b);
                    break;
                }

                case opcode::ADD: {
                    const value a = value_stack.pop_back();
                    const value b = value_stack.pop_back();
                    const value c = a + b;
                    value_stack.push_back(c);
                    break;
                }

                case opcode::MULTIPLY: {
                    const value a = value_stack.pop_back();
                    const value b = value_stack.pop_back();
                    const value c = a * b;
                    value_stack.push_back(c);
                    break;
                }

                case opcode::DIVIDE: {
                    const value a = value_stack.pop_back();
                    const value b = value_stack.pop_back();
                    const value c = b / a;
                    value_stack.push_back(c);
                    break;
                }

                case opcode::SUBTRACT: {
                    const value a = value_stack.pop_back();
                    const value b = value_stack.pop_back();
                    const value c = b - a;
                    value_stack.push_back(c);
                    break;
                }


                default: {
                    std::stringstream errMessage;
                    errMessage << "Unknown opcode: " << static_cast<i32>(current.op);
                    panic_if(true, errMessage.str(), -1);
                }
            }
        }
    }

    chunk chk;
    instruction* pc;
    dynarray<value> value_stack;
};

} // namespace sting

#endif
