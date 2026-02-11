#ifndef VMACHINE_HPP
#define VMACHINE_HPP

#include "object.hpp"
#include "utilities.hpp"
#include "dynarray.hpp"
#include "value.hpp"
#include "hashmap.hpp"
#include "string.hpp"
#include "chunk.hpp"
#include "function.hpp"
#include "native_function.hpp"

namespace sting {

enum class vm_result { // just result?
    OK,
    COMPILE_ERROR,
    RUNTIME_ERROR
};

struct call_frame {
    function f;
    u64 pc;
    u64 bp; // base pointer of function call on value_stack
    // bp is the first value not accessible by the function call.

    call_frame(const function& func, u64 bp = 0) : f(func), bp(bp), pc(0) {}
    call_frame(const call_frame& other) : f(other.f), bp(other.bp), pc(other.pc) {}
};

struct vmachine {
    vmachine(const function& f) {
        call_frame cf = call_frame(f);
        call_frames.push_back(cf);
    }

    void call(const value& callable, const vtype& type, const u64 num_args) {
        switch (type) {
            case vtype::FUNCTION: {
                function f = *static_cast<function*>(callable.obj());
                panic_if(f.get_arity() != num_args, "Wrong number of args to function call");
                call_frame frame(f, value_stack.size() - f.get_arity());
                call_frames.push_back(frame);
                break;
            }
            case vtype::NATIVE_FUNCTION: {
                // no return, so have to fix the stack here.
                // pop off args
                native_function nf = *static_cast<native_function*>(callable.obj());
                panic_if(nf.get_arity() != num_args, "Wrong number of args to native function call");
                dynarray<value> args;
                for (u64 i = 0; i < num_args; i++) {
                    args.push_back(value_stack.pop_back()); // reverse order
                }
                value_stack.push_back(nf.call(args));
                break;
            }
            default: {
                panic("Cannot call");
            }
        }
    }

    vm_result run_chunk() {
        for (;;) {
            u64 *const pc = &call_frames.back().pc;
            instruction const& current = call_frames.back().f.get_chunk().bytecode.at(*pc);
            call_frames.back().pc++;

            switch(current.op) {
                case opcode::RETURN: {
                    if (call_frames.size() == 1) {
                        std::cout << "stack size: " << value_stack.size() << "\n";
                        for (u64 i = 0; i < value_stack.size(); i++) {
                            std::cout << i << ": " << value_stack.at(i) << "\n";
                        }
                        call_frames.pop_back();
                        return vm_result::OK;
                    }
                    // save the returned value, delete all locals + params made by the call.
                    const value v = value_stack.pop_back();
                    for (u64 i = value_stack.size(); i > call_frames.back().bp; i--) {
                        value_stack.pop_back();
                    }
                    value_stack.push_back(v);
                    call_frames.pop_back();
                    break;
                }

                case opcode::CALL: {
                    // value_stack: arg1, arg2, arg3, fn, {}
                    // fn gets popped before execution.
                    const u64 num_args = current.operands.at(0);
                    const value& callable = value_stack.pop_back();
                    const vtype type = callable.type;
                    call(callable, type, num_args);
                    break;
                }

                case opcode::LOAD_CONST: {
                    u64 index = current.operands.at(0);
                    const value& val = get_current_chunk().constant_pool.at(index);
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
                    u32 num = current.operands.at(0);
                    for (u32 i{}; i < num; i++) {
                        value_stack.pop_back();
                    }
                    break;
                }

                case opcode::DEFINE_GLOBAL: {
                    u32 index = current.operands.at(0);
                    const value& v = get_current_chunk().constant_pool.at(index);
                    const string *name = static_cast<string*>(v.obj());
                    // this looks really bad but guaranteed to be a string.
                    panic_if(globals.contains(*name), "Already defined global");
                    globals.insert(*name, value_stack.pop_back());
                    break;
                }

                case opcode::GET_GLOBAL: {
                    u32 index = current.operands.at(0);

                    const value& v = get_current_chunk().constant_pool.at(index);
                    const string *name = static_cast<string*>(v.obj());

                    panic_if(!globals.contains(*name), "Cannot get undefined global");

                    value_stack.push_back(globals.at(*name));
                    break;
                }

                case opcode::SET_GLOBAL: {
                    u32 index = current.operands.at(0);
                    const value& v = get_current_chunk().constant_pool.at(index);
                    const string *name =static_cast<string*>(v.obj());
                    panic_if(!globals.contains(*name), "Cannot set undefined global");

                    globals.at(*name) = value_stack.back();

                    break;
                }

                case opcode::GET_LOCAL: {
                    u32 index = current.operands.at(0) + call_frames.back().bp;
                    value_stack.push_back(value_stack.at(index));
                    break;
                }

                case opcode::SET_LOCAL: {
                    u32 index = current.operands.at(0) + call_frames.back().bp;
                    value_stack.at(index) = value_stack.back();
                    break;
                }

                case opcode::BRANCH_FALSE: {
                    u32 increment = current.operands.at(0);
                    if (!value_stack.back().byte()) {
                        *pc += increment;
                    }
                    break;
                }

                case opcode::BRANCH: {
                    u32 increment = current.operands.at(0);
                    *pc += increment;
                    break;
                }

                case opcode::LOOP: {
                    u32 increment = current.operands.at(0);
                    *pc -= increment;
                    break;
                }

                default: {
                    std::stringstream errMessage;
                    errMessage << "Unknown opcode: " << static_cast<u64>(current.op);
                    panic(errMessage.str());
                }
            }
        }
    }

    chunk& get_current_chunk() { return call_frames.back().f.get_chunk(); }

    const chunk& script() {
        return call_frames.at(0).f.get_chunk();
    }

    dynarray<call_frame> call_frames;
    dynarray<value> value_stack;
    hashmap<string, value> globals; // builtins get stored here too?
};

} // namespace sting

#endif
