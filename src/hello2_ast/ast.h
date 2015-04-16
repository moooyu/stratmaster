#ifndef _AST_H
#define _AST_H
#include <stdarg.h>
/*typedef enum {ast_program_type ,ast_uselist_type ,ast_stratlist_type \
		,ast_strat_type ,ast_actionlist_type ,ast_order_type \
		,ast_order_item_type} nodeEnum;

*/

typedef struct{
    int type;  /*1ACCOUNT, 2DATAFEED, 3DATABASE, 4EXCHANGE*/
    char * name;
}use_define;

/********************AST NODES***********************/
typedef struct {
    char * name;
}ast_primary_expression;

typedef struct {
    ast_primary_expression *primary_expression;
}ast_postfix_expression;

typedef struct {
    ast_postfix_expression *postfix_expression;
}ast_unary_expression;

typedef struct {
    ast_unary_expression *unary_expression;
}ast_multiplicative_expression;

typedef struct {
    ast_multiplicative_expression *multiplicative_expression;
}ast_additive_expression;

typedef struct {
    ast_additive_expression *additive_expression;
}ast_relation_expression;

typedef struct {
    ast_relation_expression *relation_expression;
}ast_equality_expression;

typedef struct {
    ast_equality_expression *equality_expression;
}ast_logical_AND_expression;

typedef struct {
    ast_logical_AND_expression *logical_AND_expression;
}ast_logical_OR_expression;

typedef struct {
    int this_type;
    ast_unary_expression * unary_expression;
    ast_logical_OR_expression * logical_OR_expression;
}ast_assignment_expression;

typedef struct {
    int num_of_assignment_expressions;
    ast_assignment_expression ** assignment_expression;
}ast_expression;

typedef struct {
    ast_unary_expression *unary_expression;
    ast_logical_OR_expression *logical_OR_expression;
}ast_argument_expression_list;

typedef struct {
    int this_type;
    ast_argument_expression_list *argument_expression_list;
    ast_expression *expression;
}ast_set_statement;

typedef struct {
    ast_set_statement * set_statement;
}ast_statement;

typedef struct {
    int num_of_statement;
    ast_statement ** statement;
}ast_statement_list;

typedef struct {
    int type;
}ast_arithmetic_type;

typedef struct {
    ast_arithmetic_type * arithmetic_type;
}ast_type_specifier;

typedef struct{
    ast_type_specifier *type_specifier;
    char * name;
}type_spec;

typedef struct {
    ast_type_specifier * type_specifier;
    char *name;
}ast_variable_declaration;

typedef struct {
    int num_of_variable_declaration;
    ast_variable_declaration ** variable_declaration;
}ast_variable_declaration_list;

typedef struct {
    ast_variable_declaration_list * variable_declaration_list;
    ast_statement_list * statement_list;
}ast_compound_statement;

typedef struct {
    int num_of_type_specs;
    type_spec ** typespec;
}ast_target_list;

typedef struct {
    char * name;
    ast_target_list * target_list;
}ast_algorithm_parameter_list;

typedef struct {
    char * name;
    ast_algorithm_parameter_list * algorithm_parameter_list;
}ast_algorithm_header;

typedef struct {
    ast_algorithm_header * algorithm_header;
    ast_compound_statement * compound_statement;
}ast_algorithm_function;

typedef struct {
    int num_of_algos;
    ast_algorithm_function ** algorithm_function;
}ast_algorithm_list;

typedef struct {
    ast_algorithm_list * algorithm_list;
}ast_decision_list;

typedef struct {
    int num_of_use_defines;
    use_define ** usedefine;
}ast_use_others;

typedef struct{
    char * first_acc_name;
    ast_use_others * use_others;
}ast_use_list;

typedef struct {
    ast_use_list * use_list;
    ast_decision_list * decision_list;
}ast_program;

/*****************************************/

ast_program *
create_program(ast_use_list * use_list, ast_decision_list * decision_list);

ast_use_list *
create_use_list(char* first_acc_name, ast_use_others * use_others);

ast_use_others *
create_use_others();

ast_use_others *
add_use_others(ast_use_others *list, int type, char * name);

ast_decision_list *
create_decision_list(ast_algorithm_list * algorithm_list);

ast_algorithm_list *
create_algorithm_list();

ast_algorithm_list *
add_algorithm_list(ast_algorithm_list *list, ast_algorithm_function * algorithm_function);

ast_algorithm_function *
create_algorithm_function(ast_algorithm_header * algorithm_header,ast_compound_statement * compound_statement);

ast_algorithm_header *
create_algorithm_header(char * name, ast_algorithm_parameter_list * algorithm_parameter_list);

ast_algorithm_parameter_list *
create_algorithm_parameter_list(char* name, ast_target_list * target_list);

ast_target_list *
create_target_list();

ast_target_list *
add_target_list(ast_target_list * list, ast_type_specifier *type_specifier, char * name);

ast_compound_statement *
create_compound_statement(ast_variable_declaration_list * variable_declaration_list, ast_statement_list * statement_list);


ast_variable_declaration_list *
create_variable_declaration_list();

ast_variable_declaration_list *
add_variable_declaration_list(ast_variable_declaration_list * list, ast_variable_declaration * variable_declaration);


ast_variable_declaration *
create_variable_declaration(ast_type_specifier * type_specifier, char *name);

ast_type_specifier *
create_type_specifier(ast_arithmetic_type * arithmetic_type);

ast_arithmetic_type *
create_arithmetic_type(int type);

ast_statement_list *
create_statement_list();

ast_statement_list *
add_statement_list(ast_statement_list * list, ast_statement* statement);

ast_statement *
create_statement(ast_set_statement * set_statement);

ast_set_statement *
create_set_statement(int this_type, ast_argument_expression_list *argument_expression_list, ast_expression *expression);

ast_argument_expression_list *
create_argument_expression_list(ast_unary_expression *unary_expression,ast_logical_OR_expression *logical_OR_expression);

ast_unary_expression *
create_unary_expression(ast_postfix_expression *postfix_expression);


ast_postfix_expression *
create_postfix_expression(ast_primary_expression *primary_expression);

ast_primary_expression *
create_primary_expression(char * name);

ast_logical_OR_expression *
create_logical_OR_expression(ast_logical_AND_expression *logical_AND_expression);

ast_logical_AND_expression *
create_logical_AND_expression(ast_equality_expression *equality_expression);


ast_equality_expression *
create_equality_expression(ast_relation_expression *relation_expression);

ast_relation_expression *
create_relation_expression(ast_additive_expression *additive_expression);

ast_additive_expression *
create_additive_expression(ast_multiplicative_expression *multiplicative_expression);

ast_multiplicative_expression *
create_multiplicative_expression(ast_unary_expression *unary_expression);

ast_assignment_expression *
create_assignment_expression(int this_type, ast_unary_expression * unary_expression, ast_logical_OR_expression * logical_OR_expression);

ast_expression *
create_expression(ast_assignment_expression * assignment_expression);

ast_expression *
add_expression(ast_expression * list, ast_assignment_expression * assignment_expression);






































/**********end here can't be more ***********/  /*
typedef struct {
}ast_strategy_list;

typedef struct {
}ast_strategy;

typedef struct {
}ast_strategy_body;

typedef struct {
}ast_strategy_block;

typedef struct {
}ast_action_list;

typedef struct {
}ast_order;

typedef struct {
}ast_order_type;

typedef struct {
}ast_constraint_list;

typedef struct {
}ast_constraint;

typedef struct {
}ast_order_item;

typedef struct {
}ast_security;

typedef struct {
}ast_security_type;

typedef struct {
}ast_process_statement_list;

typedef struct {
}ast_process_statement;


ast_strategy_list *strategy_list;
ast_strategy *strategy;
ast_strategy_body *strategy_body;
ast_strategy_block *strategy_block;
ast_action_list *action_list;
ast_order *order;
ast_order_type *order_type;
ast_constraint_list *constraint_list;
ast_constraint *constraint;
ast_order_item *order_item;
ast_security *security;
ast_security_type *security_type;
ast_process_statement_list *process_statement_list;
ast_process_statement *process_statement;

 %type <ast_strategy_list> strategy_list;
 %type <ast_strategy> strategy;
 %type <ast_strategy_body> strategy_body;
 %type <ast_strategy_block> strategy_block;
 %type <ast_action_list> action_list;
 %type <ast_order> order;
 %type <ast_order_type> order_type;
 %type <ast_constraint_list> constraint_list;
 %type <ast_constraint> constraint;
 %type <ast_order_item> order_item;
 %type <ast_security> security;
 %type <ast_security_type> security_type;
 %type <ast_process_statement_list> process_statement_list;
 %type <ast_process_statement> process_statement;


strategy_list   : strategy_list strategy	{$$ = add_strategy_list($1, $2);}
 | strategy                  {$$ = create_strategy_list($1);}
 ;
 
 strategy        : STRATEGY IDENTIFIER  '{' strategy_body '}' {$$ = create_strategy($2, $4);}
 ;
 
 strategy_body   : variable_declaration_list strategy_block  {$$ = create_strategy_body($1, $2);}
 ;
 
 strategy_block  : action_list process_statement_list  {$$ = create_strategy_block(0, $1, $2);}
 | action_list                         {$$ = create_strategy_block(1, $1);}
 | process_statement_list              {$$ = create_strategy_block(1, $1);}
 |
;

action_list     : action_list order     { $$ = add_action_list($1, $2);}
| order                 { $$ = create__action_list($1);}
;

order           : order_type '{' constraint_list'}'  {$$ = create_order($1, $3);}
;

order_type      : BUY  { printf($$ = 0;}
                                | SELL  { printf($$ = 1;}
                                                 ;
                                                 
                                                 constraint_list : constraint_list constraint  {$$ = add_constraint_list($1, $2);}
                                                 | constraint  {$$ = create_constraint_list($1);}
                                                 ;
                                                 
                                                 constraint      : WHAT ':' order_item  {$$ = create_constraint($3);}
                                                 ;
                                                 
                                                 order_item      : security '.'  AMOUNT '('NUMBER')' '.' PRICE '(' PRICEXP  ')' ';'  {$$ = create_order_item($1, $<int_val>5, $<str>8);}
                                                 ;
                                                 
                                                 security        :security_type '(' IDENTIFIER ')'  {$$ = create_security($1, $3);}
                                                 ;
                                                 
                                                 security_type   : EQTY {$$ = 0;}
                                                 | BOND {$$ = 1;}
                                                 ;
                                                 
                                                 process_statement_list  : process_statement_list process_statement {$$ = add_process_statement_list($1, $2);}
                                                 | process_statement {$$ = create_process_statement_list($1);}
                                                 ;
                                                 
                                                 process_statement   : WHEN '(' expression ')' '{' action_list '}' UNTIL '(' expression ')' {$$ = create_process_statement(0, $3, $6, $10);}
                                                 | WHEN '(' expression ')' '{' action_list '}'      {$$ = create_process_statement(1, $3, $6);}
                                                 ;*/


#define DEBUG 1

#ifdef DEBUG
#define PRINT(x)  do { if (DEBUG) dbg_printf x; } while (0)
#else
#define PRINT(x) do {} while(0)
#endif

void dbg_printf(const char *fmt, ...);
#endif




