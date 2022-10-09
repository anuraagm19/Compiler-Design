CS 348 Programming Assignment 2: Assembler for SIC XE + 2 Pass Linking Loader
Anuraag Mahajan - 190101110

Execution Instructions: (LINUX g++)
Assembler:
$ g++ assembler.cpp
$ ./a.out <input_file_name>        - you can use input.txt included in the folder

Output files:
output.txt : contains generated object code in the format of Header (H) , Define (D) , Refer (R) , Text (T) and End (E) records. 
intermediate.txt : Pass1 will create an intermediate file named intermediate.txt containing assigned addresses to all statements in the program.

Linker Loader:
$ g++ linker.cpp
$ ./a.out <input_file_name>   - you can use the output.txt produced by assembler as an input here.

(extra sample input from the book also provided in sample.txt
sample.txt contains object code from book = fig 3.9, note that the ouput would be generated with "0000" as the starting address,
as opposed to the ouput in the book which starts at "4000".)


Output files:
estab.txt - pass 1: contains all external symbols defined in the set of control sections together with the address assigned to each 
memory_map.txt - pass2: memory mapping representation
memory_listing.txt - extra: lists the byte wise address mapping

(the linking loader takes object codes both with or without caret symbols)

Extra considerations:
1) all input statements are assumed to be in uppercase
2) blank lines, if any, would be ignored by the program
3) ram size is restricted to 2^15 bytes
4) assumed character strings stored using BYTE do not exceed 30 bytes i.e. max text record length.
5) absence of START directive - set start address and location counter to 0
6) duplicate external symbols and duplicate sections reported accordingly by the linker.