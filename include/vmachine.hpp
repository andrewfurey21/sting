#ifndef VMACHINE_HPP
#define VMACHINE_HPP

#include "object.hpp"
#include "utilities.hpp"
#include "dynarray.hpp"
#include "value.hpp"
#include "hashmap.hpp"

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
    u32 b; // TODO: remove
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

    ~chunk() {

    }
};

enum class vm_result { // just result?
    OK,
    COMPILE_ERROR,
    RUNTIME_ERROR
};

struct vmachine {
    vmachine(const chunk& chk) : chk(chk) {
        pc = this->chk.bytecode.data();
    }

    vm_result run_chunk() {
        for (;;) {
            const instruction& current = *pc;
            pc++;

            switch(current.op) {
                case opcode::RETURN: {
                    std::cout << "stack size at end: " << value_stack.size() << "\n" << std::flush;
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
                    value_stack.push_back(-a);
                    break;
                }

                case opcode::NOT: {
                    value a = value_stack.pop_back();
                    value_stack.push_back(!a);
                    break;
                }

                case opcode::ADD: {
                    const value b = value_stack.pop_back();
                    const value a = value_stack.pop_back();
                    const value c = b + a;
                    value_stack.push_back(c);
                    break;
                }

                case opcode::MULTIPLY: {
                    const value b = value_stack.pop_back();
                    const value a = value_stack.pop_back();
                    const value c = a * b;
                    value_stack.push_back(c);
                    break;
                }

                case opcode::DIVIDE: {
                    const value b = value_stack.pop_back();
                    const value a = value_stack.pop_back();
                    const value c = a / b;
                    value_stack.push_back(c);
                    break;
                }

                case opcode::SUBTRACT: {
                    const value b = value_stack.pop_back();
                    const value a = value_stack.pop_back();
                    const value c = a - b;
                    value_stack.push_back(c);
                    break;
                }

                case opcode::TRUE: {
                    const value t = value(static_cast<u8>(true));
                    value_stack.push_back(t);
                    break;
                }

                case opcode::FALSE: {
                    const value t = value(static_cast<u8>(false));
                    value_stack.push_back(t);
                    break;
                }

                case opcode::NIL: {
                    value_stack.push_back(value());
                    break;
                }

                case opcode::EQUAL: {
                    const value b = value_stack.pop_back();
                    const value a = value_stack.pop_back();
                    value_stack.push_back(a == b);
                    break;
                }

                case opcode::GREATER: {
                    const value b = value_stack.pop_back();
                    const value a = value_stack.pop_back();
                    value_stack.push_back(a > b);
                    break;
                }

                case opcode::LESS: {
                    const value b = value_stack.pop_back();
                    const value a = value_stack.pop_back();
                    value_stack.push_back(a < b);
                    break;
                }

                case opcode::PRINT: {
                    std::cout << value_stack.pop_back() << "\n" << std::flush;
                    break;
                }

                case opcode::POP: {
                    value_stack.pop_back();
                    break;
                }

                case opcode::POPN: {
                    u32 num = current.a;
                    for (u32 i{}; i < num; i++) {
                        value_stack.pop_back();
                    }
                    break;
                }

                case opcode::DEFINE_GLOBAL: {
                    u32 index = current.a;
                    const value& v = chk.constant_pool.at(index);
                    const string* name = static_cast<string*>(v.obj());
                    // this looks really bad but guaranteed to be a string.
                    panic_if(globals.contains(*name), "Already defined global");
                    globals.insert(*name, value_stack.back());
                    value_stack.pop_back();

                    break;
                }

                case opcode::GET_GLOBAL: {
                    u32 index = current.a;

                    const value& v = chk.constant_pool.at(index);
                    const string* name = static_cast<string*>(v.obj());

                    panic_if(!globals.contains(*name), "Cannot get undefined variable");

                    value_stack.push_back(globals.at(*name));
                    break;
                }

                case opcode::SET_GLOBAL: {
                    u32 index = current.a;
                    const value& v = chk.constant_pool.at(index);
                    const string* name = static_cast<string*>(v.obj());
                    panic_if(!globals.contains(*name), "Cannot set undefined variable");

                    globals.at(*name) = value_stack.back();

                    break;
                }

                case opcode::GET_LOCAL: {
                    u32 index = current.a;
                    value_stack.push_back(value_stack.at(index));
                    break;
                }

                case opcode::SET_LOCAL: {
                    u32 index = current.a;
                    value_stack.at(index) = value_stack.back();
                    break;
                }

                case opcode::BRANCH_FALSE: {
                    u32 increment = current.a;
                    if (!value_stack.back().byte()) {
                        pc += increment;
                    }
                    break;
                }

                case opcode::BRANCH: {
                    u32 increment = current.a;
                    pc += increment;
                    break;
                }

                case opcode::LOOP: {
                    u32 increment = current.a;
                    pc -= increment;
                    break;
                }

                default: {
                    std::stringstream errMessage;
                    errMessage << "Unknown opcode: " << static_cast<i32>(current.op);
                    panic(errMessage.str());
                }
            }
        }
    }

    chunk chk;
    instruction* pc;
    dynarray<value> value_stack;
    hashmap<string, value> globals;
};

} // namespace sting

#endif
