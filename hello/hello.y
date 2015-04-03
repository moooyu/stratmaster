%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

typedef enum { BUY_ORDER, SELL_ORDER } order_type;

/* Structure for an order */
typedef struct Order {
	char sym[16];
	int  amt;
	char price[16];
	int  type;
} Order;

/* Functions to interface with Lex */
int yylex(void);
void yyerror (char *s);

/* Temporary confirmation function */
void emit_order(int t);

/* Variable declarations */
Order my_order;
char stratname[32];

/* For debugging: print line numbers on error */
extern int yylineno;
%}

/* Data types for yylval in Lexer */
%union {
   char str[32];
   int  int_val;
};

%start program
%token STRATEGY BUY SELL WHAT USE ACCOUNT EQTY AMOUNT PRICE
%token <str> IDENTIFIER PRICEXP
%token <int_val> NUMBER

%%
program : use_list strategy_list	{   }
	;

use_list : USE ACCOUNT IDENTIFIER	{ printf("Using account: %s\n\n", $<str>3); }
	;

strategy_list : strategy		{         }
	| strategy_list strategy
	;

strategy : STRATEGY IDENTIFIER  
	 		{ strcpy(stratname, $<str>2);  } 
	   '{' action_list '}' 
	 ;

action_list : order		{     }
	| action_list order
	;

order 	: order_type '{' WHAT ':' order_item ';' '}'  { }
	;

order_type : BUY	{ my_order.type = BUY_ORDER; } 
	| SELL		{ my_order.type = SELL_ORDER;}
	;

order_item : EQTY '(' IDENTIFIER ')''.' AMOUNT '('NUMBER')' '.' PRICE '(' PRICEXP  ')'  {strcpy(my_order.sym, $<str>3); my_order.amt = $<int_val>8;  strcpy(my_order.price, $<str>13); emit_order(my_order.type); } 

%%
void emit_order(int t)
{
   /*  Get local timestamp */
   char buf[64];
   time_t local_t;
   struct tm *tmp;
   time(&local_t);
   tmp = localtime(&local_t);
   strftime(buf, 64, "%Y-%m-%d %T",tmp);
   
   /* Determine order type */
   char *order_type = "";
   switch(t)
   {
	case BUY_ORDER: order_type = "BOUGHT"; break; 
	case SELL_ORDER: order_type = "SOLD"; break; 
	default: order_type = "DID SOMETHING ELSE";
   }

   /* Print confirmation */
   printf("++++++++++++++++++STRATMASTER CONFIRMATION+++++++++++++++++\n");
   printf("[%s] YOU %s: %d SHARES OF %s AT USD %s\n", buf, order_type, my_order.amt, my_order.sym, my_order.price);
   printf(" >>>>>> ORDER PLACED BY %s\n", stratname);
   printf("++++++++++++++++++END CONFIRMATION+++++++++++++++++++++++++\n\n");
}

void yyerror(char * s) 
{
   fprintf(stderr, "line %d: %s\n", yylineno, s);
}

int main(void) 
{
   yyparse();
   return 0;
}
