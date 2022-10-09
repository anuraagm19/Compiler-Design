/*
CS 348 Programming Assignment 3: Pascal Compiler
Anuraag Mahajan - 190101110

Commands to execute program: (LINUX g++)
$ flex -o lex.yy.cc 190101110_lex.l 
$ g++ lex.yy.cc
$ ./a.out < input.pas > output.txt

Output file: output.txt

*/

%{
#include <bits/stdc++.h>
using namespace std;

// Token coding scheme
#define PROGRAM 1
#define VAR 2
#define BEGINPROGRAM 3
#define END 4
#define EOPROG 5
#define INTEGER 6
#define FOR 7
#define READ 8
#define WRITE 9
#define TO 10
#define DO 11
#define DIV 19
#define ID 22
#define INT 23

#define MAX_TABLE_SIZE 256


int install_num();
int install_id();
bool tableFind();

int tableIndex = 0; 
int flag = 0;
string str="";
int yyline = 1;
int prevline = 0;
int lineNo;
int buffer;

string symbol_table[MAX_TABLE_SIZE];		// stores tokens and their codes in a hashtable

%}

%option noyywrap

%%


(program|PROGRAM)		{ return PROGRAM;}
(var|VAR)				{ return VAR;}
(begin|BEGIN)			{ return BEGINPROGRAM;}
(end|END)				{return END;}
(end|END)\.				{return EOPROG;}
(integer|INTEGER)		{ return INTEGER;}
(for|FOR)				{ return FOR;}
(read|READ)				{ return READ;}
(write|WRITE)			{ return WRITE;}
(to|TO)					{ return TO;}
(do|DO)					{ return DO;}
(div|DIV)				{return DIV;}

\n						{yyline++; }
[ \t]+					{}
[a-zA-Z]+[0-9]*			{flag = install_id(); return ID;}
[0-9]+					{flag = install_num();  return INT;}
"."						{ return EOPROG;}
";"                         {return 12;}
":"                         {return 13;}
","                         {return 14;}
":="                        {return 15;}
"+"                         {return 16;}
"-"                         {return 17;}
"*"                         {return 18;}
"("                         {return 20;}
")"                         {return 21;}
.							{ str=yytext; cout << "Unexpected input character \'" + str + "\'" << ", line=" << yyline << endl; }

%%

// finds a token in the hashtable
bool tableFind(string &s){
	for(auto it:symbol_table){
		if(it == s) return 1;
	}
	return 0;
}

// auxiliary function to insert a new id in the hashtable
int install_id(){ 
	str = yytext; 
	if(tableFind(str)) return 1;
	tableIndex++;
	symbol_table[tableIndex-1]=str; // insert if not present already
	return 0;
}
// auxiliary function to insert a new id in the hashtable
int install_num(){
	str = yytext;
	if(tableFind(str)) return 1;
	tableIndex++;
	symbol_table[tableIndex-1]=str; // insert if not present already
	return 0;
}


int main()
{	

	printf("Line\tToken type\tToken Specifier\n");
	// loop to process every token and generate the desired output
	while(1){
		if((buffer=yylex()) == EOPROG){ // end of file
			printf("%d",yyline);			// Print EOPROG 
			printf("\t\t%d\t\t", buffer);
			cout<<"\n";
			
			return 0;
		}

		// Read line by line and print output in format
		if(prevline == yyline){
		}
		else{
			prevline=yyline; 
			lineNo = yyline;		//Change line number
			printf("%d",lineNo);
		}

		printf("\t\t%d\t\t", buffer);

		// if yytext is a num
		if(buffer==23) {
			cout<<"#"<<yytext<<"\n";
		}
		// if yytext is an ID
		else if (buffer==22) cout<<"^"<<yytext<<"\n";
		else  printf("\n");
		
	}
}