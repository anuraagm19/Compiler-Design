all: 190101110_lexer.l  190101110_parser.y
	flex 190101110_lexer.l
	yacc -d 190101110_parser.y
	g++ y.tab.c lex.yy.c -o parser

clean:
	rm -f parser lex.yy.c y.tab.c y.tab.h tokenizer