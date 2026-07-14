#!/bin/bash

[[ $# -lt 2  ]] && games=1000 && epics=1
[[ $# -eq 2 ]] && games=$1 && epics=1 
[[ $# -eq 3  ]] && epics=$1 && games=$2
gcc -W -Wall -O2 src/main.c --sanitize=address -g  -o tris &&  ./tris $@ 
# gcc src/main.c -o tris &&  ./tris $@ 
