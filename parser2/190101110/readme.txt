Assignment 4
Anuraag Mahajan
190101110

Pre-requisite: 
Bison (yacc) and flex (lex) should be installed.
Install using:
$ sudo apt-get install bison flex

Run instructions : (LINUX g++)
Run make  
>> make 

Run parser on correct input file 
>> ./parser input.p

Run parser on incorrect input file
>> ./parser error_input.p

Note that the input files must have "LF" end of line sequence format.

Errors handles : 
1. Syntactical Errors
2. Redeclaration of variable
3. Redefinition of variable
4. Missing declaration of variable
5. Using variable without initialisation 
6. Type mismatch in assigninment
7. Using keyword as identifier (syntactic error)
8. Unidentified character in input (lexical/syntactic error)
9. For loop has same type in range of index
10. Type mismatch in expression
(Any error occuring after a syntax errors is detected only after resolution of the syntactic error)