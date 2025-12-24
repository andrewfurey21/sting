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
    value val;
    val.data = std::stod(std::string(prev->start, prev->length));
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
        default:
            std::cout << "Unknown unary\n";
            return;
    }
}

void parser::binary() {
    token_type type = prev->type;
    parse_rule* rule = get_rule(type);
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
        default:
            break;
    }
}

parse_rule rules[] = { // order matters here, indexing with token_type
  {&parser::grouping, nullptr, precedence::NONE}, // LEFT PAREN
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
  {nullptr,     nullptr,   precedence::NONE},   // [BANG]
  {nullptr,     nullptr,   precedence::NONE},   // [BANG_EQUAL]
  {nullptr,     nullptr,   precedence::NONE},   // [EQUAL]
  {nullptr,     nullptr,   precedence::NONE},   // [EQUAL_EQUAL]
  {nullptr,     nullptr,   precedence::NONE},   // [GREATER]
  {nullptr,     nullptr,   precedence::NONE},   // [GREATER_EQUAL]
  {nullptr,     nullptr,   precedence::NONE},   // [LESS]
  {nullptr,     nullptr,   precedence::NONE},   // [LESS_EQUAL]
  {nullptr,     nullptr,   precedence::NONE},   // [IDENTIFIER]
  {nullptr,     nullptr,   precedence::NONE},   // [STRING]
  {&parser::number,   nullptr,   precedence::NONE},   // [NUMBER]
  {nullptr,     nullptr,   precedence::NONE},   // [AND]
  {nullptr,     nullptr,   precedence::NONE},   // [CLASS]
  {nullptr,     nullptr,   precedence::NONE},   // [ELSE]
  {nullptr,     nullptr,   precedence::NONE},   // [FALSE]
  {nullptr,     nullptr,   precedence::NONE},   // [FOR]
  {nullptr,     nullptr,   precedence::NONE},   // [FUN]
  {nullptr,     nullptr,   precedence::NONE},   // [IF]
  {nullptr,     nullptr,   precedence::NONE},   // [NIL]
  {nullptr,     nullptr,   precedence::NONE},   // [OR]
  {nullptr,     nullptr,   precedence::NONE},   // [PRINT]
  {nullptr,     nullptr,   precedence::NONE},   // [RETURN]
  {nullptr,     nullptr,   precedence::NONE},   // [SUPER]
  {nullptr,     nullptr,   precedence::NONE},   // [THIS]
  {nullptr,     nullptr,   precedence::NONE},   // [TRUE]
  {nullptr,     nullptr,   precedence::NONE},   // [VAR]
  {nullptr,     nullptr,   precedence::NONE},   // [WHILE]
  {nullptr,     nullptr,   precedence::NONE},   // [ERROR]
  {nullptr,     nullptr,   precedence::NONE},   // [EOF]
};


parse_rule* get_rule(token_type type) {
    return &rules[static_cast<u64>(type)];
}

} // namespace sting
