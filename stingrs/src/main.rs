#![allow(dead_code)]
use std::{
    env, fs,
    io::{self, Write},
    path,
};

fn correct_usage_message() {
    println!("Usage: string [script]");
}

fn run_file(file_name: String) {
    println!("Running {}...", file_name);
    let file_path = path::Path::new(&file_name);
    let program: String = load_file(&file_path);
    run_program(&program);
}

fn load_file(file_path: &path::Path) -> String {
    let bytes: Vec<u8> = fs::read(file_path).expect("Couldn't read bytes from file");
    let bytes_as_string = String::from_utf8(bytes).expect("Couldn't convert bytes to string");
    return bytes_as_string;
}

#[allow(dead_code)]
fn error_message(line_number: u32, message: &String) {
    println!("[line {}] Error: {}", line_number, message);
}

#[derive(Debug, Default)]
enum TokenType {
    //Single char tokens
    LeftParen,
    RightParen,
    LeftBrace,
    RightBrace,
    Comma,
    Dot,
    Minus,
    Plus,
    Semicolon,
    Slash,
    Star,

    //one or to char tokens
    Bang,
    BangEqual,
    Equal,
    EqualEqual,
    Greater,
    GreaterEqual,
    Less,
    LessEqual,

    //literals
    Identifier,
    String,
    Number,

    //keywords
    And,
    Class,
    Else,
    False,
    Fun,
    For,
    If,
    Nil,
    Or,
    Print,
    Return,
    Super,
    This,
    True,
    Var,
    While,

    //Stop token
    #[default]
    Eof,
}

#[derive(Debug, Default)]
struct Token {
    token_type: TokenType,
    lexeme: String,
    literal: Option<String>,
    line: usize,
}

impl Token {
    // fn token_type(&mut self, ttype: TokenType) {
    //     self.token_type = ttype;
    // }

    fn to_string(&self) -> String {
        return format!("{}, {:?}, {}\n", self.lexeme, self.token_type, self.line);
    }
}

fn scan(source: &String) -> Vec<Token> {
    let source_bytes = source.as_bytes();
    let mut current_byte: usize = 0;
    let mut start_byte: usize = 0;
    let mut line: usize = 1;

    let mut tokens = vec![];
    // loop {
    //     if current_byte >= source_bytes.len() {
    //         break;
    //     }
    //     start_byte = current_byte;
    //
    //     // match source_bytes[current_byte] {
    //     //     '(' => tokens.push(Token {TokenType::LeftParen, });
    //     //
    //     // }
    // }
    tokens.push(Token {
        token_type: TokenType::Eof,
        lexeme: String::from(""),
        literal: None,
        line,
    });
    return tokens;
}

fn run_program(program: &String) {
    let tokens = scan(program);
    for token in tokens {
        if let Some(value) = token.literal {
            println!("literal: {}", value);
        } else {
            println!("End of file ({:?})", token.token_type);
        }
    }
}

fn run_prompt() {
    let mut stdout = io::stdout().lock();
    loop {
        print!(">> ");
        let _ = stdout.flush();
        let mut buffer = String::new();
        io::stdin()
            .read_line(&mut buffer)
            .expect("Couldn't read input");
        run_program(&buffer);
    }
}

fn main() {
    let args: Vec<String> = env::args().collect();
    match args.len() {
        1 => run_prompt(),
        2 => run_file(args[1].clone()),
        _ => correct_usage_message(),
    }
}
