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

bool parser::match(token_type type) {
    if (prev->type != type) return false;
    return true;
}

bool parser::parse() {
    current = &tokens.at(0);

    while (current->type != token_type::END_OF_FILE) {
        declaration();
    }

    consume(token_type::END_OF_FILE, "Expected end of file");
    if (panic) return !parse_error;

    chk.write_instruction(opcode::RETURN, current->line);
    return true;
}

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

    bool assignable = p <= precedence::ASSIGNMENT;
    (this->*prefix_rule)(assignable);

    int pi = static_cast<int>(p);
    int ci = static_cast<int>(get_rule(current->type)->prec);
    while (pi <= ci) {
        get_next_token();
        parse_fn infix_rule = get_rule(prev->type)->infix;
        (this->*infix_rule)(assignable);
        ci = static_cast<int>(get_rule(current->type)->prec);
    }
}

void parser::declaration() {
    if (current->type == token_type::VAR) {
        var_declaration();
    } else {
        statement();
    }
}

// add local variable to list of variables in given scope.
void parser::declare_local_variable() {
    if (c.scope_depth == 0) return;

    local l = {
        .name = *prev,
        .depth = -1,
    };

    // check if local with same scope has same name
    for (u64 i = c.locals.size(); i > 0; i--) {
        const local& current_local = c.locals.at(i - 1);
        if (current_local.depth < c.scope_depth && current_local.depth != -1)
            break;

        panic_if(l == current_local, "Error: redeclaration of variable");
    }
    c.locals.push_back(l); // local == token + scope
}

u64 parser::parse_global_variable_name() {
    string name(prev->start, prev->length);
    value v(&name, vtype::STRING);
    return chk.load_constant(v);
}

void parser::var_declaration() {
    get_next_token();
    consume(token_type::IDENTIFIER, "Expected variable name");

    u64 index;
    declare_local_variable();
    if (c.scope_depth > 0) {
        index = 0;
    } else {
        index = parse_global_variable_name();
    }

    if (current->type == token_type::EQUAL) {
        get_next_token();
        expression();
        if (c.locals.size() >= 1)
            c.locals.back().depth = c.scope_depth;
    } else {
        chk.write_instruction(opcode::NIL, current->line);
    }

    if (c.scope_depth == 0) {
        chk.write_instruction(opcode::DEFINE_GLOBAL, prev->line, index);
    }
    consume(token_type::SEMICOLON, "Expected ';' after variable declaration");
}

void parser::variable(bool assignable) {
    named_variable(*prev, assignable);
}

void parser::named_variable(const token& tok_name, bool assignable) {

    if (current->type == token_type::EQUAL) {
        panic_if(!assignable, "Cannot assign to this expression");

        i64 local = c.resolve_local(*prev);
        if (local == -1) {
            u64 index = parse_global_variable_name();
            get_next_token();
            expression();
            chk.write_instruction(opcode::SET_GLOBAL, prev->line, index);
        } else {
            get_next_token();
            expression();
            chk.write_instruction(opcode::SET_LOCAL, prev->line, local);
        }
    } else {

        i64 local = c.resolve_local(*prev);
        if (local == -1) {
            u64 index = parse_global_variable_name();
            chk.write_instruction(opcode::GET_GLOBAL, prev->line, index);
        } else {
            chk.write_instruction(opcode::GET_LOCAL, prev->line, local);
        }
    }
}

void parser::statement() {
    if (current->type == token_type::PRINT) {
        print();
    } else if (current->type == token_type::IF) {
        if_statement();
    } else if (current->type == token_type::WHILE) {
        while_statement();
    } else if (current->type == token_type::FOR) {
        for_statement();
    } else if (current->type == token_type::LEFT_BRACE) {
        c.scope_depth++;
        block();

        u32 count = 0;
        while (c.locals.size() > 0 && c.locals.back().depth == c.scope_depth) {
            count++;
            c.locals.pop_back();
        }
        if (count != 0)
            chk.write_instruction(opcode::POPN, prev->line, count);
        c.scope_depth--;
    } else {
        expression_statement();
    }
}

u64 parser::emit_jump(opcode branch_type) {
    chk.write_instruction(branch_type, prev->line, 0);
    return chk.bytecode.size();
}

void parser::backpatch(u64 branch) {
    u64 current_size = chk.bytecode.size();
    chk.bytecode.at(branch - 1).a = current_size - branch;
}

void parser::while_statement() {
    get_next_token();
    consume(token_type::LEFT_PAREN, "Expected '(' after while");
    u64 start = chk.bytecode.size();
    expression();
    consume(token_type::RIGHT_PAREN, "Expected ')' after expression");

    u64 jump_if_false = emit_jump(opcode::BRANCH_FALSE);
    chk.write_instruction(opcode::POP, prev->line);

    statement();
    u64 end = chk.bytecode.size();
    chk.write_instruction(opcode::LOOP, prev->line, end - start + 1);
    backpatch(jump_if_false);
    chk.write_instruction(opcode::POP, prev->line);
}

void parser::for_statement() {
    get_next_token();
    u64 loop_line = prev->line;
    u64 start = chk.bytecode.size();
    consume(token_type::LEFT_PAREN, "Expected '(' after while");
    consume(token_type::SEMICOLON, "Expected ';' after while");
    consume(token_type::SEMICOLON, "Expected ';' after while");
    consume(token_type::RIGHT_PAREN, "Expected ')' after expression");


    statement();
    u64 end = chk.bytecode.size();
    chk.write_instruction(opcode::LOOP, loop_line, end - start + 1);
}

void parser::if_statement() {
    get_next_token();
    consume(token_type::LEFT_PAREN, "Expected '(' after if");
    expression();
    consume(token_type::RIGHT_PAREN, "Expected ')' after expression");

    u64 if_statement = emit_jump(opcode::BRANCH_FALSE);
    chk.write_instruction(opcode::POP, prev->line);
    // resolve local assumes there are no extraneous values on stack
    statement();

    if (current->type == token_type::ELSE) {
        get_next_token();
        u64 else_statement = emit_jump(opcode::BRANCH);
        backpatch(if_statement);
        chk.write_instruction(opcode::POP, prev->line);
        // need to check if else statement exists in order to backpatch properly
        statement();
        backpatch(else_statement);
    } else {
        backpatch(if_statement);
    }
}

void parser::block() {
    get_next_token();
    while (current->type != token_type::RIGHT_BRACE && current->type != token_type::END_OF_FILE) {
        declaration();
    }

    consume(token_type::RIGHT_BRACE, "Expected '}' after block");
}

void parser::expression_statement() {
    expression();
    consume(token_type::SEMICOLON, "Expected ;");
    chk.write_instruction(opcode::POP, prev->line);
}

void parser::expression() {
    parse_precedence(precedence::ASSIGNMENT);
}

// gets the number, emits LOAD_CONST and pushes number onto value stack
void parser::number(bool assignable) {
    const value val = std::stof(std::string(prev->start, prev->length));
    u32 index = chk.load_constant(val);
    chk.write_instruction(opcode::LOAD_CONST, prev->line, index);
}

void parser::literal(bool assignable) {
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

void parser::str(bool assignable) {
    object* str = new string(prev->start, prev->length);
    object_list.push_back(str);
    value val = value(str, vtype::STRING);
    u32 index = chk.load_constant(val);
    chk.write_instruction(opcode::LOAD_CONST, prev->line, index);
}

void parser::grouping(bool assignable) {
    // assume ( is in previous.
    expression();
    consume(token_type::RIGHT_PAREN, "Expected ')' after expression");
}

void parser::unary(bool assignable) {
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

void parser::binary(bool assignable) {
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
            sting::panic("Unknown binary operator");
    }
}

void parser::binary_and(bool assignable) {
    u64 _and = emit_jump(opcode::BRANCH_FALSE);
    chk.write_instruction(opcode::POP, prev->line);
    parse_precedence(precedence::AND);
    backpatch(_and);
}

void parser::binary_or(bool assignable) { // could just have  a BRANCH_TRUE.
    u64 first_not = emit_jump(opcode::BRANCH_FALSE);
    u64 first_true = emit_jump(opcode::BRANCH);
    backpatch(first_not);
    chk.write_instruction(opcode::POP, prev->line);
    parse_precedence(precedence::OR);
    backpatch(first_true);
}

void parser::print() {
    get_next_token();
    expression();
    consume(token_type::SEMICOLON, "Expected ;");
    chk.write_instruction(opcode::PRINT, prev->line);
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
  {&parser::variable,     nullptr,   precedence::NONE},   // [IDENTIFIER]
  {&parser::str,     nullptr,   precedence::NONE},   // [STRING]
  {&parser::number,   nullptr,   precedence::NONE},   // [NUMBER]
  {nullptr,     &parser::binary_and,   precedence::AND},   // [AND]
  {nullptr,     nullptr,   precedence::NONE},   // [CLASS]
  {nullptr,     nullptr,   precedence::NONE},   // [ELSE]
  {&parser::literal,     nullptr,   precedence::NONE},   // [FALSE]
  {nullptr,     nullptr,   precedence::NONE},   // [FOR]
  {nullptr,     nullptr,   precedence::NONE},   // [FUN]
  {nullptr,     nullptr,   precedence::NONE},   // [IF]
  {&parser::literal,     nullptr,   precedence::NONE},   // [NIL]
  {nullptr,     &parser::binary_or,   precedence::OR},   // [OR]
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
