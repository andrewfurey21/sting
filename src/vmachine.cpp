#include "../include/vmachine.hpp"

namespace sting {

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

// NOTE: might just have a std::array impl? (heap allocated though)
std::ostream& operator<<(std::ostream& os, const instruction& instr) {
    os << opcode_to_string(instr.op) << ": " << instr.a << ", " << instr.b << ", " << instr.c << ", ";
    return os;
}

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

} // namespace sting
