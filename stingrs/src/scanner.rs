use std::{
    collections::HashMap,
    fs,
    io::{self, Write},
    path,
};

pub fn correct_usage_message() {
    println!("Usage: string [script]");
}

fn is_alpha(a: &u8) -> bool {
    (b'a'..=b'z').contains(a) || (b'A'..=b'Z').contains(a) || *a == b'_'
}

fn is_alphanumeric(a: &u8) -> bool {
    is_alpha(a) || (b'0'..=b'9').contains(a)
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

#[derive(Debug, Default, Clone)]
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

    //one or two char tokens
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
    Nil,
    True,
    False,
    And,
    Or,
    If,
    Else,
    Class,
    Super,
    This,
    For,
    While,
    Fun,
    Return,
    Var,
    Print,
    //Stop token
    #[default]
    Eof,
}

#[derive(Debug, Default)]
pub struct Token {
    token_type: TokenType,
    literal: Option<Vec<u8>>,
    line: usize,
}

impl Token {
    fn to_string(&self) -> String {
        if let Some(data) = &self.literal {
            return format!("{:?}, {:?}, {}", data, self.token_type, self.line);
        }
        return format!("{:?}, {:?}, {}", self.literal, self.token_type, self.line);
    }
}

//Really need to make DFA thing thats nicer looking
fn scan(source: &String) -> Vec<Token> {
    let source_bytes = source.as_bytes();
    let mut current_byte: usize = 0;
    let mut start_byte: usize;
    let mut line: usize = 1;

    let reserved_words: HashMap<&'static str, TokenType> = HashMap::from([
        ("nil", TokenType::Nil),
        ("true", TokenType::True),
        ("false", TokenType::False),
        ("and", TokenType::And),
        ("or", TokenType::Or),
        ("if", TokenType::If),
        ("else", TokenType::Else),
        ("class", TokenType::Class),
        ("super", TokenType::Super),
        ("this", TokenType::This),
        ("for", TokenType::For),
        ("while", TokenType::While),
        ("fun", TokenType::Fun),
        ("return", TokenType::Return),
        ("var", TokenType::Var),
        ("Print", TokenType::Print),
    ]);

    let mut tokens = vec![];
    loop {
        if current_byte >= source_bytes.len() {
            break;
        }
        start_byte = current_byte;

        match source_bytes[start_byte] {
            b'(' => tokens.push(Token {
                token_type: TokenType::LeftParen,
                literal: None,
                line,
            }),
            b')' => tokens.push(Token {
                token_type: TokenType::RightParen,
                literal: None,
                line,
            }),
            b'{' => tokens.push(Token {
                token_type: TokenType::LeftBrace,
                literal: None,
                line,
            }),
            b'}' => tokens.push(Token {
                token_type: TokenType::RightBrace,
                literal: None,
                line,
            }),
            b',' => tokens.push(Token {
                token_type: TokenType::Comma,
                literal: None,
                line,
            }),
            b'.' => tokens.push(Token {
                token_type: TokenType::Dot,
                literal: None,
                line,
            }),
            b'-' => tokens.push(Token {
                token_type: TokenType::Minus,
                literal: None,
                line,
            }),
            b'+' => tokens.push(Token {
                token_type: TokenType::Plus,
                literal: None,
                line,
            }),
            b';' => tokens.push(Token {
                token_type: TokenType::Semicolon,
                literal: None,
                line,
            }),
            b'*' => tokens.push(Token {
                token_type: TokenType::Star,
                literal: None,
                line,
            }),
            b'!' => {
                if source_bytes[current_byte + 1] == b'=' {
                    tokens.push(Token {
                        token_type: TokenType::BangEqual,
                        literal: None,
                        line,
                    });
                    current_byte += 1;
                } else {
                    tokens.push(Token {
                        token_type: TokenType::Bang,
                        literal: None,
                        line,
                    })
                }
            }
            b'=' => {
                if source_bytes[current_byte + 1] == b'=' {
                    tokens.push(Token {
                        token_type: TokenType::EqualEqual,
                        literal: None,
                        line,
                    });
                    current_byte += 1;
                } else {
                    tokens.push(Token {
                        token_type: TokenType::Equal,
                        literal: None,
                        line,
                    })
                }
            }
            b'>' => {
                if source_bytes[current_byte + 1] == b'=' {
                    tokens.push(Token {
                        token_type: TokenType::GreaterEqual,
                        literal: None,
                        line,
                    });
                    current_byte += 1;
                } else {
                    tokens.push(Token {
                        token_type: TokenType::Greater,
                        literal: None,
                        line,
                    })
                }
            } //TODO: use regex library instead to fix match cases
            b'<' => {
                if source_bytes[current_byte + 1] == b'=' {
                    tokens.push(Token {
                        token_type: TokenType::LessEqual,
                        literal: None,
                        line,
                    });
                    current_byte += 1;
                } else {
                    tokens.push(Token {
                        token_type: TokenType::Less,
                        literal: None,
                        line,
                    })
                }
            }
            b'/' => {
                if source_bytes[current_byte + 1] == b'/' {
                    while source_bytes[current_byte + 1] != b'\n' {
                        current_byte += 1;
                    }
                } else {
                    tokens.push(Token {
                        token_type: TokenType::Slash,
                        literal: None,
                        line,
                    })
                }
            }
            b' ' | b'\r' | b'\t' => (),
            b'\n' => line += 1,
            b'"' => {
                current_byte += 1;
                let started_line = line;
                while current_byte < source_bytes.len() && source_bytes[current_byte] != b'"' {
                    if source_bytes[current_byte] == b'\n' {
                        line += 1;
                    }
                    current_byte += 1;
                }
                if current_byte >= source_bytes.len() {
                    println!("Error: unterminated string at line {}", started_line);
                    break;
                }
                tokens.push(Token {
                    token_type: TokenType::String,
                    literal: Some(source_bytes[start_byte + 1..current_byte].to_vec()),
                    line,
                });
            }
            b'0'..=b'9' => {
                while current_byte < source_bytes.len()
                    && (b'0'..=b'9').contains(&source_bytes[current_byte])
                {
                    current_byte += 1;
                }
                if source_bytes[current_byte] == b'.' {
                    current_byte += 1;
                    while current_byte < source_bytes.len()
                        && (b'0'..=b'9').contains(&source_bytes[current_byte])
                    {
                        current_byte += 1;
                    }
                }
                tokens.push(Token {
                    token_type: TokenType::Number,
                    literal: Some(source_bytes[start_byte..current_byte].to_vec()),
                    line,
                });
                current_byte -= 1; // TODO: this really needs clean up
            }
            b'a'..=b'z' | b'A'..=b'Z' | b'_' => {
                while current_byte < source_bytes.len()
                    && is_alphanumeric(&source_bytes[current_byte])
                {
                    current_byte += 1;
                }
                let source_as_vec = source_bytes[start_byte..current_byte].to_vec();
                let word = std::str::from_utf8(&source_as_vec).expect("wtf");
                if reserved_words.contains_key(word) {
                    tokens.push(Token {
                        token_type: reserved_words.get(word).unwrap().clone(),
                        literal: Some(source_bytes[start_byte..current_byte].to_vec()),
                        line,
                    });
                } else {
                    tokens.push(Token {
                        token_type: TokenType::Identifier,
                        literal: Some(source_bytes[start_byte..current_byte].to_vec()),
                        line,
                    });
                }
                current_byte -= 1; // TODO: this really needs clean up
            }
            unknown_token => println!(
                "Error: unknown token {} at line {}",
                String::from_utf8(vec![unknown_token]).unwrap(),
                line
            ),
        }
        current_byte += 1;
    }
    tokens.push(Token {
        token_type: TokenType::Eof,
        literal: None,
        line,
    });
    return tokens;
}

pub fn run_program(program: &String) {
    let tokens = scan(program);
    for token in tokens {
        println!("{}", token.to_string());
    }
}

pub fn run_prompt() {
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

pub fn run_file(file_name: String) {
    println!("Running {}...", file_name);
    let file_path = path::Path::new(&file_name);
    let program: String = load_file(&file_path);
    run_program(&program);
}
