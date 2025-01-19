import sys
from typing import List

from scan import *
from expr import *
from syntax_error import Error

# TODO: change to classmethods?
class ASTPrinter(Operation[str]):
    def paren(self, name:str, exprs:List[Expr]) -> str:
        string = "(" + name
        for expr in exprs:
            string += " " + expr.accept(self)
        string += ")"
        return string

    def print(self, expr:Expr):
        return expr.accept(self)

    def visit_binary_expr(self, expr:Binary) -> str:
        return self.paren(expr.op.lexeme, [expr.left, expr.right])

    def visit_grouping_expr(self, expr:Grouping) -> str:
        return self.paren("group", [expr.expression])

    def visit_literal_expr(self, expr:Literal) -> str:
        # TODO: deal wil nil/null
        return str(expr.value)

    def visit_unary_expr(self, expr:Unary) -> str:
        return self.paren(expr.op.lexeme, [expr.right])


def run_from_file(file_name:str):
    assert file_name.split(".")[-1] == "sting", "this is not a sting source file"
    with open(file_name, "r") as file:
        source = file.read()
    tokens = scan(file_name, source)
    assert not Error.error, "there was a problem running the code"
    
    for token in tokens:
        print(repr(token))


if __name__ == "__main__":
    assert len(sys.argv) == 2, "usage: python main.py [script]"
    run_from_file(sys.argv[1])

    expression = Binary(Unary(Token(TokenType.MINUS, "-", None, 1), Literal(123)), Token(TokenType.STAR, "*", None, 1), Grouping(Literal(45.67)))

    ast_print = ASTPrinter()
    print(ast_print.print(expression))





