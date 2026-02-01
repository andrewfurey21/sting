#ifndef PARSER_HPP
#define PARSER_HPP

#include "utilities.hpp"
#include "dynarray.hpp"
#include "scanner.hpp"
#include "vmachine.hpp"
#include "object.hpp"
#include "hashmap.hpp"
#include "function.hpp"

/*
 *  Parsing + codegen
 */

namespace sting {

enum precedence {
    NONE,        // 0
    ASSIGNMENT,  // 1 =
    OR,          // 2 or
    AND,         // 3 and
    EQUALITY,    // 4 == !=
    COMPARISON,  // 5 < > <= >=
    TERM,        // 6 + -
    FACTOR,      // 7 * /
    UNARY,       // 8 ! -
    CALL,        // 9 . ()
    PRIMARY      // 10
};

struct local {
    token name;
    i64 depth; // can have locals with same name, but different depths.

    bool operator==(const local& other) const {
        return name == other.name && depth == other.depth;
    }
};

struct compiler {
    compiler() : functions(), scope_depth(0), locals() {
        functions.push_back(function("script", 0));
    }
    dynarray<function> functions;
    i64 scope_depth;
    dynarray<local> locals; // kind of like simulating stack but at compile time
    // get correct location of local on the stack.
    // all locals in the current scope get popped off when going out of scope.
    // back through the stack and find first one with same token.

    i64 resolve_local(const token& t) {
        for (i64 i{static_cast<i64>(locals.size()) - 1l}; i >= 0; i--) {
            if (locals.at(i).name == t) {
                panic_if(locals.at(i).depth == -1, "Cannot define local with itself.");
                return i;
            }
        }
        return -1;
    }

    ~compiler() {
        // std::cout << "functions size: " << functions.size() << "\n" << std::flush;
    }
};

class parser {
public:
    parser();
    parser(const std::string& name);
    bool parse();
    void define_native_function(const string& name, const native_function& fn);
    void define_native_functions();
    dynarray<token>& get_tokens() { return tokens; }
    function& get_script() { return c.functions.at(0); }
    void error_at_token(const token& t, const std::string& msg);
    void check_current_token(const token_type expected, const std::string& mesg);
    void get_next_token();
    void consume(token_type type, const char* msg);
    void parse_precedence(precedence p);
    bool match(token_type type);
    u64 emit_jump(opcode branch_type);
    void backpatch(u64 branch);
    function& get_current_function() { return c.functions.back(); }

    // parse functions that generate code
    void declaration();
    void var_declaration();
    void fun_declaration();
    void declare_function_param();
    void declare_local_variable();
    void variable(bool assignable);
    u64 parse_global_variable_name();
    void named_variable(const token& name, bool assignable);
    void block();
    void return_statement();
    void statement();
    void if_statement();
    void while_statement();
    void for_statement();
    void expression_statement();
    void expression();
    void number(bool assignable);
    void literal(bool assignable);
    void str(bool assignable);
    void grouping(bool assignable);
    void unary(bool assignable);
    void binary(bool assignable);
    void binary_and(bool assignable);
    void binary_or(bool assignable);
    void print();

    // token related
    token *prev;
    token *current;
    u64 index;
    dynarray<token> tokens;
    bool parse_error;
    bool panic;

    // most important part
    compiler c;
};

using parse_fn = void (parser::*)(bool assignable);

struct parse_rule {
    parse_fn prefix;
    parse_fn infix;
    precedence prec;
};

parse_rule *get_rule(token_type type);

} // namespace sting

#endif
