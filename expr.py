from __future__ import annotations
from typing import Protocol, TypeVar, Optional
T = TypeVar('T', covariant=True)

from main import Token
class Expr():
  def accept(self, visitor:Operation[T]) -> Optional[T]:
    pass

class Operation(Protocol[T]):
  def visit_binary_expr(self, expr:Binary) -> Optional[T]:
    pass

  def visit_grouping_expr(self, expr:Grouping) -> Optional[T]:
    pass

  def visit_literal_expr(self, expr:Literal) -> Optional[T]:
    pass

  def visit_unary_expr(self, expr:Unary) -> Optional[T]:
    pass

class Binary(Expr):
  def __init__(self, left:Expr, op:Token, right:Expr):
    self.left = left
    self.op = op
    self.right = right

  def accept(self, visitor:Operation[T]) -> Optional[T]:
    return visitor.visit_binary_expr(self)

class Grouping(Expr):
  def __init__(self, expression:Expr):
    self.expression = expression

  def accept(self, visitor:Operation[T]) -> Optional[T]:
    return visitor.visit_grouping_expr(self)

class Literal(Expr):
  def __init__(self, value:object):
    self.value = value

  def accept(self, visitor:Operation[T]) -> Optional[T]:
    return visitor.visit_literal_expr(self)

class Unary(Expr):
  def __init__(self, op:Token, right:Expr):
    self.op = op
    self.right = right

  def accept(self, visitor:Operation[T]) -> Optional[T]:
    return visitor.visit_unary_expr(self)

