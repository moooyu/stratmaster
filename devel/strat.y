%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>



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
    double fp_val;
};


%start program
%token STRATEGY ALGORITHM FUNCTION USE ACCOUNT DATAFEED DATABASE EXCHANGE
%token BUY SELL WHAT AMOUNT PRICE EQTY SET IF WHERE WHEN UNTIL RETURNS
%token INT LONG DOUBLE BOOLEAN SECURITY VOID CURRENCY POSITION IS ISNOT
%token TRUE FALSE OR AND NOT WHILE
%token USD EUR JPY POS SEC AMT PRC AVAIL_CASH NEXT
%token <str> IDENTIFIER PRICEXP
%token <int_val> INTEGER
%token <fp_val> FLOATPT



%%
program		: { fprintf(stdout, "STARTING PROGRAM\n"); } use_list process_list { fprintf(stdout, "ENDING PROGRAM\n");  }
	 	;

use_list	: USE  variable_declaration			{ fprintf(stdout, "USE statement\n"); }	
		| use_list USE variable_declaration 		{ fprintf(stdout, "USE statement\n"); }
		;

process_list	: strategy_list
	     	| function_list strategy_list
		| algorithm_list strategy_list
		| function_list algorithm_list strategy_list
		;	     	

function_list	: function_definition			{ fprintf(stdout, "Function\n"); }
		| function_list function_definition	{ fprintf(stdout, "Function\n"); }
		;

algorithm_list 	: algorithm_definition			{ fprintf(stdout, "Algorithm\n"); }
		| algorithm_list algorithm_definition	{ fprintf(stdout, "Algorithm\n"); }
		;

strategy_list 	: strategy_definition				{ fprintf(stdout, "Strategy\n"); }
		| strategy_list strategy_definition		{ fprintf(stdout, "Strategy\n"); }
		;

function_definition : function_header compound_statement
		;

function_header : FUNCTION IDENTIFIER '(' parameter_list ')' func_return	{ fprintf(stdout, "Function HDR\n"); }
		;

func_return	: RETURNS type_specifier
		;

algorithm_definition : algorithm_header compound_statement
		;

algorithm_header : ALGORITHM IDENTIFIER '(' parameter_list ')'		{ fprintf(stdout, "Algo Hdr\n"); }

parameter_list	: type_specifier IDENTIFIER				{ fprintf(stdout, "Param List\n"); }	
	       	| type_specifier '#' IDENTIFIER				{ fprintf(stdout, "Param List\n"); }
		| parameter_list ',' type_specifier '#' IDENTIFIER	{ fprintf(stdout, "Param List\n"); }
		| /* empty */
		;

strategy_definition : STRATEGY IDENTIFIER '{' strategy_body '}'
	  	;

strategy_body	: variable_declaration_list statement_list strategy_block
	      	| variable_declaration_list strategy_block 
		| strategy_block
		| /* empty */
		;

strategy_block	: action_list
		| process_statement_list
		;

process_statement_list : process_statement
		| process_statement_list process_statement
		;

process_statement : WHEN '(' expression ')' '{' process_body '}' UNTIL '(' expression ')' { fprintf(stdout, "Process statement\n"); } 
		| WHEN '(' expression ')' '{' process_body '}'				{ fprintf(stdout, "Process statement\n"); }
		;

process_body	: action_list
	     	| action_list statement_list
		;

action_list	: order_type '{' constraint_list '}'				{ fprintf(stdout, "Action List\n"); }
		| action_list order_type '{' constraint_list '}'		{ fprintf(stdout, "Action List\n"); }
		;

constraint_list	: constraint
		| constraint_list constraint
		;

constraint	: WHAT ':' order_item ';'
	   	| WHERE ':' exchange_item ';'
		;

order_item	: security '.' AMOUNT '(' INTEGER ')''.' PRICE '(' price_expr ')' 	{ fprintf(stdout, "Order Item\n"); }
		;

exchange_item	: EXCHANGE IDENTIFIER
		;

price_expr	: PRICEXP
		| currency
		| IDENTIFIER
		;

variable_declaration_list : variable_declaration
		| variable_declaration_list variable_declaration
		;

variable_declaration : type_specifier IDENTIFIER ';'
		;

type_specifier	: INT
		| LONG
		| DOUBLE
		| BOOLEAN
		| SECURITY
		| PRICE
		| use_type
		| VOID
		;

use_type	: ACCOUNT
		| DATAFEED
		| DATABASE
		| EXCHANGE
		;

security_type 	: EQTY
		;

currency_type	: USD
		| EUR
		| JPY
		;

order_type	: BUY
		| SELL
		;

statement	: expression_statement
	  	| compound_statement
		| selection_statement
		| iteration_statement
		| set_statement
		;

expression_statement : expression ';'
		| ';'
		;

compound_statement : '{' variable_declaration_list statement_list '}'
		| '{' statement_list '}'
		| '{' '}'
		;

statement_list	: statement
		| statement_list statement
		;

selection_statement : IF '(' expression ')' statement
		;

iteration_statement : WHILE '(' expression ')' statement
		;

set_statement 	: SET '{' argument_expression_list '}' IF ':' '{' expression '}'
		;

expression	: assignment_expression
            	| expression ',' assignment_expression
		;

assignment_expression : logical_OR_expression
            	| unary_expression '=' logical_OR_expression
		;

logical_OR_expression :  logical_AND_expression 
            	| logical_OR_expression OR logical_AND_expression
		;

logical_AND_expression : equality_expression
           	| logical_AND_expression AND equality_expression
		;

equality_expression : relation_expression 
		| equality_expression IS relation_expression
		| equality_expression ISNOT relation_expression
		;

relation_expression :  additive_expression
	        | relation_expression '<' additive_expression
       		| relation_expression '>' additive_expression
       		| relation_expression '<''=' additive_expression
        	| relation_expression '>''=' additive_expression
		;

additive_expression : multiplicative_expression
        	| additive_expression '+' multiplicative_expression
	        | additive_expression '-' multiplicative_expression
		;

multiplicative_expression : unary_expression
	        | multiplicative_expression '*' unary_expression
        	| multiplicative_expression '/' unary_expression
		;
	
unary_expression : postfix_expression 
		| unary_operator unary_expression
		;

unary_operator 	: '-' 
		| '#' 
		| NOT
		;

postfix_expression : primary_expression
	        | postfix_expression '(' ')'
       		| postfix_expression '(' argument_expression_list ')'
		;

argument_expression_list : assignment_expression 
		| argument_expression_list ',' assignment_expression
		;

primary_expression : type_name
		| INTEGER
		| PRICEXP
		| security
		| currency
		| position
		| TRUE
		| FALSE
		| '(' expression ')'
		;

type_name	: IDENTIFIER
	  	| type_name '.' IDENTIFIER
		| type_name '.' attribute
		;

position 	: POS '(' IDENTIFIER ')'
	  	;

security	: security_type '(' IDENTIFIER ')'
		;

currency	: currency_type '(' PRICEXP ')'
	 	;

attribute	: SEC
	  	| AMT
		| PRC
		| POS
		| AVAIL_CASH
		| NEXT
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



