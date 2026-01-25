#include "parser.hpp"
#include "utilities.hpp"

namespace sting {

parser::parser() : parser("unknown_chunk") {}

parser::parser(const std::string& name) :
    index{},
    panic{},
    parse_error{}
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

    get_current_function().write_instruction(opcode::RETURN, current->line);
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
        string tok(prev->start, prev->length);
        std::cout << "Error at " << tok << "\n" << std::flush;
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
    } else if (current->type == token_type::FUN) {
        fun_declaration();
    } else {
        statement();
    }
}

void parser::declare_function_param() {
    panic_if(c.scope_depth == 0, "Parameters are declared in a local scope");

    local l = {
        .name = *prev,
        .depth = c.scope_depth,
    };

    // check if parameter name already exists
    for (u64 i = c.locals.size(); i > 0; i--) {
        const local& current_local = c.locals.at(i - 1);
        if (c.scope_depth > current_local.depth) break;
        // if (c.scope_depth != current_local.depth) break;
        panic_if(current_local.name == l.name, "Cannot have parameters with the same name.");
    }

    c.locals.push_back(l);
}

void parser::fun_declaration() {
    panic_if(c.scope_depth != 0, "CLOSURES NOT IMPLEMENTED");

    get_next_token();
    consume(token_type::IDENTIFIER, "Expected function name");

    // store function name in script constant pool
    u64 name_index = parse_global_variable_name();
    u64 fn_line = prev->line;
    c.scope_depth++;
    u64 arity = 0; // for error checking.
    consume(token_type::LEFT_PAREN, "Expected '(' after function name");
    while (current->type == token_type::IDENTIFIER) {
        arity++;
        get_next_token(); // param token in prev.
        declare_function_param();
        if (current->type == token_type::RIGHT_PAREN)
            break;

        consume(token_type::COMMA, "Expected ',' after parameter definition");
    }
    consume(token_type::RIGHT_PAREN, "Expected ')' after function definition");

    const string& fname =
        *static_cast<string*>(get_current_function().get_chunk().constant_pool.at(name_index).obj());

    consume(token_type::LEFT_BRACE, "Expected '{' after function declaration");

    c.functions.push_back(function(fname, arity));
    block();

    consume(token_type::RIGHT_BRACE, "Expected '}' after function definition");

    const dynarray<instruction>& bc = c.functions.back().get_chunk().bytecode;
    if (bc.size() == 0 || bc.back().op != opcode::RETURN) {
        c.functions.back().write_instruction(opcode::NIL, fn_line);
        c.functions.back().write_instruction(opcode::RETURN, fn_line);
    }

    // we cant pop off parameters statically atm because result will be on top of args.
    // return opcode handles this using the base pointer.

    // TODO: needs to be its own helper function, anytime scope_depth--;
    while (c.locals.size() > 0 && c.locals.back().depth == c.scope_depth) {
        c.locals.pop_back();
    }

    c.scope_depth--;
    // at end, store function in previous functions constant pool
    const function& f = c.functions.pop_back();
    const value fv(static_cast<object const*>(&f), vtype::FUNCTION);
    // load const push function onto stack here.
    u64 findex = get_current_function().load_constant(fv);
    get_current_function().write_instruction(opcode::LOAD_CONST, fn_line, findex);
    // add function name to global hashtable at runtime.
    get_current_function().write_instruction(opcode::DEFINE_GLOBAL, prev->line, name_index);
    // define global pops the value stack.
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

// this is kinda weird, multiple copies of global names in constant pool. idk.
u64 parser::parse_global_variable_name() {
    string name(prev->start, prev->length);
    value v(&name, vtype::STRING);
    return get_current_function().load_constant(v);
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
        get_current_function().write_instruction(opcode::NIL, current->line);
    }

    if (c.scope_depth == 0) {
        get_current_function().write_instruction(opcode::DEFINE_GLOBAL, prev->line, index);
    }
    consume(token_type::SEMICOLON, "Expected ';' after variable declaration");
}

void parser::variable(bool assignable) {
    named_variable(*prev, assignable);
}

// includes function calls
void parser::named_variable(const token& tok_name, bool assignable) {
    if (current->type == token_type::EQUAL) {
        panic_if(!assignable, "Cannot assign to this expression");

        i64 local = c.resolve_local(*prev);
        if (local == -1) {
            u64 index = parse_global_variable_name();
            get_next_token();
            expression();
            get_current_function().write_instruction(opcode::SET_GLOBAL, prev->line, index);
        } else {
            get_next_token();
            expression();
            get_current_function().write_instruction(opcode::SET_LOCAL, prev->line, local);
        }
    } else if (current->type == token_type::LEFT_PAREN) {
        // attempting function call
        // only checking globals, because closures not implemented yet.
        u64 index = parse_global_variable_name();
        u64 fnline = current->line;
        get_next_token();
        while (true) {
            if (current->type == token_type::RIGHT_PAREN) {
                break;
            }

            expression();
            if (current->type == token_type::RIGHT_PAREN) {
                break;
            }
            consume(token_type::COMMA, "Expected comma after function parameter expression");
        }
        // Get global function.
        consume(token_type::RIGHT_PAREN, "Expected ')' to end a function call.");
        get_current_function().write_instruction(opcode::GET_GLOBAL, fnline, index);
        get_current_function().write_instruction(opcode::CALL, fnline);
        // call pops the function object off the stack.
    } else {
        i64 local = c.resolve_local(*prev);
        if (local == -1) {
            u64 index = parse_global_variable_name();
            get_current_function().write_instruction(opcode::GET_GLOBAL, prev->line, index);
        } else {
            get_current_function().write_instruction(opcode::GET_LOCAL, prev->line, local);
        }
    }
}

void parser::return_statement() {
    get_next_token();
    if (current->type != token_type::SEMICOLON) {
        expression();
    } else {
        get_current_function().write_instruction(opcode::NIL, prev->line);
    }
    consume(token_type::SEMICOLON, "Expected ';' after return expression");
    get_current_function().write_instruction(opcode::RETURN, prev->line);
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
        consume(token_type::LEFT_BRACE, "Expected '{' after block");
        block();
        consume(token_type::RIGHT_BRACE, "Expected '}' after block");

        u32 count = 0;
        while (c.locals.size() > 0 && c.locals.back().depth == c.scope_depth) {
            count++;
            c.locals.pop_back();
        }
        if (count == 1)
            get_current_function().write_instruction(opcode::POP, prev->line);
        else if (count > 1)
            get_current_function().write_instruction(opcode::POPN, prev->line, count);

        c.scope_depth--;
    } else if (current->type == token_type::RETURN) {
        return_statement();
    } else {
        expression_statement();
    }
}

u64 parser::emit_jump(opcode branch_type) {
    get_current_function().write_instruction(branch_type, prev->line, 0);
    return get_current_function().get_chunk().bytecode.size();
}

void parser::backpatch(u64 branch) {
    u64 current_size = get_current_function().get_chunk().bytecode.size();
    get_current_function().get_chunk().bytecode.at(branch - 1).a = current_size - branch;
}

void parser::while_statement() {
    get_next_token();
    consume(token_type::LEFT_PAREN, "Expected '(' after while");
    u64 start = get_current_function().get_chunk().bytecode.size();
    expression();
    consume(token_type::RIGHT_PAREN, "Expected ')' after expression");

    u64 jump_if_false = emit_jump(opcode::BRANCH_FALSE);
    get_current_function().write_instruction(opcode::POP, prev->line);

    statement();
    u64 end = get_current_function().get_chunk().bytecode.size();
    get_current_function().write_instruction(opcode::LOOP, prev->line, end - start + 1);
    backpatch(jump_if_false);
    get_current_function().write_instruction(opcode::POP, prev->line);
}

void parser::for_statement() {
    // this isn't great, the variable will be in a different scope than the block
    c.scope_depth++;
    get_next_token();
    consume(token_type::LEFT_PAREN, "Expected '(' after for");

    // must have a var declaration.
    var_declaration(); // var i = 0; a

    u64 start = get_current_function().get_chunk().bytecode.size(); // check
    expression(); // i < size; b
    consume(token_type::SEMICOLON, "Expected ';' after expression");

    u64 end_for_loop = emit_jump(opcode::BRANCH_FALSE);
    get_current_function().write_instruction(opcode::POP, prev->line);

    u64 to_statement = emit_jump(opcode::BRANCH);
    u64 to_inc = get_current_function().get_chunk().bytecode.size();
    expression(); // i++, expression_statement expects a ;
    get_current_function().write_instruction(opcode::POP, prev->line);
    consume(token_type::RIGHT_PAREN, "Expected ')' after for loop statement");

    get_current_function().write_instruction(opcode::LOOP, prev->line, get_current_function().get_chunk().bytecode.size() - start + 1);

    backpatch(to_statement);
    statement();

    get_current_function().write_instruction(opcode::LOOP, prev->line, get_current_function().get_chunk().bytecode.size() - to_inc + 1);

    // end_for_loop
    backpatch(end_for_loop);
    get_current_function().write_instruction(opcode::POPN, prev->line, 2); // truth value + var i

    while (c.locals.size() > 0 && c.locals.back().depth == c.scope_depth) {
        c.locals.pop_back();
    }

    c.scope_depth--;
}

void parser::if_statement() {
    get_next_token();
    consume(token_type::LEFT_PAREN, "Expected '(' after if");
    expression();
    consume(token_type::RIGHT_PAREN, "Expected ')' after expression");

    u64 if_statement = emit_jump(opcode::BRANCH_FALSE);
    statement();

    if (current->type == token_type::ELSE) {
        get_next_token();
        u64 else_statement = emit_jump(opcode::BRANCH);
        backpatch(if_statement);
        statement();
        backpatch(else_statement);
    } else {
        backpatch(if_statement);
    }
    get_current_function().write_instruction(opcode::POP, prev->line);
}

void parser::block() {
    while (current->type != token_type::RIGHT_BRACE && current->type != token_type::END_OF_FILE) {
        declaration();
    }
}

void parser::expression_statement() {
    expression();
    consume(token_type::SEMICOLON, "Expected ;");
    get_current_function().write_instruction(opcode::POP, prev->line);
}

// should be able to escape quickly if token is just a semicolon.
// for ;; doesn't work etc.
void parser::expression() {
    parse_precedence(precedence::ASSIGNMENT);
}

// gets the number, emits LOAD_CONST and pushes number onto value stack
void parser::number(bool assignable) {
    const value val = std::stof(std::string(prev->start, prev->length));
    u32 index = get_current_function().load_constant(val);
    get_current_function().write_instruction(opcode::LOAD_CONST, prev->line, index);
}

void parser::literal(bool assignable) {
    switch(prev->type) {
        case token_type::TRUE: {
            get_current_function().write_instruction(opcode::TRUE, 0);
            break;
        }
        case token_type::FALSE: {
            get_current_function().write_instruction(opcode::FALSE, 0);
            break;
        }
        case token_type::NIL: {
            get_current_function().write_instruction(opcode::NIL, 0);
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
    u32 index = get_current_function().load_constant(val);
    get_current_function().write_instruction(opcode::LOAD_CONST, prev->line, index);
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
            get_current_function().write_instruction(opcode::NEGATE, prev->line);
            break;
        }
        case token_type::BANG: {
            get_current_function().write_instruction(opcode::NOT, prev->line);
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
            get_current_function().write_instruction(opcode::ADD, prev->line);
            break;
        }
        case token_type::MINUS: {
            get_current_function().write_instruction(opcode::SUBTRACT, prev->line);
            break;
        }
        case token_type::SLASH: {
            get_current_function().write_instruction(opcode::DIVIDE, prev->line);
            break;
        }
        case token_type::STAR: {
            get_current_function().write_instruction(opcode::MULTIPLY, prev->line);
            break;
        }
        case token_type::EQUAL_EQUAL: {
            get_current_function().write_instruction(opcode::EQUAL, prev->line);
            break;
        }
        case token_type::BANG_EQUAL: {
            get_current_function().write_instruction(opcode::EQUAL, prev->line);
            get_current_function().write_instruction(opcode::NOT, prev->line);
            break;
        }
        case token_type::GREATER: {
            get_current_function().write_instruction(opcode::GREATER, prev->line);
            break;
        }
        case token_type::GREATER_EQUAL: {
            get_current_function().write_instruction(opcode::LESS, prev->line);
            get_current_function().write_instruction(opcode::NOT, prev->line);
            break;
        }
        case token_type::LESS: {
            get_current_function().write_instruction(opcode::LESS, prev->line);
            break;
        }
        case token_type::LESS_EQUAL: {
            get_current_function().write_instruction(opcode::GREATER, prev->line);
            get_current_function().write_instruction(opcode::NOT, prev->line);
            break;
        }
        default:
            sting::panic("Unknown binary operator");
    }
}

void parser::binary_and(bool assignable) {
    u64 _and = emit_jump(opcode::BRANCH_FALSE);
    get_current_function().write_instruction(opcode::POP, prev->line);
    parse_precedence(precedence::AND);
    backpatch(_and);
}

void parser::binary_or(bool assignable) { // could just have  a BRANCH_TRUE.
    u64 first_not = emit_jump(opcode::BRANCH_FALSE);
    u64 first_true = emit_jump(opcode::BRANCH);
    backpatch(first_not);
    get_current_function().write_instruction(opcode::POP, prev->line);
    parse_precedence(precedence::OR);
    backpatch(first_true);
}

void parser::print() {
    get_next_token();
    expression();
    consume(token_type::SEMICOLON, "Expected ;");
    get_current_function().write_instruction(opcode::PRINT, prev->line);
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
