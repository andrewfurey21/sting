import sys
from typing import List

"""
[
"Binary   : left:Expr, op:Token, right:Expr",
"Grouping : expression:Expr",
"Literal  : value:object",
"Unary    : op:Token, right:Expr"
]
"""

def define_ast(output_dir:str, base_name:str, types:List[str]):
    path = output_dir + "/" + base_name + ".py"
    with open(path, "w") as f:
        f.write("from main import Token\n")
        f.write("class Expr():\n  pass\n\n")
        for type in types:
            # TODO: might need to trim
            class_name = type.split(":", 1)[0].strip()
            fields = type.split(":", 1)[1].strip()

            f.write(f"class {class_name}({base_name.capitalize()}):\n")
            f.write(f"  def __init__(self, {fields}):\n")

            for field in fields.split(", "):
                name = field.split(":")[0].strip()
                f.write(f"    self.{name} = {name}")
                f.write("\n")

            f.write("\n")


if __name__ == "__main__":
    assert len(sys.argv) == 2, "usage: python generate_ast.py <output directory>"
    output_dir = sys.argv[1]

    types = \
[
"Binary   : left:Expr, op:Token, right:Expr",
"Grouping : expression:Expr",
"Literal  : value:object",
"Unary    : op:Token, right:Expr"
]

    define_ast(output_dir, "expr", types)


