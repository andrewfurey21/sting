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


def run_from_file(file_name:str):
    assert file_name.split(".")[-1] == "sting", "this is not a sting source file"
    with open(file_name, "r") as file:
        source = file.read()
    tokens = scan(source)
    assert not SyntaxError._error, "there was a lexer error"

    parser = Parser(tokens)
    assert not SyntaxError._error, "there was a parser error"
    final_expression = parser()
    assert final_expression is not None, "parser returned none"

    ast_printer = ASTPrinter()
    print(ast_printer(final_expression))

if __name__ == "__main__":
    assert len(sys.argv) == 2, "usage: python main.py [script]"
    run_from_file(sys.argv[1])
