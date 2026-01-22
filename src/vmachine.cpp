#include "vmachine.hpp"

namespace sting {

std::string opcode_to_string(opcode op) {
    switch (op) {
        case opcode::RETURN:
            return "RETURN";
        case opcode::LOAD_CONST:
            return "CONST";
        case opcode::NEGATE:
            return "NEGATE";
        case opcode::NOT:
            return "NOT";
        case opcode::ADD:
            return "ADD";
        case opcode::MULTIPLY:
            return "MULTIPLY";
        case opcode::DIVIDE:
            return "DIVIDE";
        case opcode::SUBTRACT:
            return "SUBTRACT";
        case opcode::TRUE:
            return "TRUE";
        case opcode::FALSE:
            return "FALSE";
        case opcode::NIL:
            return "NIL";
        case opcode::GREATER:
            return "GREATER";
        case opcode::LESS:
            return "LESS";
        case opcode::EQUAL:
            return "EQUAL";
        case opcode::PRINT:
            return "PRINT";
        case opcode::POP:
            return "POP";
        case opcode::POPN:
            return "POP N";
        case opcode::DEFINE_GLOBAL:
            return "DEFINE GLOBAL";
        case opcode::GET_GLOBAL:
            return "GET GLOBAL";
        case opcode::SET_GLOBAL:
            return "SET GLOBAL";
        case opcode::GET_LOCAL:
            return "GET LOCAL";
        case opcode::SET_LOCAL:
            return "SET LOCAL";
        case opcode::BRANCH_FALSE:
            return "BRANCH (if false)";
        case opcode::BRANCH:
            return "BRANCH";
        case opcode::LOOP:
            return "LOOP";
        case opcode::CALL:
            return "CALL";
        default:
            return "WARNING: UNKNOWN OPCODE";
    }
}

std::ostream& operator<<(std::ostream& os, const instruction& instr) {
    os << opcode_to_string(instr.op) << ": " << instr.a;
    return os;
}

std::ostream& operator<<(std::ostream& os, const chunk& chk) {
    os << "---- CHUNK: " << chk.name << " ---- \n";
    for (u64 i{0}; i < chk.bytecode.size(); i++) {
        os << std::setw(3) << std::setfill(' ') << i + 1 << ": ";
        const instruction& instr = chk.bytecode.at(i);
        os << instr << "\t";
        switch (instr.op) {
            case opcode::LOAD_CONST: {
                f32 data = chk.constant_pool.at(instr.a).number();
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
