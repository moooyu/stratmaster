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

    ast_program *program;
    ast_use_list *use_list;
    ast_use_others *use_others;
    ast_decision_list *decision_list;
    ast_algorithm_list *algorithm_list;
    ast_algorithm_function *algorithm_function;
    ast_algorithm_header *algorithm_header;
    ast_algorithm_parameter_list *algorithm_parameter_list;
    ast_target_list *target_list;
    ast_compound_statement *compound_statement;
    ast_variable_declaration_list *variable_declaration_list;
    ast_variable_declaration *variable_declaration;
    ast_type_specifier *type_specifier;
    ast_arithmetic_type *arithmetic_type;
    ast_statement_list *statement_list;
    ast_statement *statement;
    ast_set_statement *set_statement;
    ast_argument_expression_list *argument_expression_list;
    ast_unary_expression *unary_expression;
    ast_postfix_expression *postfix_expression;
    ast_primary_expression *primary_expression;
    ast_logical_OR_expression *logical_OR_expression;
    ast_logical_AND_expression *logical_AND_expression;
    ast_equality_expression *equality_expression;
    ast_relation_expression *relation_expression;
    ast_additive_expression *additive_expression;
    ast_multiplicative_expression *multiplicative_expression;
    ast_expression *expression;
    ast_assignment_expression *assignment_expression;

};


%start program
%token STRATEGY BUY SELL WHAT USE ACCOUNT EQTY AMOUNT PRICE
%token DATAFEED DATABASE ALGORITHM EXCHANGE
%token SET IF
%token WHERE WHEN UNTIL BOND
%token <str> IDENTIFIER PRICEXP
%token <int_val> NUMBER
%type <program> program;
%type <use_list> use_list;
%type <use_others> use_others;
%type <decision_list> decision_list;
%type <algorithm_list> algorithm_list;
%type <algorithm_function> algorithm_function;
%type <algorithm_header> algorithm_header;
%type <algorithm_parameter_list> algorithm_parameter_list;
%type <target_list> target_list;
%type <compound_statement> compound_statement;
%type <variable_declaration_list> variable_declaration_list;
%type <variable_declaration> variable_declaration;
%type <type_specifier> type_specifier;
%type <arithmetic_type> arithmetic_type;
%type <statement_list> statement_list;
%type <statement> statement;
%type <set_statement> set_statement;
%type <argument_expression_list> argument_expression_list;
%type <unary_expression> unary_expression;
%type <postfix_expression> postfix_expression;
%type <primary_expression> primary_expression;
%type <logical_OR_expression> logical_OR_expression;
%type <logical_AND_expression> logical_AND_expression;
%type <equality_expression> equality_expression;
%type <relation_expression> relation_expression;
%type <additive_expression> additive_expression;
%type <multiplicative_expression> multiplicative_expression;
%type <expression> expression;
%type <assignment_expression> assignment_expression;



%%
program         : use_list decision_list 	{ $$ = create_program($1, $2);}
                ;

use_list        : USE ACCOUNT IDENTIFIER use_others {$$ = create_use_list($<str>3, $4);}
                ;

use_others      : use_others USE ACCOUNT IDENTIFIER {printf("ACC\n");$$ = add_use_others($1, 1, $<str>4);}
                | use_others USE DATAFEED IDENTIFIER{printf("DF\n");$$ = add_use_others($1, 2, $<str>4);}
                | use_others USE DATABASE IDENTIFIER{printf("DB\n");$$ = add_use_others($1, 3, $<str>4);}
                | use_others USE EXCHANGE IDENTIFIER{printf("EX\n");$$ = add_use_others($1, 4, $<str>4);}
| /*NULL*/                          {$$ = create_use_others();}
                ;

decision_list   : algorithm_list  {$$ = create_decision_list($1);}
                ;

algorithm_list  : algorithm_list algorithm_function  {$$ = add_algorithm_list($1, $2);}
                | /*NULL*/                           {$$ = create_algorithm_list();}
                ;

algorithm_function  : algorithm_header '{' compound_statement '}'  {$$ = create_algorithm_function($1, $3);}
                    ;

algorithm_header    : ALGORITHM IDENTIFIER '(' algorithm_parameter_list ')'  {$$ = create_algorithm_header($<str>2, $4);}
                    ;

algorithm_parameter_list    :DATAFEED IDENTIFIER target_list  {$$ = create_algorithm_parameter_list($<str>2, $3);}
                            ;

target_list     : target_list type_specifier IDENTIFIER  {$$ = add_target_list($1, $2, $<str>3);}
                |/*NULL*/                                {$$ = create_target_list();}
                ;

compound_statement  : variable_declaration_list statement_list {$$ = create_compound_statement($1, $2);}
                    ;

variable_declaration_list   : variable_declaration_list variable_declaration {$$ = add_variable_declaration_list($1, $2);}
                            | /*NULL*/ {$$ = create_variable_declaration_list();}
                            ;

variable_declaration: type_specifier IDENTIFIER ';'{$$ = create_variable_declaration($1, $<str>2);}
                    ;

type_specifier  : arithmetic_type {$$ = create_type_specifier($1);}
                ;

arithmetic_type : PRICE {$$ = create_arithmetic_type(1);}
                ;

statement_list  : statement_list statement {$$ = add_statement_list($1, $2);}
                | /*NULL*/                  {$$ = create_statement_list();}
                ;

statement       : set_statement {$$ = create_statement($1);}
                ;

set_statement   : SET '{' argument_expression_list '}' IF ':' '{' expression '}' {$$ = create_set_statement(0, $3, $8);}
                | SET '{' argument_expression_list '}'                           {$$ = create_set_statement(1, $3, NULL);}
                ;

argument_expression_list: unary_expression '=' logical_OR_expression {$$ = create_argument_expression_list($1, $3);}
                ;

unary_expression: postfix_expression {$$ = create_unary_expression($1);}
                ;

postfix_expression  : primary_expression {$$ = create_postfix_expression($1);}
                    ;

primary_expression  : IDENTIFIER  {$$ = create_primary_expression($<str>1);}
                    ;

logical_OR_expression   : logical_AND_expression  {$$ = create_logical_OR_expression($1);}
                        ;

logical_AND_expression  : equality_expression   {$$ = create_logical_AND_expression($1);}
                        ;

equality_expression : relation_expression  {$$ = create_equality_expression($1);}
                    ;

relation_expression : additive_expression  {$$ = create_relation_expression($1);}
                    ;

additive_expression : multiplicative_expression  {$$ = create_additive_expression($1);}
                    ;

multiplicative_expression   : unary_expression  {$$ = create_multiplicative_expression($1);}
                    ;

expression  : assignment_expression                 {$$ = create_expression($1);}
            | expression ',' assignment_expression  {$$ = add_expression($1, $3);}
            ;

assignment_expression   : logical_OR_expression                         {$$ = create_assignment_expression(0, NULL, $1);}
                        | unary_expression '=' logical_OR_expression    {$$ = create_assignment_expression(1, $1, $3);}
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



