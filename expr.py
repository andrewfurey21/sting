from __future__ import annotations
from typing import Protocol, TypeVar
T = TypeVar('T', covariant=True)

from scan import Token
class Expr():
  def accept(self, visitor:Operation[T]) -> T:
    raise NotImplementedError

class Operation(Protocol[T]):
  def visit_binary_expr(self, expr:Binary) -> T:
    raise NotImplementedError

  def visit_grouping_expr(self, expr:Grouping) -> T:
    raise NotImplementedError

  def visit_literal_expr(self, expr:Literal) -> T:
    raise NotImplementedError

  def visit_unary_expr(self, expr:Unary) -> T:
    raise NotImplementedError

class Binary(Expr):
  def __init__(self, left:Expr, op:Token, right:Expr):
    self.left = left
    self.op = op
    self.right = right

  def accept(self, visitor:Operation[T]) -> T:
    return visitor.visit_binary_expr(self)

class Grouping(Expr):
  def __init__(self, expression:Expr):
    self.expression = expression

  def accept(self, visitor:Operation[T]) -> T:
    return visitor.visit_grouping_expr(self)

class Literal(Expr):
  def __init__(self, value:object):
    self.value = value

  def accept(self, visitor:Operation[T]) -> T:
    return visitor.visit_literal_expr(self)

class Unary(Expr):
  def __init__(self, op:Token, right:Expr):
    self.op = op
    self.right = right

  def accept(self, visitor:Operation[T]) -> T:
    return visitor.visit_unary_expr(self)

