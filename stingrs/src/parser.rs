use crate::scanner::Token;

//should i write a macro that generates expressions?
//visitor pattern

trait Expr {
    fn interpret();
}

struct Binary<T: Expr> {
    left: T,
    operator: Token,
    right: T,
}

impl<T> Expr for Binary<T>
where
    T: Expr,
{
    fn interpret() {
        println!("interpreting binary...");
    }
}
