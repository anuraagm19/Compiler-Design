CS 348 Programming Assignment 3: Pascal Compiler
Anuraag Mahajan - 190101110

Pre-requisite: 
Bison (yacc) and flex (lex) should be installed.
Install using:
$ sudo apt-get install bison flex

Commands to execute program: (LINUX g++)
$ flex -o lex.yy.cc 190101110_lex.l 
$ g++ lex.yy.cc
$ ./a.out < input.pas > output.txt

Output file: output.txt : contains line wise token type and assigned token specifier