syntax: calculator.y
	bison -d calculator.y

lexer: calculator.l
	lex calculator.l

calculator: syntax lexer utils.c syntax_tree.c
	gcc lex.yy.c calculator.tab.c utils.c syntax_tree.c -ocalculator -lm

clean:
	rm calculator lex.yy.c calculator.tab.c calculator.tab.h