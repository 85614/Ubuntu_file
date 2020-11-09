#! /bin/bash
echo "flex"
flex hello.l
echo "gcc"
gcc lex.yy.c -o lex.yy -ll
echo "run"
./lex.yy 	
