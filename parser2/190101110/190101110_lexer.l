%{
#include <bits/stdc++.h>
#include "y.tab.h"
using namespace std;

%}

%option yylineno
%option caseless

DIGIT 				[0-9]
DIGITS  			{DIGIT}+

ID                  [a-zA-Z][_a-zA-Z0-9]*
INTEGER 			{DIGITS}
FLOAT				{INTEGER}(\.{DIGITS}([Ee][+-]?{DIGITS})?)

%%
[ \t]			                    ;
\n				                    {}
"{"[^"}"]*"}"                       ;
";"                                 { return SEMICOLON; }   
":"                                 { return COLON; }
","                                 { return COMMA; }
:=					                { return EQUAL; }
"+"                                 { return ADD; }
"-"                                 { return SUB; }
"*"                                 { return MULTIPLY; }
"DIV"                               { return DIVIDE; }
"("                                 { return OPENPAREN; }
")"                                 { return CLOSEPAREN; }
"program"                           { return PROGRAM; }
"var"                               { return VAR; }
"begin"                             { return BEGINPROG; }
"end"                               { return END; }
"end."                              {return EOFCODE;}
"integer"                           { return INTEGER; }
"real"                              { return REAL; }
"for"                               { return FOR;}
"read"                              { return READ;}
"write"                             { return WRITE;}
"to"                                { return TO; }
"do"                                { return DO; }
{INTEGER}                           { return INT; }
{FLOAT}	                            { return FLOAT; }
{ID}		                        { 
                                        yylval.stringValue=strdup(yytext);
                                        return IDENTIFIER; 
                                    }
.                                   {
                                        //unexpected character
                                        cout<< "Line "<<yylineno <<" : lexical error, unexpected character "<<yytext[0]<<"\n";
                                    } 
%%


int yywrap(void)    // used on end of input
{
    return 1;
}