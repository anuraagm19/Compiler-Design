%{

#include <bits/stdc++.h>
using namespace std;



// Global variables

vector<string> DEC_LIST; // Temporary variable list
unordered_map<string,pair<int,int>> SYMTAB; // Symbol table
bool ERROR_FLAG=0; // Error flag

// Extern variables
extern int yylineno;
extern FILE* yyin;

// Function declarations
void checkAndInsert(string,int);
void semanticError(string);
void yyerror(const char *);
int yylex(void);
int yyparse(void);


#define YYERROR_VERBOSE


string types[2] ={"Integer", "Real"}; // Variable types
%}

%start Program

%union
{
    int intValue;
    float floatValue;
    char *stringValue;
}


%token <intValue> PROGRAM 1
%token <intValue> VAR 2
%token <intValue> BEGINPROG 3
%token <intValue> END 4
%token <intValue> EOFCODE  5
%token <intValue> INTEGER 6
%token <intValue> REAL  7
%token <intValue> FOR 8
%token <intValue> READ 9
%token <intValue> WRITE 10
%token <intValue> TO 11
%token <intValue> DO  12
%token <intValue> SEMICOLON 13
%token <intValue> COLON 14
%token <intValue> COMMA 15
%token <intValue> EQUAL 16
%token <intValue> ADD 17
%token <intValue> SUB 18
%token <intValue> MULTIPLY 19
%token <intValue> DIVIDE 20
%token <intValue> OPENPAREN 21
%token <intValue> CLOSEPAREN 22
%token <intValue> IDENTIFIER  23
%token <intValue> INT 24
%token <intValue> FLOAT  25
%type <intValue>  Factor
%type <intValue>  Exp 
%type <intValue>  Term 
%type <intValue>  Type 

%%
Program 				: PROGRAM ProgramName 
						VAR DecList
						BEGINPROG StmtList EOFCODE
                        ;

ProgramName             : IDENTIFIER
                        ;
                        

DecList 		        : Dec 
						|  DecList SEMICOLON Dec 
						;

Dec                     : IDList COLON Type 
                        {
                            // insert all symbols in SYM table with their type and check for redeclarations
                            for (auto x: DEC_LIST){
                                    checkAndInsert(x,$3);
                            }
                            DEC_LIST.clear();
                        }
                        ;

Type					: INTEGER {$$=0;}
						| REAL    {$$=1;}
						;
IDList  		        : IDENTIFIER 
                        {  
                            int sz = DEC_LIST.size();
                            DEC_LIST.resize(sz+1);
                            DEC_LIST[DEC_LIST.size()-1] = yylval.stringValue;
                        }
						
                        | IDList COMMA IDENTIFIER 
                        { 
                            int sz = DEC_LIST.size();
                            DEC_LIST.resize(sz+1);
                            DEC_LIST[DEC_LIST.size()-1] = yylval.stringValue;
                        }
						;
StmtList                : Stmt
                        | StmtList SEMICOLON Stmt
                        | error
                        ;

Stmt                    : Assign
                        | Read
                        | Write
                        | For
                        ;

Assign                  :  IDENTIFIER EQUAL Exp {
    
                            if(SYMTAB.find(yyval.stringValue) != SYMTAB.end()){
                                int type1 = SYMTAB[yyval.stringValue].first;

                                // Check if type matches in assignment 
                                if(type1==$3) SYMTAB[yyval.stringValue].second =1;
                                else semanticError("Type mismatch, trying to assign "+types[$3] +" to "+types[type1]+" variable : "+yylval.stringValue);
                               
                            }else{
                                // Check if variable is declared when it is being referenced
                                semanticError("Variable "+string(yylval.stringValue)+" not declared");
                            }
                        }
                        ;
Exp                     : Term 
                        | Exp ADD Term { if($1^$3) semanticError("Type mismatch in expression");}
                        | Exp SUB Term { if($1^$3) semanticError("Type mismatch in expression");}
                        ;

Term                    : Factor {$$=$1;}
                        | Term MULTIPLY Factor { if($1^$3) semanticError("Type mismatch in expression");}
                        | Term DIVIDE Factor { if($1^$3) semanticError("Type mismatch in expression");}
                        ;

Factor                  : IDENTIFIER
                        {
                            // Check if symbol table contains identifier
                            if(SYMTAB.find(yyval.stringValue) == SYMTAB.end()){
                                // Check if variable is declared before reference
                                semanticError("Variable "+string(yylval.stringValue)+" not declared");
                                $$=0;
                            }
                            else{
                                 // Check if variables is initialised when it is being used
                                if(!SYMTAB[yyval.stringValue].second)
                                    semanticError("Variable "+string(yylval.stringValue)+" used without initialisation"); 
                                $$ = SYMTAB[yylval.stringValue].first;
                            }
                        }
                        
                        | SUB INT {$$=0;}
                        | FLOAT {$$=1;}

                        | SUB FLOAT {$$=1;}

                        | INT {$$=0;}
                        | OPENPAREN Exp CLOSEPAREN {$$=$2;}
                
                        ;

Read                    : READ OPENPAREN IDList CLOSEPAREN
                            {

                                for (int i =0;i<int(DEC_LIST.size());i++){
                                    if(SYMTAB.find(DEC_LIST[i]) == SYMTAB.end()){
                                        // Check if variables is declared before reference
                                        semanticError("Variable "+DEC_LIST[i]+" not declared");
                                    }
                                    else SYMTAB[DEC_LIST[i]].second=1;     // initialize valid variables
                                }
                                DEC_LIST.clear();
                            }
                        ;

Write                   : WRITE OPENPAREN IDList CLOSEPAREN
                            {
                                for (int i =0;i<int(DEC_LIST.size());i++){
                                    if(SYMTAB.find(DEC_LIST[i]) == SYMTAB.end()){
                                        // Check if variables is declared before reference
                                        semanticError("Variable "+DEC_LIST[i]+" not declared");
                                    }
                                }
                                DEC_LIST.clear();
                            }
                        ;


For                     :  FOR IndexExp DO Body
                        ;

IndexExp                :  IDENTIFIER EQUAL Exp TO Exp
                            {
                                if($3==$5){
                                    if(SYMTAB.find(yyval.stringValue) != SYMTAB.end()){
                                        int type1 = SYMTAB[yyval.stringValue].first;

                                        // Check if type matches in assignment 
                                        if(type1==$3) SYMTAB[yyval.stringValue].second =1;
                                        else semanticError("Type mismatch, trying to assign "+types[$3] +" to "+types[type1]+" variable : "+yylval.stringValue);
                                        // else 
      
                                    }
                                    else{
                                        // Check if variables is declared when it is being referenced
                                        semanticError("Variable "+string(yylval.stringValue)+" not declared");
                                    }    
                                }
                                else {
                                    semanticError("Type mismatch, trying to iterate from type "+types[$3] +" to "+types[$5]);
                                }
                            }
                        ;

Body                    : Stmt
                        | BEGINPROG StmtList END
                        ;



%%

void yyerror(const char *s) {   // for syntax errors
    ERROR_FLAG=1;
    fprintf (stderr, "Line %d : %s\n", yylineno, s);
	
}
void semanticError(string s){ //for semantic errors
    ERROR_FLAG=1;
    cout <<"Line "<<to_string(yylineno)<<" : semantic error, "<<s<<endl;
}
void checkAndInsert(string x,int type1)          // inserts an identifier into the symbol table

{                          
    // Check if symbol table already contains the identifier
    if(SYMTAB.find(x)==SYMTAB.end()){
        // Insert into symbol table if all clear
        SYMTAB.insert({x,{type1,0}});
    }
    else{
        // Contains same identifier
        semanticError("Redeclaration of variable :  "+x);
    }
}

int main(int argc, char* argv[]) {
    
    if(argc ^ 2) {
        printf("Provide file name as an argument\n");
        exit(1);
    }

    yyin = fopen(argv[1], "r"); // Open file stream
    if(yyin == NULL) {  // if file not found
        printf("File does not exist\n");
        exit(1);
    }
    yyparse(); // Parsing

    fclose(yyin);  // close file stream

    if(!ERROR_FLAG) {           // parsed succssfully
        printf("Parsing completed successfully!\n");
        printf("SYMBOL TABLE:\n");
       
        cout.width(20);cout<<left<<"Identifier";
        cout.width(20);cout<<left<<"Type";
        cout<<"\n";
        for(auto x: SYMTAB){
           cout.width(20);cout<<left<<x.first;
           cout.width(20);cout<<left<<types[x.second.first];
           cout<<"\n";
        }

    }
    return 0;
    
}
