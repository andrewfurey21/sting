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
    println!("{}", program);
}

fn load_file(file_path: &path::Path) -> String {
    let bytes: Vec<u8> = fs::read(file_path).expect("Couldn't read bytes from file");
    let bytes_as_string = String::from_utf8(bytes).expect("Couldn't convert bytes to string");
    return bytes_as_string;
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
        println!("This is what you typed: {}", buffer);
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
