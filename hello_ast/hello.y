%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "ast.h"

/* Functions to interface with Lex */
int yylex(void);
void yyerror (char *s);

/* For debugging: print line numbers on error */
extern int yylineno;



%}

/* Data types for yylval in Lexer */
%union {
   char str[32];
   int  int_val;
   nodeType *nPtr;
   ast_program *program;
   ast_uselist *uselist;
   ast_stratlist *stratlist;
   ast_strat *strat;
   ast_actionlist *actionlist;
   ast_order *order;
   ast_order_item *order_item;
};

%start program
%token STRATEGY BUY SELL WHAT USE ACCOUNT EQTY AMOUNT PRICE
%token <str> IDENTIFIER PRICEXP
%token <int_val> NUMBER
%type <int_val> order_type
%type <nPtr> strategy action_list order order_item
%type <program> program
%type <uselist> use_list
%type <stratlist> strategy_list 
%%
program : use_list strategy_list	{ $$ = create_node_program($1, $2); ex_ast($$);}
	;

use_list : USE ACCOUNT IDENTIFIER	{ $$ = create_node_uselist($3);}
	;

strategy_list : strategy		{  $$ = create_node_stratlist($1);}
	;

strategy : STRATEGY IDENTIFIER  '{' action_list '}'  { $$ = create_node_strat($2, $4);}
	 ;

action_list : order		{  $$ = create_node_actionlist($1);   }
	;

order 	: order_type '{' WHAT ':' order_item ';' '}'  { $$ = create_node_order($1, $5); }
	;

order_type : BUY	{ $$ = 1; }
	| SELL		{ $$ = 0;}
	;

order_item : EQTY '(' IDENTIFIER ')''.' AMOUNT '('NUMBER')' '.' PRICE '(' PRICEXP  ')'
		{
			$$ = create_node_order_item($<str>3, $8, $13);
		} 
	;

%%

void yyerror(char * s) 
{
   fprintf(stderr, "line %d: %s\n", yylineno, s);
}

int main(void) 
{
   yyparse();
   return 0;
}
/*
struct nodeTypeTag *order_node(Order *myorder) {
	nodeType *p;
	p = malloc(sizeof(nodeType));
	if (!p)
		yyerror("out of memory");
	
	p->order = malloc (sizeof(Order));
	if (!p)
		yyerror("out of memory");
	p->type = typeOrder;
	memcpy (p->order, myorder, sizeof(Order));
	
	return p;
}
 */
