
# book

1. where do closed variables go?
2. when do we close variables?

we always close them if theres an upvalue referencing the data.
GC takes care of it if the closure is dead. checking adds complexity ig
the author wasn't bothered with.

## closed vs open

`is_captured` bool to differentiate running `POP` and `CLOSE_VALUE` opcodes.

### TODO:

- [x] add an `is_captured` bool to local
- [x] when writing pop instructions, check to see if it's captured. if it is `CLOSE_VALUE`.

## tracking open upvalues

if two closures reference the same variable, and are live outside of that
variables enclosing function, then they should still reference the same
variable on the heap, to keep the semantics when that variable was on the stack.

instead of every rtupvalue pointing to a data, one points to a data and you
form a linked list for every other rtupvalue that references the same variable.

the book:
doesn't this make getting the value of an upvalue slow? O(n) right?
I don't think there should be that many upvalues. so not a big deal?

me:
my original idea was to just have rtupvalues point to the same thing.
maybe that makes doing gc hard.

### TODO
1. Add a next member to rtupvalue. use these as a link list, where the root node
has the data.

2. In `capture_value`, loop through

## closing upvalues at runtime












































# function call notes

## call frame

* function object, including bytecode, name and arity
* current program counter
* base value pointer. (includes params), some offset into value_stack

# backend - running functions
## call op

vm has a stack of call frames.

calls vm::call_value, takes in value + argcount. 
call_value will switch case on value type, if function then vm::call_function;
call_function create a new call frame, push onto frame stack. set top frame ip to function bytecode, and bp to stack.size - arity.

```
fn(arg);
```

fn function value will get loaded onto the stack (at compile time the function object is placed in the current functions constant pool),
since fn is just a global variable that points to a function object.
call_function takes this function object, creates a new call frame.
new frames pc points to first bytecode instruction in function object. increments with next_instruction.
new frames base value pointer points to top of stack - arg count.

in assembly, you need a frame pointer when stack allocation is not known at compile time.

## return op

not necessarily tied to return keyword.
end of script will have a return to clean up stack and end program.

if frame count == 0, pop
pop frame.
set vm frame to current top frame.

# compiling and generating code

compiler struct will have a function array.

## allocating locals

### params

i imagine you do this by simply calling `expression()` for each paramter at the call site.
params will be then at the top of the stack.

## return addresses and returning values

basically just return op. value should be on top of stack.

## call stack

### compiling to function objects

compiler struct keeps track of locals and scope depth, so that we can write instructions for getting/settings locals.

now, add a function and type (function or top-level script). write to the functions chunk instead.

so, chk() should return compiler.function.bytecode

how do we output this for debug? functions stored in constant_pool. could go down recursively, constant pools holding functions holding constant pools forms a tree.

index 0 of compiler local stack should be an empty one, ie token.start = "", length = 0, depth = 0;

why?

need to disassemble chunks with function names

### stack traces

# frontend - parsing (reading functions definitions and calls)

```
fun name(param1, param2, ...) {
    statement;
    return?
}
```

should not be able to use return outside of function definition.
