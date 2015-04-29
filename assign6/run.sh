#!/bin/sh/

reset
rm obj* vafile.bin vafile
g++ vafile.cpp -o vafile
./vafile