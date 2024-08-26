use std::env;
mod scanner;

fn main() {
    let args: Vec<String> = env::args().collect();
    match args.len() {
        1 => scanner::run_prompt(),
        2 => scanner::run_file(args[1].clone()),
        _ => scanner::correct_usage_message(),
    }
}
