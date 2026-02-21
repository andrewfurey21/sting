#ifndef VMACHINE_HPP
#define VMACHINE_HPP

#include "object.hpp"
#include "parser.hpp"
#include "utilities.hpp"
#include "dynarray.hpp"
#include "value.hpp"
#include "hashmap.hpp"
#include "string.hpp"
#include "chunk.hpp"
#include "function.hpp"
#include "native_function.hpp"
#include "closure.hpp"

namespace sting {

enum class vm_result { // just result?
    OK,
    COMPILE_ERROR,
    RUNTIME_ERROR
};

struct call_frame {
    closure c;
    u64 pc;
    u64 bp; // base pointer of function call on value_stack
    // bp is the first value not accessible by the function call.

    call_frame(const closure& c, u64 bp = 0) : c(c), bp(bp), pc(0) {}
    call_frame(const call_frame& other) : c(other.c), bp(other.bp), pc(other.pc) {}
};

struct vmachine {
    vmachine(const function& f) : call_frames(), value_stack(), return_slot(), globals(), open_upvalues(nullptr) {
        call_frame cf = call_frame(f);
        call_frames.push_back(cf);
    }

    void call(const value& callable, const u64 num_args) {
        switch (callable.type) {
            case vtype::CLOSURE: {
                closure c = *static_cast<closure*>(callable.obj());
                panic_if(c.get_arity() != num_args, "Wrong number of args to function call");
                call_frame frame(c, value_stack.size() - c.get_arity());
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
                panic("Cannot call non-callable object.");
            }
        }
    }

    rtupvalue * capture_value(const u64 value_stack_index) {
        rtupvalue * previous = nullptr;
        rtupvalue * current = open_upvalues;
        while (current != nullptr && current->value_stack_index() > value_stack_index) {
            previous = current;
            current = current->next();
        }

        if (current != nullptr && current->value_stack_index() == value_stack_index) {
            return current;
        }

        rtupvalue * uv = rtupvalue::new_upvalue(value_stack_index);
        if (previous == nullptr) {
            open_upvalues = uv;
        } else {
            previous->next() = uv;
        }

        return uv;
    }

    vm_result run_chunk() {
        for (;;) {
            u64 *const pc = &call_frames.back().pc;
            instruction const& current = call_frames.back().c.get_chunk().bytecode.at(*pc);
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

                    // TODO: should be 1 if not script, else 0. fix this.
                    call_frames.pop_back();
                    break;
                }

                case opcode::CALL: {
                    // value_stack: arg1, arg2, arg3, fn, {}
                    // fn gets popped before execution.
                    const u64 num_args = current.operands.at(0);
                    const value& callable = value_stack.back();
                    const vtype type = callable.type;
                    call(callable, num_args);
                    break;
                }

                case opcode::MAKE_CLOSURE: {
                    const value& v = value_stack.pop_back();
                    const vtype type = v.type;
                    panic_if(type != vtype::FUNCTION, "Cannot make closure from non-function");
                    const function f = *static_cast<function*>(v.obj());
                    closure c = closure(f);
                    const u64 num_upvalues = current.operands.at(0);
                    panic_if((num_upvalues != (current.operands.size() - 1) / 2) &&
                             (current.operands.size() % 2 == 1),
                             "num_upvalues does not match number of upvalues passed to MAKE_CLOSURE");

                    dynarray<rtupvalue*>& uv = c.get_upvalues();
                    const u64 bp = call_frames.back().bp;
                    for (u64 i{}; i < num_upvalues; i++) {
                        const u32 local = current.operands.at(i * 2 + 1);
                        const u32 index = current.operands.at(i * 2 + 2);
                        if (local) {
                            // there is no upvalue for this local, so call capture_value
                            // makes a new upvalue and puts it in the current closure.
                            const u32 value_stack_index = index + bp;
                            uv.push_back(capture_value(value_stack_index));
                        } else {
                            // the current frame is guaranteed to have an upvalue pointing
                            // to the data. if it doesn't exist, the compiler or runtime is broken somewhere.
                            dynarray<rtupvalue*>& prev_uv = call_frames.back().c.get_upvalues();
                            uv.push_back(prev_uv.at(index));
                            // isn't its location in the upvalues array the index?
                        }
                    }
                    const value cv = value(static_cast<object*>(c.clone()), vtype::CLOSURE);
                    value_stack.push_back(cv);
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

                case opcode::CLOSE_VALUE: {
                    const u32 value_stack_index = value_stack.size() - 1;
                    rtupvalue * const top = open_upvalues;
                    top->is_closed = true;

                    panic_if(value_stack_index != top->value_stack_index(), "stack indicies should be identical");

                    open_upvalues = open_upvalues->next();
                    top->next() = nullptr;

                    top->closed = value_stack.pop_back();
                    break;
                }

                case opcode::DEFINE_GLOBAL: {
                    u32 index = current.operands.at(0);
                    const value& v = script().constant_pool.at(index);
                    const string *name = static_cast<string*>(v.obj());
                    // this looks really bad but guaranteed to be a string.
                    panic_if(globals.contains(*name), "Already defined global");
                    globals.insert(*name, value_stack.pop_back());
                    break;
                }

                case opcode::GET_GLOBAL: {
                    u32 index = current.operands.at(0);

                    const value& v = script().constant_pool.at(index);
                    const string *name = static_cast<string*>(v.obj());

                    panic_if(!globals.contains(*name), "Cannot get undefined global");

                    value_stack.push_back(globals.at(*name));
                    break;
                }

                case opcode::SET_GLOBAL: {
                    u32 index = current.operands.at(0);
                    const value& v = script().constant_pool.at(index);
                    const string *name =static_cast<string*>(v.obj());
                    panic_if(!globals.contains(*name), "Cannot set undefined global");

                    globals.at(*name) = value_stack.back();

                    break;
                }

                case opcode::GET_LOCAL: {
                    const u32 index = current.operands.at(0) + call_frames.back().bp;
                    value_stack.push_back(value_stack.at(index));
                    break;
                }

                case opcode::SET_LOCAL: {
                    const u32 index = current.operands.at(0) + call_frames.back().bp;
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

                case opcode::GET_UPVALUE: {
                    const u32 upvalue_index = current.operands.at(0);
                    rtupvalue const * const uv =
                        call_frames.back().c.get_upvalues().at(upvalue_index);

                    if (uv->is_closed) {
                        value_stack.push_back(uv->closed);
                    } else {
                        value_stack.push_back(value_stack.at(uv->value_stack_index()));
                    }

                    break;
                }

                case opcode::SET_UPVALUE: {
                    const u32 upvalue_index = current.operands.at(0);
                    rtupvalue * const uv =
                        call_frames.back().c.get_upvalues().at(upvalue_index);
                    if (uv->is_closed) {
                        uv->closed = value_stack.back();
                    } else {
                        value_stack.at(uv->value_stack_index()) = value_stack.back();
                    }
                }

                case opcode::SAVE_VALUE: {
                    return_slot = value_stack.pop_back();
                    break;
                }

                case opcode::LOAD_VALUE: {
                    value_stack.push_back(return_slot);
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

    chunk& get_current_chunk() { return call_frames.back().c.get_chunk(); }

    const chunk& script() {
        return call_frames.at(0).c.get_chunk();
    }

    dynarray<call_frame> call_frames;
    dynarray<value> value_stack;
    value return_slot;
    hashmap<string, value> globals; // builtins get stored here too?

    // not sorted.
    rtupvalue * open_upvalues;
};

} // namespace sting

#endif
