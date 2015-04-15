%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "ast.h"
#include "symtab.h"

/* Functions to interface with Lex */
int yylex(void);
void yyerror (char *s);

/* For debugging: print line numbers on error */
extern int yylineno;
struct symbol_table *top;
struct symbol_table *parent;

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
%type <program> program
%type <uselist> use_list
%type <stratlist> strategy_list 
%type <strat> strategy 
%type <actionlist> action_list 
%type <order> order
%type <order_item> order_item

%%
program : 			{ parent = NULL;
				top = symbol_table_create(parent); }			 
	use_list strategy_list	{ $$ = create_node_program($2, $3);
				print_ast($$);ex_ast($$);}
	;

use_list : USE ACCOUNT IDENTIFIER	{$$ = create_node_uselist($3);
					symbol_table_put_value(top, ACCT_SYM, $3, $$); }
	;

strategy_list : strategy		{  $$ = create_node_stratlist($1);}
	;

strategy : STRATEGY IDENTIFIER  '{'	{ parent = top;
					top = symbol_table_create(parent); }
	action_list '}'			{ $$ = create_node_strat($2, $5);
					top = parent;
					symbol_table_put_value(top, STRAT_SYM, $2, $$); }
	 ;

action_list : order		{ $$ = create_node_actionlist($1);	}
	| action_list order	{ $$ = add_order($1, $2);		}
	;

order 	: order_type '{' WHAT ':' order_item ';' '}'  { $$ = create_node_order($1, $5); }
	;

order_type : BUY	{ $$ = 1; }
	| SELL		{ $$ = 0;}
	;

order_item : EQTY '(' IDENTIFIER ')''.' AMOUNT '('NUMBER')' '.' PRICE '(' PRICEXP  ')'
		{
			$$ = create_node_order_item($<str>3, $<int_val>8, $<str>13);
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

