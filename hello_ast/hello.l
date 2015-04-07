%{
#include <stdlib.h>
#include <string.h>
#include "y.tab.h"
#include "hello.h"
void yyerror(char *);
%}

comment		\/\*[^\*]*\*+([^\/\*][^\*]*\*+)*\/     // to do
digit		[0-9]
number		[0-9]+
id		[a-zA-Z][_a-zA-Z0-9]*
ws		[ \t]
wsn		[ \t\n]


%%
\S\T\R\A\T\E\G\Y		{ return(STRATEGY); }
\B\U\Y				{ return(BUY); }
\S\E\L\L			{ return(SELL); }
\W\H\A\T 			{ return(WHAT); }
\U\S\E 				{ return(USE); }
\A\C\C\O\U\N\T			{ return(ACCOUNT); }
\A\M\O\U\N\T			{ return(AMOUNT); }
\P\R\I\C\E			{ return(PRICE); }
\E\Q\T\Y			{ return(EQTY); }
{number}.{digit}{digit}		{ strcpy(yylval.str, yytext); return(PRICEXP); }
{number}			{ yylval.int_val = atoi(yytext); return(NUMBER); }
{id}				{ strcpy(yylval.str, yytext); return(IDENTIFIER); }
[:;{}().,]			{ return *yytext; }
{comment}			/* this is a comment; do nothing */ ;
{wsn}+				/* do nothing */  ;
.				yyerror("invalid input");
%%
int yywrap(void) { return 1; }
