#include "../include/parser.hpp"

namespace sting {

parser::parser() : parser("unknown_chunk") {}

parser::parser(const std::string& name) :
        chk(name),
        index{},
        panic{}, parse_error{}
{
    prev = 0;
    current = 0;
}

bool parser::parse() {
    current = &tokens.at(0);
    expression();

    consume(token_type::END_OF_FILE, "Expected end of file");
    if (panic) return !parse_error;

    chk.write_instruction(opcode::RETURN, current->line);
    return true;
}

dynarray<token>& parser::get_tokens() { return tokens; }
chunk& parser::get_chunk() { return chk; }

void parser::error_at_token(const token& t, const std::string& msg) {
    if (panic) return;
    panic = true;
    parse_error = true;
    fprintf(stdout, "Error at line %ld: ", t.line);
    if (t.type == token_type::END_OF_FILE) {
        fprintf(stdout, "%s, issue with end of file.\n", msg.data());
    } else {
        fprintf(stdout, "%s, got %.*s\n", msg.data(), (int)t.length, t.start);
    }
}

void parser::check_current_token(const token_type expected, const std::string& msg) {
    if (current->type != expected) {
        error_at_token(*current, msg);
    }
}

void parser::get_next_token() {
    prev = current;
    index++;
    if (index < tokens.size())
        current = &tokens.at(index);
    // TODO: deal with error tokens.
}

void parser::consume(token_type type, const char* msg) {
    get_next_token();
    if (prev->type != type) error_at_token(*prev, msg);
}

void parser::parse_precedence(precedence p) {
    get_next_token();
    parse_fn prefix_rule = get_rule(prev->type)->prefix;

    if (prefix_rule == nullptr) {
        error_at_token(*prev, "Expected expression");
        return;
    }

    (this->*prefix_rule)();

    int pi = static_cast<int>(p);
    int ci = static_cast<int>(get_rule(current->type)->prec);
    while (pi <= ci) {
        get_next_token();
        parse_fn infix_rule = get_rule(prev->type)->infix;
        (this->*infix_rule)();
        ci = static_cast<int>(get_rule(current->type)->prec);
    }
}

void parser::expression() {
    parse_precedence(precedence::ASSIGNMENT);
}

// gets the number, emits LOAD_CONST and pushes number onto value stack
void parser::number() {
    const value val = std::stof(std::string(prev->start, prev->length));
    u32 index = chk.load_constant(val);
    chk.write_instruction(opcode::LOAD_CONST, prev->line, index);
}

void parser::literal() {
    switch(prev->type) {
        case token_type::TRUE: {
            chk.write_instruction(opcode::TRUE, 0);
            break;
        }
        case token_type::FALSE: {
            chk.write_instruction(opcode::FALSE, 0);
            break;
        }
        case token_type::NIL: {
            chk.write_instruction(opcode::NIL, 0);
            break;
        }
        default:
            return;
    }
}

void parser::str() {
    object* str = new string(prev->start, prev->length);
    value val = value(str, vtype::STRING);
    u32 index = chk.load_constant(val);
    chk.write_instruction(opcode::LOAD_CONST, prev->line, index);
}

void parser::grouping() {
    // assume ( is in previous.
    expression();
    consume(token_type::RIGHT_PAREN, "Expected ')' after expression");
}

void parser::unary() {
    token_type op_type = prev->type;
    parse_precedence(precedence::UNARY);

    switch(op_type) {
        case token_type::MINUS: {
            chk.write_instruction(opcode::NEGATE, prev->line);
            break;
        }
        case token_type::BANG: {
            chk.write_instruction(opcode::NOT, prev->line);
            break;
        }
        default:
            std::cout << "Unknown unary\n";
            return;
    }
}

void parser::binary() {
    token_type type = prev->type;
    parse_rule* rule = get_rule(type);

    // + 1 => left associativity. If +, only */ and above can be parsed, not +
    parse_precedence(static_cast<precedence>(rule->prec + 1));

    switch (type) {
        case token_type::PLUS: {
            chk.write_instruction(opcode::ADD, prev->line);
            break;
        }
        case token_type::MINUS: {
            chk.write_instruction(opcode::SUBTRACT, prev->line);
            break;
        }
        case token_type::SLASH: {
            chk.write_instruction(opcode::DIVIDE, prev->line);
            break;
        }
        case token_type::STAR: {
            chk.write_instruction(opcode::MULTIPLY, prev->line);
            break;
        }
        case token_type::EQUAL_EQUAL: {
            chk.write_instruction(opcode::EQUAL, prev->line);
            break;
        }
        case token_type::BANG_EQUAL: {
            chk.write_instruction(opcode::EQUAL, prev->line);
            chk.write_instruction(opcode::NOT, prev->line);
            break;
        }
        case token_type::GREATER: {
            chk.write_instruction(opcode::GREATER, prev->line);
            break;
        }
        case token_type::GREATER_EQUAL: {
            chk.write_instruction(opcode::LESS, prev->line);
            chk.write_instruction(opcode::NOT, prev->line);
            break;
        }
        case token_type::LESS: {
            chk.write_instruction(opcode::LESS, prev->line);
            break;
        }
        case token_type::LESS_EQUAL: {
            chk.write_instruction(opcode::GREATER, prev->line);
            chk.write_instruction(opcode::NOT, prev->line);
            break;
        }
        default:
            panic_if(true, "Unknown binary operator", -1);
    }
}
    // NONE       0
    // ASSIGNMENT 1 =
    // OR         2 or
    // AND        3 and
    // EQUALITY   4 == !=
    // COMPARISON 5 < > <= >=
    // TERM       6 + -
    // FACTOR     7 * /
    // UNARY      8 ! -
    // CALL       9 . ()
    // PRIMARY    10
parse_rule rules[] = { // order matters here, indexing with token_type
  // prefix, infix, precedence
  {&parser::grouping, nullptr, precedence::NONE}, // [LEFT PAREN]
  {nullptr,     nullptr,   precedence::NONE},   // [RIGHT_PAREN]
  {nullptr,     nullptr,   precedence::NONE},   // [LEFT_BRACE]
  {nullptr,     nullptr,   precedence::NONE},   // [RIGHT_BRACE]
  {nullptr,     nullptr,   precedence::NONE},   // [COMMA]
  {nullptr,     nullptr,   precedence::NONE},   // [DOT]
  {&parser::unary,    &parser::binary, precedence::TERM},   // [MINUS]
  {nullptr,     &parser::binary, precedence::TERM},   // [PLUS]
  {nullptr,     nullptr,   precedence::NONE},   // [SEMICOLON]
  {nullptr,     &parser::binary, precedence::FACTOR}, // [SLASH]
  {nullptr,     &parser::binary, precedence::FACTOR}, // [STAR]
  {&parser::unary,     nullptr,   precedence::NONE},   // [BANG]
  {nullptr,     &parser::binary,   precedence::EQUALITY},   // [BANG_EQUAL]
  {nullptr,     nullptr,   precedence::NONE},   // [EQUAL]
  {nullptr,     &parser::binary,   precedence::EQUALITY},   // [EQUAL_EQUAL]
  {nullptr,     &parser::binary,   precedence::COMPARISON},   // [GREATER]
  {nullptr,     &parser::binary,   precedence::COMPARISON},   // [GREATER_EQUAL]
  {nullptr,     &parser::binary,   precedence::COMPARISON},   // [LESS]
  {nullptr,     &parser::binary,   precedence::COMPARISON},   // [LESS_EQUAL]
  {nullptr,     nullptr,   precedence::NONE},   // [IDENTIFIER]
  {&parser::str,     nullptr,   precedence::NONE},   // [STRING]
  {&parser::number,   nullptr,   precedence::NONE},   // [NUMBER]
  {nullptr,     nullptr,   precedence::NONE},   // [AND]
  {nullptr,     nullptr,   precedence::NONE},   // [CLASS]
  {nullptr,     nullptr,   precedence::NONE},   // [ELSE]
  {&parser::literal,     nullptr,   precedence::NONE},   // [FALSE]
  {nullptr,     nullptr,   precedence::NONE},   // [FOR]
  {nullptr,     nullptr,   precedence::NONE},   // [FUN]
  {nullptr,     nullptr,   precedence::NONE},   // [IF]
  {&parser::literal,     nullptr,   precedence::NONE},   // [NIL]
  {nullptr,     nullptr,   precedence::NONE},   // [OR]
  {nullptr,     nullptr,   precedence::NONE},   // [PRINT]
  {nullptr,     nullptr,   precedence::NONE},   // [RETURN]
  {nullptr,     nullptr,   precedence::NONE},   // [SUPER]
  {nullptr,     nullptr,   precedence::NONE},   // [THIS]
  {&parser::literal,     nullptr,   precedence::NONE},   // [TRUE]
  {nullptr,     nullptr,   precedence::NONE},   // [VAR]
  {nullptr,     nullptr,   precedence::NONE},   // [WHILE]
  {nullptr,     nullptr,   precedence::NONE},   // [ERROR]
  {nullptr,     nullptr,   precedence::NONE},   // [EOF]
};


parse_rule* get_rule(token_type type) {
    return &rules[static_cast<u64>(type)];
}

} // namespace sting
