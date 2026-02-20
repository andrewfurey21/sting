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
    bool captured = false;

    bool operator==(const local& other) const {
        return name == other.name && depth == other.depth;
    }
};

struct upvalue {
    i64 index;
    bool local;

    upvalue(u64 index, bool local) : index(index), local(local) {}
    bool operator==(const upvalue& other)  { return index == other.index && local == other.local; }
};

// in the book its a stack of compilers.
struct compiler {
    // each function tracks a stack of locals and upvalues
    dynarray<function> functions;
    dynarray<dynarray<local>> _locals;
    dynarray<dynarray<upvalue>> _upvalues;
    i64 scope_depth;

    compiler() : functions(), scope_depth(0), _locals(), _upvalues() {
        new_function(function("script", 0));
    }

    i64 resolve_local(const token& t, const dynarray<local>& locals) {
        if (scope_depth == 0) return -1;
        for (i64 i{static_cast<i64>(locals.size()) - 1l}; i >= 0; i--) {
            if (locals.at(i).name == t) {
                panic_if(locals.at(i).depth == -1,
                        "Cannot define local with itself.");
                return i;
            }
        }
        return -1;
    }

    // return its location in the upvalues array
    // index and local are just used as identifiers
    i64 add_upvalue(dynarray<upvalue>& upvalues, u64 index, bool local) {
        const upvalue uv = upvalue(index, local);
        for (u64 i = 0; i < upvalues.size(); i++) {
            if (upvalues.at(i) == uv) return i;
        }
        upvalues.push_back(uv);
        return upvalues.size() - 1;
    }

    i64 resolve_upvalue(const token& t) {
        return resolve_upvalue(t, _locals.size() - 1);
    }

    // maybe shift all upvalues to the left by one in _upvalues stack?
    // first one isn't being used. would reduce all the "... - 1"
    i64 resolve_upvalue(const token& t, const i64 local_depth) {
        if (local_depth <= 0ll) return -1;
        const i64 l = resolve_local(t, _locals.at(local_depth - 1));
        if (l != -1) {
            _locals.at(local_depth - 1).at(l).captured = true;
            return add_upvalue(_upvalues.at(local_depth), l, true);
        }

        const i64 u = resolve_upvalue(t, local_depth - 1);
        if (u != -1) return add_upvalue(_upvalues.at(local_depth), u, false);
        else return -1;
    }

    void new_function(const function& f) {
        functions.push_back(f);
        _locals.push_back(dynarray<local>());
        _upvalues.push_back(dynarray<upvalue>()); // not needed for script though...
    }

    dynarray<local>& locals() { return _locals.back(); }
    dynarray<upvalue>& upvalues() { return _upvalues.back(); }

    function finish_function() {
        // sanity checks to make sure stack is cleaned up properly.
        panic_if(_locals.pop_back().size() > 0, "Stack is not zero, missed local pop somewhere");
        // panic_if(_upvalues.pop_back().size() > 0, "Stack is not zero, missed upvalue pop somewhere");
        //_upvalues.pop_back();
        return functions.pop_back();
    }

    void pop_upvalues() { _upvalues.pop_back(); }
};

// rename parser -> compiler. merge parser + compiler.
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
    void fix_block_stack();
    void declare_function_param();
    void declare_local_variable();
    void variable(bool assignable);
    u64 parse_variable_name();
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
