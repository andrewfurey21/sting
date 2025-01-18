import sys
from enum import Enum, auto
from typing import List

class TokenType(Enum):
    # Single-character tokens.
    LEFT_PAREN = auto(); RIGHT_PAREN = auto(); 
    LEFT_BRACE = auto(); RIGHT_BRACE = auto(); 
    COMMA = auto(); DOT = auto(); MINUS = auto();  
    PLUS = auto(); SEMICOLON = auto(); SLASH = auto(); STAR = auto();

    # One or two character tokens.
    NOT = auto(); NOT_EQUAL= auto();
    ASSIGN = auto(); EQUAL = auto();
    GREATER = auto(); GREATER_EQUAL = auto();
    LESS = auto(); LESS_EQUAL = auto();

    # Literals.
    IDENTIFIER = auto(); STRING = auto(); NUMBER = auto();

    # Keywords.
    AND = auto(); CLASS = auto(); ELSE = auto(); FALSE = auto(); 
    FUN = auto(); FOR = auto(); IF = auto(); NIL = auto(); OR = auto();
    PRINT = auto(); RETURN = auto(); SUPER = auto(); THIS = auto(); 
    TRUE = auto(); VAR = auto(); WHILE = auto();

    EOF = auto();

class Token:
    def __init__(self, token_type:TokenType, lexeme:str, literal:object, line_number:int):
        self.token_type = token_type
        self.lexeme = lexeme
        self.literal = literal
        self.line_number = line_number

    def __str__(self):
        return f"{self.token_type} {self.lexeme} {self.line_number}"

def scan(file_name:str, source:str) -> List[Token]:
    tokens: List[Token] = [];
    current = 0
    line = 1
    def add_token(token_type:TokenType, text, line_number, literal:object=None):
        tokens.append(Token(token_type, text, literal, line_number))
    while current < len(source):
        start = current
        current+=1
        match source[start]:
            case "\n": line += 1
            case "(": add_token(TokenType.LEFT_PAREN, source[start:current], line)
            case ")": add_token(TokenType.RIGHT_PAREN, source[start:current], line)
            case "{": add_token(TokenType.LEFT_BRACE, source[start:current], line)
            case "}": add_token(TokenType.RIGHT_BRACE, source[start:current], line)
            case ",": add_token(TokenType.COMMA, source[start:current], line)
            case ".": add_token(TokenType.DOT, source[start:current], line)
            case "-": add_token(TokenType.MINUS, source[start:current], line)
            case "+": add_token(TokenType.PLUS, source[start:current], line)
            case ";": add_token(TokenType.SEMICOLON, source[start:current], line)
            case "*": add_token(TokenType.STAR, source[start:current], line)
            case _: 
                Error.line_error(file_name, line, "unexpected lexeme")

    return tokens

def run_from_file(file_name:str):
    assert file_name.split(".")[-1] == "sting", "this is not a sting source file"
    with open(file_name, "r") as file:
        source = file.read()
    tokens = scan(file_name, source)
    assert not Error.error, "there was a problem running the code"
    
    for token in tokens:
        print(token)

class Error:
    error = False
    @classmethod
    def line_error(cls, file_name, line_number, message):
        print(f"Error at {file_name}:{line_number}: {message}")
        cls.error = True

if __name__ == "__main__":
    assert len(sys.argv) == 2, "usage: python main.py [script]"
    run_from_file(sys.argv[1])



