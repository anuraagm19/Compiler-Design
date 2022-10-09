CS 348 Programming Assignment 1: 2 PASS ASSEMBLER
Anuraag Mahajan - 190101110

To execute program: (LINUX g++)
$ g++ 190101110_Assign01.cpp
$ ./a.out <inputfile>

Output file: out.txt
Intermediate.txt : Pass1 will create an intermediate file named intermediate.txt containing assigned addresses 
to all statements in the program

Assumptions:
1) all input statements are assumed to be in uppercase
2) blank lines, if any, would be ignored by the program
3) ram size is restricted to 2^15 bytes, error displayed if memory is exceeded  
4) character strings stored using BYTE do not exceed 30 bytes i.e. max text record length.
5) only aplhanumeric characters allowed for labels
6) first six characters of label assigned to start statement would be used as program name, 
default name "PROG  " will be used incase the label is not specified


Essential cases and error checks included:
1) absence of START directive - set start address and location counter to 0
2) absence of END directive - exit program by reporting the error
3) Invalid label check (should be alphanumeric, starting with character and not same as any reserved words) 
4) Invalid mnemonic check (scan OPTABLE)
5) Invalid operand check :
    should be present as a valid label (scan SYMTAB) or
    should be hex/characters/decimal type for WORD,BYTE,RESB and RESWORD or
    should be in a index based addressing mode format
   
6) Duplicate label
7) For hex/characters/decimal type operand, check for corresponding memory size
8) error on location counter exceeding maximum memory limit
