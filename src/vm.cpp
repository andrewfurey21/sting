#include "sting.hpp"

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

// should be std::expected
std::string opcode_to_string(opcode op) {
    switch (op) {
        case opcode::RETURN:
            return "RETURN";
        case opcode::LOAD_CONST:
            return "CONST";
        case opcode::NEGATE:
            return "NEGATE";
        case opcode::ADD:
            return "ADD";
        case opcode::MULTIPLY:
            return "MULTIPLY";
        case opcode::DIVIDE:
            return "DIVIDE";
        case opcode::SUBTRACT:
            return "SUBTRACT";
        default:
            return "UNKNOWN";
    }
}

// NOTE: could possibly be a base class whose derivatives point to memory of some size
struct value {
    f32 data;

    value operator+(const value& other) const {
        return value { .data = this->data + other.data };
    }

    value operator-(const value& other) const {
        return value { .data = this->data - other.data };
    }

    value operator*(const value& other) const {
        return value { .data = this->data * other.data };
    }

    value operator/(const value& other) const {
        return value { .data = this->data / other.data };
    }
};

// NOTE: might just have a std::array impl? (heap allocated though)
struct instruction {
    opcode op;
    u32 a;
    u32 b;
    u32 c;
};

std::ostream& operator<<(std::ostream& os, const instruction& instr) {
    os << opcode_to_string(instr.op) << ": " << instr.a << ", " << instr.b << ", " << instr.c << ", ";
    return os;
}

struct chunk {
    chunk() : name("unnamed_chunk") {}
    chunk(const std::string& name) : name(name) {}
    std::string name;
    dynamic_array<instruction> bytecode;
    dynamic_array<value> constant_pool;
    dynamic_array<u64> lines;

    void write_instruction(const opcode op, u64 line, u32 a = 0, u32 b = 0, u32 c = 0) {
        instruction instr = {
            .op = op,
            .a = a,
            .b = b,
            .c = c
        };

        // lines.size() == bytecode.size();
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

std::ostream& operator<<(std::ostream& os, const chunk& chk) {
    os << "---- CHUNK: " << chk.name << " ---- \n";
    for (u64 i{0}; i < chk.bytecode.size(); i++) {
        const instruction& instr = chk.bytecode.at(i);
        os << instr << "\t";
        switch (instr.op) {
            case opcode::LOAD_CONST:
                os << "Value(" << chk.constant_pool.at(instr.a).data << ")";
            default:
                os << "\t";
        }
        os << "\tline: " << chk.lines.at(i) << "\n";
    }
    return os;
}


enum class vm_result { // just result?
    OK,
    COMPILE_ERROR,
    RUNTIME_ERROR
};

struct virtual_machine {
    virtual_machine(const chunk& chk) : chk(chk) {
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
                        std::cerr << val.data << "\n";
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
                    a.data *= -1;
                    value_stack.push_back(a);
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
    dynamic_array<value> value_stack;
};

vm_result interpret(const std::filesystem::path& file) {
    bool result;
    std::string source = read_file(file);
    std::cout << "------- SOURCE -------\n" << source
              << "----------------------\n";
    scanner scan(source.data(), source.size());
    pratt_parser parser(file.string());
    result = scan.tokenize(parser.get_tokens());
    if (!result) return vm_result::COMPILE_ERROR;

    const dynamic_array<token>& ts = parser.get_tokens();
    int line = 0;
    for (u64 i{}; i < ts.size(); i++) {
        if (ts.at(i).type == token_type::END_OF_FILE) {
            break;
        } else if (line != ts.at(i).line) {
            line = ts.at(i).line;
            std::cout << "\n" << line << ": ";
        }
        printf("token(%.*s), ", (int)ts.at(i).length, ts.at(i).start);
    }
    std::cout << "\n";

    result = parser.parse();
    if (!result) return vm_result::COMPILE_ERROR;
    virtual_machine vm(parser.get_chunk());

    std::cout << vm.chk << "\n";
    return vm.run_chunk();
}

void manage_result(vm_result result) {
    u8 code = -1;
    switch (result) {
        case sting::vm_result::OK: {
            std::cerr << "Success.\n";
            code = 0;
            break;
        }
        case sting::vm_result::COMPILE_ERROR: {
            std::cerr << "Compiler error.\n";
            break;
        }
        case sting::vm_result::RUNTIME_ERROR: {
            std::cerr << "Runtime error.\n";
            break;
        }
        default:
            std::cerr << "Unknown interpreter result.\n";
    }
    exit(code);
}

} // namespace sting
