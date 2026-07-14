#!/bin/bash
# comple with debug symbols enabled and no optimization
#
gcc -g src/main.c -O1  -o tris.dbg && gdb ./tris.dbg 
#-fsanitize=address -fno-omit-frame-pointer
