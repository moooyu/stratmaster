%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "symtab.h"
#include "ast.h"
#include "runtime.h"
#include "check.h"
/* Functions to interface with Lex */
int yylex(void);
void yyerror (char *s);
/* Other functions declarations */
void strat_error_msg(char *msg, int error_t, int lineno);
/* For debugging: print line numbers on error */
extern int yylineno;
void semantic_check(int operator, int nopr);
/* Global Variables */
struct symbol_table *top;
struct symbol_table *parent;
struct Stack *stack;
///ast_program * root;
%}

/* Data types for yylval in Lexer */
%union {
    char str[32];
    int  int_val;
    double fp_val;
    ast_currency *currency;
    ast_security *security;
    ast_position *position;     
	ast_order_item *order_item;
	ast_order_item *constraint;
	ast_order_item * constraint_list;
	ast_action_list *process_body;
	ast_action_list *action_list;
	ast_strategy *strategy;
	ast_strategy_list *strategy_list;
	ast_algorithm_list *algorithm_list;
	ast_program *program;
	ast_exp *exp;
	ast_process_statement * process_statement;
	ast_process_statement_list * process_statement_list;	
	ast_algorithm_header *algorithm_header;
	ast_algorithm *algorithm_function;
    	ast_statement_list * statement_list;
	ast_parameter_list *parameter_list;
ast_strategy_block *strategy_block;
ast_statement *statement;
};


%start program
%token STRATEGY ALGORITHM FUNCTION USE 
%token BUY SELL WHAT AMOUNT EQTY PRICE SET IF WHERE WHEN UNTIL RETURNS
%token POSITION IS ISNOT
%token TRUE_S FALSE_S OR AND NOT WHILE
%token POS SEC AMT PRC AVAIL_CASH NEXT
%token <str> IDENTIFIER PRICESTRING
%token <int_val> USD EUR JPY
%token <int_val> INT LONG DOUBLE BOOLEAN SECURITY VOID CURRENCY ACCOUNT DATAFEED DATABASE EXCHANGE 
/* Arithmetic objec tokens */
%token <int_val> INTEGER
%token <fp_val> FLOATPT


%type <int_val> order_type;
%type <algorithm_list> algorithm_list;
%type <algorithm_header> algorithm_header;
%type <algorithm_function> algorithm_definition;
%type <int_val> type_specifier;

%type <statement_list> compound_statement;
%type <statement_list> statement_list;
%type <statement> statement;
%type <statement> expression_statement;
%type <statement> selection_statement;
%type <statement> set_statement;

%type <exp> argument_expression_list;
%type <exp> unary_expression;
%type <exp> postfix_expression;
%type <exp> primary_expression;
%type <exp> logical_OR_expression;
%type <exp> logical_AND_expression;
%type <exp> equality_expression;
%type <exp> relation_expression;
%type <exp> additive_expression;
%type <exp> multiplicative_expression;
%type <exp> expression;
%type <exp> assignment_expression;
%type <exp> type_name;
%type <exp> attribute;
%type <exp> int_expr;
%type <currency> currency;
%type <currency> curr_expr;
%type <security> security;
%type <security> sec_expr;
%type <position> position
%type <int_val> security_type;
%type <int_val> currency_type;
%type <strategy> strategy_definition
%type <strategy_list> strategy_list;
%type <strategy_block> strategy_body;
%type <strategy_block> strategy_block;
%type <action_list> process_body
%type <action_list> action_list;
%type <order_item> constraint_list;
%type <order_item> order_item;
%type <order_item> constraint;
%type <process_statement_list> process_statement_list; 
%type <process_statement> process_statement;
%type <program> process_list
%type <program> program
%type <int_val> unary_operator
%type <parameter_list> parameter_list


%%
program		: { PRINTI(("STARTING PARSE\n")); parent = NULL; top = symbol_table_create(parent); stack = malloc(sizeof(Stack));Stack_Init(stack); } 
		use_list process_list				{ $$=$3; PRINTI(("ENDING PARSE\n"));  free(stack);print_ast($$);print_symtab(top);root = $$;}
		;
		
use_list	: USE  variable_declaration		{ }	
		| use_list USE variable_declaration 
		;

process_list	: strategy_list 			{$$ = create_program(NULL, $1, top);}
	     	| function_list strategy_list		{ }
		| algorithm_list strategy_list 		{$$ = create_program($1, $2, top);}
		| function_list algorithm_list strategy_list	{ }
		;	     	

function_list	: function_definition			{ }
		| function_list function_definition	{ }
		;

algorithm_list 	: algorithm_definition			{ $$ = create_algorithm_list($1);}
		| algorithm_list algorithm_definition	{ $$ = add_algorithm_list($1, $2); }
		;

strategy_list 	: strategy_definition				{ $$ = create_strategy_list($1); }
		| strategy_list strategy_definition		{ $$ = add_strategy_list($1, $2); }
		;

function_definition : function_header compound_statement
		;

function_header : FUNCTION IDENTIFIER '(' parameter_list ')' func_return	{ }
		;

func_return	: RETURNS type_specifier
		;

algorithm_definition : algorithm_header compound_statement     { $$ = create_algorithm_ast($1, $2, top);
							            top = parent;
								   symbol_table_put_value(top, ALGORITHM_T, $$->name, $$); }
		;		

algorithm_header: ALGORITHM IDENTIFIER          { if (symbol_table_get_value(top, ALGORITHM_T, $2)) 
						strat_error_msg("variable name already exist", 1, yylineno); 
						parent = top; top = symbol_table_create(parent); }
	     	'(' parameter_list ')'		{ $$ = create_algorithm_header($<str>2, $5); }
		;

parameter_list	: type_specifier IDENTIFIER				{ $$ = create_parameter_list($1,0,$<str>2);
									if (symbol_table_put_value(top, $1, $<str>2, (void*)0) != 0)
									strat_error_msg("Symbol Already exist", 1, yylineno);}
	       	| type_specifier '#' IDENTIFIER				{ $$ = create_parameter_list($1,1, $<str>3);
									if (symbol_table_put_value(top, $1, $<str>3, (void*)0) != 0)
									strat_error_msg("Symbol Already exist", 1, yylineno);} 
		| parameter_list ',' type_specifier '#' IDENTIFIER	{ $$ = add_parameter_list($1,$3,1, $<str>5);
									if (symbol_table_put_value(top, $3, $<str>5, (void*)0) != 0)
									strat_error_msg("Symbol Already exist", 1, yylineno); }

		| /* empty */						{ }
		;

strategy_definition : STRATEGY IDENTIFIER '{' { if (symbol_table_get_value(top, STRATEGY_T, $2)) 
						strat_error_msg("variable name already exist", 1, yylineno);
						parent = top; 
		    				top = symbol_table_create(parent);  }    
		      strategy_body '}'	      {$$ = create_strategy($<str>2, $5, top);
						top = parent;
						symbol_table_put_value(top, STRATEGY_T, $<str>2, $$); }
	  	;

strategy_body	: variable_declaration_list statement_list strategy_block	{ $$ = $3; }
	      	| variable_declaration_list strategy_block 	{ $$ = $2; }
		| strategy_block		{ $$ = $1; }
		| /* empty */			{ }
		;

strategy_block	:  action_list			{ $$ = create_strategy_block(0, $1, NULL); }
		|  process_statement_list	{ $$ = create_strategy_block(1, NULL, $1); }
		;

process_statement_list : process_statement   { $$ = create_process_statement_list($1);}
		| process_statement_list process_statement { $$ = add_process_statement_list($1, $2); }
		;

process_statement : WHEN '(' expression ')' '{' process_body '}' UNTIL '(' expression ')' { $$ = create_process_statement($3, $6, $10); } 
		| WHEN '(' expression ')' '{' process_body '}'				{  if  (Stack_Top(stack) != BOOLEAN_T) 						strat_error_msg("invalid operation WHEN, the type in WHEN is not boolean", 1,yylineno);
											$$ = create_process_statement($3,$6,NULL);}
		;

process_body	: action_list { $$ = $1; }
	     	| action_list statement_list   // delete this plz
		;

action_list	: order_type '{' constraint_list '}'				{ $$ = create_action_list($1,$3);}
		| action_list order_type '{' constraint_list '}'		{ $$ = add_action_list($1,$2,$4);}
		;

constraint_list	: constraint							{ $$ = $1;}
		| constraint_list constraint
		;

constraint	: WHAT ':' order_item ';'					{ $$ = $3;}
	   	| WHERE ':' IDENTIFIER ';'		{ }
		;

order_item	: SECURITY '(' sec_expr ')' '.' AMOUNT '(' int_expr ')''.' PRICE '(' curr_expr ')' 	{ $$ = create_order_item($<security>3, $<exp>8, $<currency>13, 0); }
		;

sec_expr	: security				{ $$ = $1; }
	 	| IDENTIFIER				{ struct symbol_value *val = symbol_table_get_value(top, SECURITY_T, $1);	
							if (!val)strat_error_msg("security not found", 1, yylineno);		
							if (val -> type_specifier != SECURITY_T)strat_error_msg("Type access error", 1, yylineno);
$$ = (ast_security *)val->nodePtr; } 
		;

int_expr	: INTEGER				{ $$ = create_integer_const($1); }
	 	| IDENTIFIER				{ struct symbol_value *val = symbol_table_get_value(top, INT_T, $1);
							if (!val)strat_error_msg("int not found", 1, yylineno);		
							if (val -> type_specifier != INT_T)strat_error_msg("Type access error", 1, yylineno);
$$ = (ast_exp *)val->nodePtr; } 
		;

curr_expr	: PRICESTRING				{ $$ = create_ast_currency(USD_T, $<str>1); }
		| currency				{ $$ = $1; }
		| IDENTIFIER				{ struct symbol_value *val = symbol_table_get_value(top, CURRENCY_T, $1);
							if (!val)strat_error_msg("price  not found", 1, yylineno);		
							if (val -> type_specifier != CURRENCY_T)strat_error_msg("Type access error", 1, yylineno);
$$ = (ast_currency *)val->nodePtr; }
		;

variable_declaration_list : variable_declaration			{ }
		| variable_declaration_list variable_declaration	
		;

variable_declaration : type_specifier IDENTIFIER ';'	{ if( install_symbol($<int_val>1, $<str>2, top) != 0 )
		     					 	strat_error_msg("Symbol Name already in use", 1, yylineno); }
		;

type_specifier	: INT					{ $$ = INT_T; }
		| DOUBLE				{ $$ = DOUBLE_T; }
		| BOOLEAN				{ $$ = BOOLEAN_T; }
		| SECURITY				{ $$ = SECURITY_T; }
		| CURRENCY				{ $$ = CURRENCY_T; }
		| VOID					{ $$ = VOID_T; }
		| ACCOUNT				{ $$ = ACCOUNT_T; }
		| DATAFEED				{ $$ = DATAFEED_T; }
		| DATABASE				{ $$ = DATABASE_T; }
		| EXCHANGE				{ $$ = EXCHANGE_T; }
		;

security_type 	: EQTY					{$$ = EQTY_T;}
		;

currency_type	: USD					{$$ = USD_T;}
		| EUR					{$$ = EUR_T;}
		| JPY					{$$ = JPY_T;}
		;

order_type	: BUY					{ $$ = BUY_ORDER;}
		| SELL					{ $$ = SELL_ORDER;}
		;

statement	: expression_statement		{ $$ = $1;}
	  	| compound_statement		{ $$ = create_compound_statement($1); }
		| selection_statement		{ $$ = $1; }
		| iteration_statement		{ }
		| set_statement			{ $$ = $1; }
		;

expression_statement : expression ';'			{ $$ = create_expression_statement($1); }
		| ';'					{ }
		;

compound_statement : '{' variable_declaration_list statement_list '}'	{ $$ = $3; /*TODO: Add symbol table!*/}
		| '{' statement_list '}'	{ $$ = $2; /*TODO: Add symbol table */ }
		| '{' '}'			{ $$ = NULL; }
		;

statement_list	: statement			{ $$ = create_statement_list($1); }
		| statement_list statement	{ $$ = add_statement_list($1, $2);}
		;

selection_statement : IF '(' expression ')' statement   { $$ = create_selection_statement($3, $5); 
							if  (Stack_Top(stack) != BOOLEAN_T)
							strat_error_msg("invalid operation IF, the type in if is not boolean", 1, yylineno);}
		;

iteration_statement : WHILE '(' expression ')' statement
		;

set_statement 	: SET '{' argument_expression_list '}' IF ':' '{' expression '}'	{ /* $$ = create_set_statement(0,$3,$8); */
											if  (Stack_Top(stack) != BOOLEAN_T) 												strat_error_msg("invalid operation IF, the type in if is not boolean", 1, yylineno);
											$$ = create_set_statement($3,$8);  }
		;

expression	: assignment_expression				{ $$ = $1; }
            	| expression ',' assignment_expression
		;

assignment_expression : logical_OR_expression 			{ $$ = $1; }
            	| unary_expression '=' logical_OR_expression	{ semantic_check(OP_ASSIGN,2);$$ = create_opr(OP_ASSIGN, 2, $1, $3); }
		;

logical_OR_expression :  logical_AND_expression 		{ $$ = $1; }
            	| logical_OR_expression OR logical_AND_expression { semantic_check(OP_OR,2);$$ = create_opr(OP_OR, 2, $1, $3); }
		;

logical_AND_expression : equality_expression 				{ $$ = $1;}
           	| logical_AND_expression AND equality_expression { semantic_check(OP_AND,2);$$ = create_opr(OP_AND, 2, $1, $3); }
		;

equality_expression : relation_expression 				{ $$ = $1;}
		| equality_expression IS relation_expression		{ semantic_check(OP_IS,2);$$ = create_opr(OP_IS, 2, $1, $3);}
		| equality_expression ISNOT relation_expression		{ semantic_check(OP_ISNOT,2);$$ = create_opr(OP_ISNOT, 2, $1, $3); }
		;

relation_expression :  additive_expression 				{ $$ = $1;}
	        | relation_expression '<' additive_expression		{ semantic_check(OP_LT,2);$$ = create_opr(OP_LT, 2, $1, $3); }
       		| relation_expression '>' additive_expression		{ semantic_check(OP_GT,2);$$ = create_opr(OP_GT, 2, $1, $3); }
       		| relation_expression '<''=' additive_expression	{ semantic_check(OP_LTEQ,2);$$ = create_opr(OP_LTEQ, 2, $1, $4); }
        	| relation_expression '>''=' additive_expression	{ semantic_check(OP_LTEQ,2);$$ = create_opr(OP_GTEQ, 2, $1, $4); }
		;

additive_expression : multiplicative_expression				{ $$ = $1; }
        	| additive_expression '+' multiplicative_expression     { semantic_check(OP_ADD,2);$$= create_opr(OP_ADD, 2, $1, $3);}
	        | additive_expression '-' multiplicative_expression	{ semantic_check(OP_SUB,2);$$ = create_opr(OP_SUB, 2, $1, $3); }
		;

multiplicative_expression : unary_expression { $$ = $1; }
	        | multiplicative_expression '*' unary_expression	{ semantic_check(OP_MULT,2);$$ = create_opr(OP_MULT, 2, $1, $3); }
        	| multiplicative_expression '/' unary_expression	{ semantic_check(OP_DIV,2);$$ = create_opr(OP_DIV, 2, $1, $3); }
		;
	
unary_expression : postfix_expression { $$ = $1;}
		| unary_operator unary_expression { semantic_check($1,1);$$ = create_opr($1,1,$2,NULL);}
		;

unary_operator 	: '-'  { $$ = OP_UNARY_MINUS; }
		| '#'  { $$ = OP_UNARY_SHARP; }
		| NOT  { $$ = OP_UNARY_NOT; }
		;

postfix_expression : primary_expression 			{ $$ = $1; }
	        | postfix_expression '(' ')'			{ $$ = create_opr(OP_FUNC, 1, $1, NULL);}
       		| postfix_expression '(' argument_expression_list ')'  { $$ = create_opr(OP_FUNC, 2, $1, $3);}
		;

argument_expression_list : assignment_expression   { $$ = create_argument_expression_list($1);}
		| argument_expression_list ',' assignment_expression  { $$ = add_argument_expression_list($1, $3);}
		;

primary_expression : type_name		{ $$ = $1;}
		| INTEGER		{ Stack_Push(stack,INT_T);$$ = create_integer_const($1);}
		| FLOATPT		{ Stack_Push(stack,DOUBLE_T);$$ = create_double_const($1); }
		| PRICESTRING		{ Stack_Push(stack,DOUBLE_T);$$ = create_price_const($1); }
		| security		{ Stack_Push(stack,SECURITY_T);$$ = create_security_const($1); }
		| currency		{ }
		| position		{ }
		| TRUE_S		{ Stack_Push(stack,BOOLEAN_T);$$ = create_boolean_const(TRUE_T);  }
		| FALSE_S		{ Stack_Push(stack,BOOLEAN_T);$$ = create_boolean_const(FALSE_T); }
		| '(' expression ')'	{ }
		;

type_name	: IDENTIFIER		        { struct symbol_value *val = symbol_table_get_value(top, typeID, $1);
						if (!val)strat_error_msg("Identifier not found", 1, yylineno);
						Stack_Push(stack, val -> type_specifier); //semantic check
						$$ = create_id($1, top);}
	  	| type_name '.' IDENTIFIER	{ }
		| type_name '.' attribute	{ $$ = create_opr(OP_ATTR, 2, $1, $3);}
		;

position 	: POS '(' IDENTIFIER ')'	{  }
	  	;

security	: security_type '(' IDENTIFIER ')' { $$ = create_ast_security($<int_val>1, $<str>3); }
		;

currency	: currency_type '(' PRICESTRING ')'    { $$ = create_ast_currency($<int_val>1, $<str>3); }
	 	;

attribute	: SEC			{ semantic_check(SEC_T,1);$$ = create_attr(SEC_T);}
	  	| AMT			{ semantic_check(AMT_T,1);$$ = create_attr(AMT_T);}
		| PRC			{ semantic_check(PRC_T,1);$$ = create_attr(PRC_T);}
		| POS			{ semantic_check(POS_T,1);$$ = create_attr(POS_T);}
		| AVAIL_CASH		{ semantic_check(AVAIL_CASH_T,1);$$ = create_attr(AVAIL_CASH_T);}
		| NEXT			{ semantic_check(NEXT_T,1);$$ = create_attr(NEXT_T);}
		;

%%

void yyerror(char * s)
{
   fprintf(stderr, "line %d: %s\n", yylineno, s);
}

void strat_error_msg(char *msg, int error_t, int lineno)
{
    if( error_t == 1 )
    {	
        fprintf(stderr, "ERROR: %s line %d\n",msg, lineno);
        exit(EXIT_FAILURE);
    }
}

void semantic_check(int operator, int nopr)// todo change the para to num of operants  all use type in symbol table		
{		
	int op1_type,op2_type;		
	if (stack -> size == 0)strat_error_msg("empty stack at ", 1, yylineno);		
	op1_type = Stack_Top(stack);Stack_Pop(stack);		
	if (nopr >= 2){		
		if (stack -> size == 0)strat_error_msg("empty stack at ", 1, yylineno);		
		op2_type = Stack_Top(stack);Stack_Pop(stack);		
		}		
	switch(operator)		
	{		
		case OP_IS: if (op1_type != op1_type) 		
				strat_error_msg("invalid operation IS", 1, yylineno);		
				Stack_Push(stack, BOOLEAN_T);break;		
		case OP_ISNOT: if (op2_type != BOOLEAN_T) 		
				strat_error_msg("invalid operation IS_NOT", 1, yylineno);		
				Stack_Push(stack, BOOLEAN_T);break;		
		case OP_FUNC: break;		
		case OP_ASSIGN:  //strat_error_msg("invalid operation Assign", 1, yylineno); 		
				break;		
		case OP_OR: 		
		case OP_AND: if (op1_type != BOOLEAN_T || op2_type != BOOLEAN_T) 		
				strat_error_msg("invalid cmp operation ", 1, yylineno);		
				Stack_Push(stack, BOOLEAN_T); break;		
		case OP_ADD:		
		case OP_SUB:		
		case OP_MULT:		
		case OP_DIV:		
			if (op1_type != op2_type || (op1_type != INT_T && op1_type != DOUBLE_T)) 		
				strat_error_msg("invalid mathmatical operation  ", 1, yylineno);		
			if (op1_type > op2_type)		
				Stack_Push(stack, op1_type);		
			else Stack_Push(stack, op2_type);		
				break;		
		case OP_LT:		
		case OP_GT: 		
		case OP_LTEQ: 		
		case OP_GTEQ:		
			if ((op1_type != INT_T && op1_type != DOUBLE_T && op1_type != CURRENCY_T) || (op2_type != INT_T && op2_type != DOUBLE_T && op2_type != CURRENCY_T)) 		
				strat_error_msg("invalid relational operation ", 1, yylineno);		
				Stack_Push(stack, BOOLEAN_T); break;		
		case OP_UNARY_SHARP: if (op1_type != CURRENCY_T) 		
					strat_error_msg("invalid unary operation", 1, yylineno);		
					Stack_Push(stack, op1_type);break;		
		case OP_UNARY_MINUS: if (op1_type != INT_T && op1_type != DOUBLE_T) 		
					strat_error_msg("invalid unary operation", 1, yylineno);		
					Stack_Push(stack, op1_type);break;		
		case OP_UNARY_NOT: if (op1_type !=  BOOLEAN_T) 		
					strat_error_msg("invalid unary operation", 1, yylineno);		
					Stack_Push(stack, BOOLEAN_T);break;		
		case SEC_T:	if (op1_type !=  SECURITY_T) 		
				strat_error_msg("invalid postfix attribute", 1, yylineno);		
				Stack_Push(stack, SECURITY_T);break;				
		case AMT_T:		
		case PRC_T:	if (op1_type !=  SECURITY_T) 		
				strat_error_msg("invalid postfix attribute", 1, yylineno);		
				Stack_Push(stack, CURRENCY_T);break;		
		case POS_T:			
		case AVAIL_CASH_T:		
		case NEXT_T:	if (op1_type !=  DATAFEED_T) 		
				strat_error_msg("invalid postfix attribute", 1, yylineno);		
				Stack_Push(stack, SECURITY_T);break;		
	}		
	return;		
}		



int main(void)
{
   yyparse();
   run_interp(root);
   return 0;
}
