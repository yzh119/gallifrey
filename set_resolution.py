import os
import sys

path = "src/io.h"
assert os.path.exists(path)
with open(path, 'r') as f:
    prog = f.readlines()

prog[22] = "const unsigned int width = " + sys.argv[1] + ";\n"
prog[23] = "const unsigned int height = " + sys.argv[2] + ";\n"

with open(path, 'w') as f:
    for line in prog:
        f.write(line)
