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
   /* nodeType *nPtr;
   ast_program *program;
   ast_uselist *uselist;
   ast_stratlist *stratlist;
   ast_strat *strat;
   ast_actionlist *actionlist;
   ast_order *order;
   ast_order_item *order_item;*/
};

%start program
%token STRATEGY BUY SELL WHAT USE ACCOUNT EQTY AMOUNT PRICE
%token DATAFEED DATABASE ALGORITHM EXCHANGE
%token SET IF VOID
%token WHERE WHEN UNTIL BOND
%token <str> IDENTIFIER PRICEXP
%token <int_val> NUMBER


%%
program         : use_list decision_list strategy_list	{}
                ;

use_list        : USE ACCOUNT IDENTIFIER use_others {}
                ;

use_others      : use_others USE ACCOUNT IDENTIFIER  {}
                | use_others USE DATAFEED IDENTIFIER  {}
                | use_others USE DATABASE IDENTIFIER  {}
                | use_others USE EXCHANGE IDENTIFIER  {}
                | /*NULL*/
                ;

decision_list   : algorithm_list  {}
                ;

algorithm_list  : algorithm_list algorithm_function  {}
                | /*NULL*/
                ;

algorithm_function  : algorithm_header '{' compound_statement '}'  {}
                    ;

algorithm_header    : ALGORITHM IDENTIFIER '(' algorithm_parameter_list ')'  {}
                    ;

algorithm_parameter_list    :DATAFEED IDENTIFIER target_list  {}
                            ;

target_list     : target_list type_specifier IDENTIFIER  {}
                |/*NULL*/
                ;

compound_statement  : variable_declaration_list statement_list {}
                    ;

variable_declaration_list   : variable_declaration_list variable_declaration
                            | /*NULL*/
                            ;

variable_declaration: type_specifier IDENTIFIER ';'
                    ;

type_specifier  : arithmetic_type
                | VOID
                ;

arithmetic_type : PRICE
                ;

statement_list  : statement_list statement
                | /*NULL*/
                ;

statement       : set_statement
                ;

set_statement   : SET '{' argument_expression_list '}' IF ':' '{' expression '}'
                | SET '{' argument_expression_list '}'
                ;

argument_expression_list: unary_expression '=' logical_OR_expression
                ;

unary_expression: postfix_expression
                ;

postfix_expression  : primary_expression
                    ;

primary_expression  : IDENTIFIER
                    ;

logical_OR_expression   : logical_AND_expression
                        ;

logical_AND_expression  : equality_expression
                        ;

equality_expression : relation_expression
                    ;

relation_expression : additive_expression
                    ;

additive_expression : multiplicative_expression;
                    ;

multiplicative_expression   : unary_expression
                    ;

expression  : assignment_expression
            | expression ',' assignment_expression
            ;

assignment_expression   : logical_OR_expression
                        | unary_expression '=' logical_OR_expression
                        ;


strategy_list   : strategy_list strategy	  {}
                | strategy
                ;

strategy        : STRATEGY IDENTIFIER  '{' strategy_body '}'  {}
                ;

strategy_body   : variable_declaration_list strategy_block  {}
                ;

strategy_block  : action_list process_statement_list
                | action_list
                | process_statement_list
                | /*NULL*/
                ;

action_list     : action_list order
                | order
                ;

order           : order_type '{' constraint_list'}'  {}
                ;

order_type      : BUY  { printf("BUY\n");}
                | SELL  { printf("SELL\n");}
                ;

constraint_list : constraint_list constraint
                | constraint
                ;

constraint      : WHAT ':' order_item
                ;

order_item      : security '.'  AMOUNT '('NUMBER')' '.' PRICE '(' PRICEXP  ')' ';'  {printf("order_item\n");}
                ;

security        :security_type '(' IDENTIFIER ')'
                ;

security_type   : EQTY
                | BOND
                ;

process_statement_list  : process_statement_list process_statement
                        | process_statement
                        ;

process_statement   : WHEN '(' expression ')' '{' action_list '}' UNTIL '(' expression ')'
                    | WHEN '(' expression ')' '{' action_list '}'
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



