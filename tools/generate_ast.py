import sys
from typing import List
from io import TextIOWrapper

def fn_name(type_name:str, base_name:str):
    return f"visit_{type_name.lower()}_{base_name.lower()}"

def define_ast(output_dir:str, base_name:str, types:List[str]):
    path = output_dir + "/" + base_name + ".py"
    with open(path, "w") as f:
        f.write("from __future__ import annotations\n")
        f.write("from typing import Protocol, TypeVar\n")
        f.write("T = TypeVar('T', covariant=True)\n\n")
        f.write("from scan import Token\n")

        f.write(f"class {base_name.capitalize()}():\n")
        f.write(f"  def accept(self, visitor:Operation[T]) -> T:\n    raise NotImplementedError\n\n")
        define_operation(f, base_name, types)
    
        for type_info in types:
            class_name = type_info.split(":", 1)[0].strip()
            fields = type_info.split(":", 1)[1].strip()

            f.write(f"class {class_name}({base_name.capitalize()}):\n")
            f.write(f"  def __init__(self, {fields}):\n")

            for field in fields.split(", "):
                name = field.split(":")[0].strip()
                f.write(f"    self.{name} = {name}")
                f.write("\n")
            f.write("\n")

            f.write(f"  def accept(self, visitor:Operation[T]) -> T:\n")
            f.write(f"    return visitor.{fn_name(class_name, base_name)}(self)\n\n")

def define_operation(f:TextIOWrapper, base_name:str, types:List[str]):
    f.write("class Operation(Protocol[T]):\n")
    for type in types:
        type_name = type.split(":")[0].strip()
        f.write(f"  def {fn_name(type_name, base_name)}(self, {base_name}:{type_name}) -> T:\n    raise NotImplementedError\n\n")

if __name__ == "__main__":
    assert len(sys.argv) == 2, "usage: python generate_ast.py <output directory>"
    output_dir = sys.argv[1]

    expr_types = [
        "Binary     : left:Expr, op:Token, right:Expr",
        "Grouping   : expression:Expr",
        "Literal    : value:object",
        "Unary      : op:Token, right:Expr",
    ]
    stmt_types = [
        "Expression : expression:Expr",
        "Print      : expression:Expr",
    ]

    define_ast(output_dir, "expr", expr_types)
    define_ast(output_dir, "stmt", stmt_types)


