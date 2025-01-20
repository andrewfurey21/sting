from typing import List, Optional
from scan import *
from expr import *
from err import SyntaxError

"""
BNF grammar

program        → statement* EOF ;

statement      → exprStmt | printStmt ;

exprStmt       → expression ";" ;
printStmt      → "print" expression ";" ;

expression     → equality ;
equality       → comparison ( ( "!=" | "==" ) comparison )* ;
comparison     → term ( ( ">" | ">=" | "<" | "<=" ) term )* ;
term           → factor ( ( "-" | "+" ) factor )* ;
factor         → unary ( ( "/" | "*" ) unary )* ;
unary          → ( "!" | "-" ) unary
               | primary ;
primary        → NUMBER | STRING | "true" | "false" | "nil"
               | "(" expression ")" ;

"""

class ParserError(Exception):
  def __init__(self):
    super().__init__()

  def __str__(self) -> str:
    return f"error while parsing!"


class Parser:
  def __init__(self, tokens:List[Token]):
    self.tokens: List[Token] = tokens
    self.current = 0

  def __call__(self) -> Optional[Expr]:
    try:
      return self._expression()
    except Exception as e:
      print(f"An error occured: {e}")
      return None

  def _match_token(self, token_types:List[TokenType]) -> bool:
    for token_type in token_types:
      if self._check(token_type):
        self.current += 1
        return True
    return False
  
  def _check(self, token_type:TokenType) -> bool: return not self._at_end() and self.tokens[self.current].token_type == token_type
  def _at_end(self) -> bool: return self.tokens[self.current] == TokenType.EOF

  def _parser_error(self, token:Token, message:str):
    SyntaxError.report_error(token.line_number, message)
    return ParserError

  def _consume(self, token_type:TokenType, message:str) -> Token:
    if self._check(token_type):
      self.current += 1
      return self.tokens[self.current - 1]
    raise self._parser_error(self.tokens[self.current], message)

  def _sync(self):
    print("syncing")
    if not self._at_end():
      self.current += 1

    while not self._at_end():
      if self.tokens[self.current - 1].token_type == TokenType.SEMICOLON: return

      match self.tokens[self.current]:
        case TokenType.CLASS: return
        case TokenType.FUN: return
        case TokenType.VAR: return
        case TokenType.FOR: return
        case TokenType.IF: return
        case TokenType.WHILE: return
        case TokenType.PRINT: return
        case TokenType.RETURN: return

      if not self._at_end():
        self.current += 1

  def _expression(self) -> Expr:
    return self._equality()

  def _equality(self) -> Expr:
    comparison_expr = self._comparison()
    while self._match_token([TokenType.NOT_EQUAL, TokenType.EQUAL]):
      operator:Token = self.tokens[self.current - 1]
      rhs = self._comparison()
      comparison_expr = Binary(comparison_expr, operator, rhs)
    return comparison_expr

  def _comparison(self) -> Expr:
    term_expr = self._term()
    while self._match_token([TokenType.GREATER, TokenType.GREATER_EQUAL, TokenType.LESS, TokenType.LESS_EQUAL]):
      operator:Token = self.tokens[self.current-1]
      rhs = self._term()
      term_expr = Binary(term_expr, operator, rhs)
    return term_expr

  def _term(self) -> Expr:
    factor_expr = self._factor()
    while self._match_token([TokenType.MINUS, TokenType.PLUS]):
      operator: Token = self.tokens[self.current - 1]
      rhs = self._factor()
      factor_expr = Binary(factor_expr, operator, rhs)
    return factor_expr

  def _factor(self) -> Expr:
    unary_expr = self._unary()
    while self._match_token([TokenType.SLASH, TokenType.STAR]):
      operator: Token = self.tokens[self.current - 1]
      rhs = self._unary()
      unary_expr = Binary(unary_expr, operator, rhs)
    return unary_expr

  def _unary(self) -> Expr:
    while self._match_token([TokenType.NOT, TokenType.MINUS]):
      operator: Token = self.tokens[self.current - 1]
      rhs = self._unary()
      return Unary(operator, rhs)
    return self._primary()

  def _primary(self) -> Expr:
    if self._match_token([TokenType.FALSE]): return Literal(False)
    if self._match_token([TokenType.TRUE]): return Literal(True)
    if self._match_token([TokenType.NIL]): return Literal(None)

    if self._match_token([TokenType.NUMBER, TokenType.STRING]): return Literal(self.tokens[self. current - 1].literal)

    if self._match_token([TokenType.LEFT_PAREN]):
      new_expr: Expr = self._expression()
      self._consume(TokenType.RIGHT_PAREN, "Expected ')'")
      return Grouping(new_expr)
    raise ParserError


