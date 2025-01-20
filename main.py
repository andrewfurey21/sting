import sys
from typing import List

from scan import *
from expr import *
from err import SyntaxError
from parse import Parser

# TODO: change to classmethods?
class ASTPrinter(Operation[str]):
    def __call__(self, expr:Expr):
        return expr.accept(self)

    def paren(self, name:str, exprs:List[Expr]) -> str:
        string = "(" + name
        for expr in exprs:
            string += " " + expr.accept(self)
        string += ")"
        return string

    def visit_binary_expr(self, expr:Binary) -> str:
        return self.paren(expr.op.lexeme, [expr.left, expr.right])

    def visit_grouping_expr(self, expr:Grouping) -> str:
        return self.paren("group", [expr.expression])

    def visit_literal_expr(self, expr:Literal) -> str:
        # TODO: deal wil nil/null
        return str(expr.value)

    def visit_unary_expr(self, expr:Unary) -> str:
        return self.paren(expr.op.lexeme, [expr.right])

class Interpreter(Operation[object]):
    def __call__(self, expr:Expr):
        return self._eval(expr)

    def _eval(self, expr:Expr) -> object:
        return expr.accept(self)

    def _truthy(self, expr:Literal) -> object:
        if type(expr.value) == bool:
            return expr.value
        elif expr.value is not None:
            return True
        return False

    def _equal(self, lhs:object, rhs:object) -> bool:
        # if a is None and b is None:
        #     return True
        # if a is None:
        #     return False
        return a == b

    def visit_binary_expr(self, expr:Binary) -> object:
        left = self._eval(expr.left)
        right = self._eval(expr.right)
        match expr.op.token_type:
            case TokenType.STAR:
                return float(left) * float(right)
            case TokenType.SLASH:
                return float(left) / float(right)
            case TokenType.MINUS:
                return float(left) - float(right)
            case TokenType.PLUS:
                return left + right
            case TokenType.GREATER:
                return left > right
            case TokenType.GREATER_EQUAL:
                return left >= right
            case TokenType.LESS:
                return left < right
            case TokenType.LESS_EQUAL:
                return left <= right
            case TokenType.EQUAL:
                return self._equal(left, right)

    def visit_grouping_expr(self, expr:Grouping) -> object:
        return self._eval(expr.expression)

    def visit_literal_expr(self, expr:Literal) -> object:
        return expr.value

    def visit_unary_expr(self, expr:Unary) -> object:
        match expr.op:
            case TokenType.MINUS:
                return -float(self._eval(expr.right))
            case TokenType.NOT:
                return not self._truthy(self._eval(expr.right))
        return None

def run_from_file(file_name:str):
    assert file_name.split(".")[-1] == "sting", "this is not a sting source file"
    with open(file_name, "r") as file:
        source = file.read()
    tokens = scan(source)
    assert not SyntaxError._error, "there was a lexer error"

    parser = Parser(tokens)
    assert not SyntaxError._error, "there was a parser error"
    ast = parser()
    assert ast is not None, "parser returned none"

    ast_printer = ASTPrinter()
    print(ast_printer(ast))

    interpret = Interpreter()
    value = interpret(ast)

    print(value)

if __name__ == "__main__":
    assert len(sys.argv) == 2, "usage: python main.py [script]"
    run_from_file(sys.argv[1])
