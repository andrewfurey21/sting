from __future__ import annotations
from typing import Protocol, TypeVar
T = TypeVar('T', covariant=True)

from scan import Token
class Stmt():
  def accept(self, visitor:Operation[T]) -> T:
    raise NotImplementedError

class Operation(Protocol[T]):
  def visit_expression_stmt(self, stmt:Expression) -> T:
    raise NotImplementedError

  def visit_print_stmt(self, stmt:Print) -> T:
    raise NotImplementedError

class Expression(Stmt):
  def __init__(self, expression:Expr):
    self.expression = expression

  def accept(self, visitor:Operation[T]) -> T:
    return visitor.visit_expression_stmt(self)

class Print(Stmt):
  def __init__(self, expression:Expr):
    self.expression = expression

  def accept(self, visitor:Operation[T]) -> T:
    return visitor.visit_print_stmt(self)

