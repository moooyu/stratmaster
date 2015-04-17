#  StratMaster Hello World makefile

#Name of the binary make will create
PROGRAM = strat.out 
#Name of lex file
LEXFILE = hello.l
#Name of yacc file
YACCFILE = hello.y
#Compiler flags
CFLAGS = -g -Wall
#Compiler
CC = gcc
#Lexer and parser
LEXER = lex
PARSER = yacc
LDFLAGS = -ly -pthread `pkg-config --cflags --libs glib-2.0`
LEXFLAGS = --yylineno
YACCFLAGS = -d

# all of the .o files that the program needs
OBJECTS = y.tab.o lex.yy.o inter.o ast.o rtfunc.o symtab.o

# Make strat.out
${PROGRAM} : ${OBJECTS}
	${CC} ${CFLAGS} ${OBJECTS} -o ${PROGRAM} ${LDFLAGS}

# Turn the parser.y file into y.tab.c using "yacc"
y.tab.c : ${YACCFILE}
	${PARSER} ${YACCFLAGS} ${YACCFILE}
y.tab.o: y.tab.c
	${CC} -g -c y.tab.c ${LDFLAGS}
y.tab.h: y.tab.c
parser.h: y.tab.h

#  Turn the scanner.l file into lex.yy.c using "lex"
lex.yy.c : ${LEXFILE}
	${LEXER} ${LEXFLAGS} ${LEXFILE}
lex.yy.o: lex.yy.c
	${CC} -g -c lex.yy.c

inter.o: inter.c
	${CC} -g -c inter.c

ast.o: ast.c
	${CC} -g -c ast.c

rtfunc.o: rtfunc.c
	${CC} -g -c rtfunc.c 

symtab.o: symtab.c
	${CC} -g -c symtab.c ${LDFLAGS}

clean:
	/bin/rm -f *.o *.out lex.yy.c y.tab.c y.tab.h core *~



