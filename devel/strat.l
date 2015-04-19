%{
#include <stdlib.h>
#include <string.h>

#include "y.tab.h"
void yyerror(char *);
%}

comment		\/\*[^\*]*\*+([^\/\*][^\*]*\*+)*\/
digit		[0-9]
number		[0-9]+
id		[a-zA-Z][_a-zA-Z0-9]*
ws		[ \t]
wsn		[ \t\n]


%%
\S\T\R\A\T\E\G\Y		{ return(STRATEGY); }
\A\L\G\O\R\I\T\H\M  		{ return(ALGORITHM);}
\F\U\N\C\T\I\O\N  		{ return(FUNCTION);}
\U\S\E 				{ return(USE); }
\A\C\C\O\U\N\T			{ return(ACCOUNT); }
\D\A\T\A\F\E\E\D    		{ return(DATAFEED);}
\D\A\T\A\B\A\S\E    		{ return(DATABASE);}
\E\X\C\H\A\N\G\E    		{ return(EXCHANGE);}
\B\U\Y				{ return(BUY); }
\S\E\L\L			{ return(SELL); }
\W\H\A\T 			{ return(WHAT); }
\A\M\O\U\N\T			{ return(AMOUNT); }
\P\R\I\C\E			{ return(PRICE); }
\E\Q\T\Y			{ return(EQTY); }
\S\E\T             		{ return(SET);}
\I\F                		{ return(IF);}
\W\H\E\R\E          		{ return(WHERE);}
\W\H\E\N           	 	{ return(WHEN);}
\U\N\T\I\L          		{ return(UNTIL);}
\R\E\T\U\R\N\S      		{ return(RETURNS);}
\I\N\T				{ return(INT); }
\L\O\N\G			{ return(LONG); }
\D\O\U\B\L\E  			{ return(DOUBLE); }
\B\O\O\L\E\A\N			{ return(BOOLEAN); }
\S\E\C\U\R\I\T\Y		{ return(SECURITY); }
\V\O\I\D			{ return(VOID); }
\C\U\R\R\E\N\C\Y		{ return(CURRENCY); }
\P\O\S\I\T\I\O\N		{ return(POSITION); }
\I\S				{ return(IS); }
\I\S\N\O\T			{ return(ISNOT); }
\T\R\U\E			{ return(TRUE); }
\F\A\L\S\E			{ return(FALSE); }
\O\R				{ return(OR); }
\A\N\D				{ return(AND); }
\N\O\T				{ return(NOT); }
\W\H\I\L\E			{ return(WHILE); }
\U\S\D				{ return(USD); }
\E\U\R				{ return(EUR); }
\J\P\Y				{ return(JPY); }
\P\O\S				{ return(POS); }
\S\E\C				{ return(SEC); }
\A\M\T				{ return(AMT); }
\P\R\C				{ return(PRC); }
\A\V\A\I\L\_\C\A\S\H		{ return(AVAIL_CASH); }
\N\E\X\T			{ return(NEXT); }
{number}"."{digit}{digit}	{ strcpy(yylval.str, yytext); return(PRICEXP); }
{digit}*"."{digit}		{ yylval.fp_val  = atof(yytext); return(FLOATPT); }
{number}			{ yylval.int_val = atoi(yytext); return(INTEGER); }
{id}				{ strcpy(yylval.str, yytext); return(IDENTIFIER); }
[:;{}().,=#<>+-/*]		{ return *yytext; }
{comment}			/* this is a comment; do nothing */ ;
{wsn}+				/* do nothing */  ;
.				yyerror("invalid input");
%%
int yywrap(void) { return 1; }

