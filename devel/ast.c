#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ast.h"
#include "runtime.h"

void dbg_printf(const char *fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	vfprintf(stderr, fmt, args);
	va_end(args);
}

ast_program *
create_program(ast_algorithm_list * algorithm_list, ast_strategy_list* strategy_list, struct symbol_table* sym)
{
	ast_program * program;

	program = malloc(sizeof(ast_program));
	if (!program) {
		printf("out of memory in %s\n", __func__);
		return NULL;
	}

	if (algorithm_list) {
		program->algo_list = algorithm_list->algo_list;
		program->num_of_algos= algorithm_list->num_of_algos;
		free(algorithm_list);
	}
	if (strategy_list) {
		program->strategy_list = strategy_list->strategy;
		program->num_of_strategies = strategy_list->num_of_strategies;
		free(strategy_list);
	}
	program->sym = sym;

	PRINT(("%s\n", __func__));

	return program;

}

ast_algorithm_list *
create_algorithm_list(ast_algorithm* algo)
{
    ast_algorithm_list * algorithm_list;
    
    algorithm_list = malloc(sizeof(ast_algorithm_list));
    if (!algorithm_list) {
        printf("out of memory in %s\n", __func__);
        return NULL;
    }
    
    algorithm_list->algo_list= malloc(sizeof(ast_algorithm*));
    if (!algorithm_list->algo_list) {
        printf("out of memory in %s\n", __func__);
        return NULL;
    }
    
    algorithm_list->algo_list[0] = algo;
    algorithm_list->num_of_algos = 1;
    
    PRINT(("%s\n", __func__));
    
    return algorithm_list;
    
}

ast_algorithm_list *
add_algorithm_list(ast_algorithm_list *list, ast_algorithm * algo)
{
    int num_of_algos;
    num_of_algos = list->num_of_algos + 1;
    
    list->algo_list = realloc(list->algo_list, num_of_algos*sizeof(ast_algorithm*));
    if (!list->algo_list) {
        printf("out of memory in %s\n", __func__);
        return NULL;
    }
    
    list->algo_list[num_of_algos-1] = algo;
    list->num_of_algos += 1;
    
    PRINT(("%s\n", __func__));
    return list;
}


ast_algorithm *
create_algorithm_ast(ast_algorithm_header * algorithm_header,ast_statement_list * statement_list, struct symbol_table* sym)
{
    ast_algorithm * algo;
    
    algo = malloc(sizeof(ast_algorithm));
    if (!algo) {
        printf("out of memory in %s\n", __func__);
        return NULL;
    }
    
    strcpy (algo->name, algorithm_header->name);
    algo->num_of_para = algorithm_header->num_of_para;
    algo->para_list = algorithm_header->para_list;
    if( statement_list == NULL )
    {
	    algo->num_of_statement = 0;
	    algo->statement = NULL;
    }
    else
    {
	    algo->num_of_statement = statement_list->num_of_statement;
	    algo->statement = statement_list->statement;
    }
    algo->sym = sym;
    
    PRINT(("%s\n", __func__));
    
    return algo;
    
}

ast_algorithm_header *
create_algorithm_header(char * name, ast_parameter_list * parameter_list)
{
    ast_algorithm_header * algorithm_header;
    
    algorithm_header = malloc(sizeof(ast_algorithm_header));
    if (!algorithm_header) {
        printf("out of memory in %s\n", __func__);
        return NULL;
    }
    
    strcpy (algorithm_header->name, name);
    algorithm_header->para_list = parameter_list->para_list;
    algorithm_header->num_of_para = parameter_list->num_of_para;
    
    PRINT(("%s\n", __func__));
    free(parameter_list);
    
    return algorithm_header;
    
}

ast_parameter_list *
create_parameter_list(int type_specifier, int has_sharp, char * name)
{
    ast_parameter_list * list;
    
    list = malloc(sizeof(ast_parameter_list));
    if (!list) {
        printf("out of memory in %s\n", __func__);
        return NULL;
    }
    
    list->para_list = malloc(sizeof(parameter*));
    if (!list->para_list) {
        printf("out of memory in %s\n", __func__);
        return NULL;
    }
    
    parameter * para;
    para = malloc(sizeof(parameter));
    if (!para) {
    	printf("out of memory in %s\n", __func__);
        return NULL;
    }
    para = malloc(sizeof(parameter));
    para->type_specifier = type_specifier;
    para->has_sharp = has_sharp;
    strcpy(para->name, name);

    list->para_list[0] = para; 
    list->num_of_para = 1;
    
    PRINT(("%s\n", __func__));
    
    return list;
}

ast_parameter_list *
add_parameter_list(ast_parameter_list * list, int type_specifier, int has_sharp, char * name)
{
	int num_of_para = list->num_of_para+1;
	list->para_list = realloc(list->para_list, num_of_para*sizeof(parameter*));
      	if (!list->para_list) {
        	printf("out of memory in %s\n", __func__);
        	return NULL;
    	}
    
    	parameter * para;
    	para = malloc(sizeof(parameter));
	if (!para) {
	    	printf("out of memory in %s\n", __func__);
	        return NULL;
    	}
    	para->type_specifier = type_specifier;
    	para->has_sharp = has_sharp;
    	strcpy(para->name, name);

	list->para_list[num_of_para-1] = para;
    	list->num_of_para += 1;
    
    	PRINT(("%s\n", __func__));
    	return list;
}

ast_statement*
create_expression_statement(ast_exp *exp)
{
	ast_statement *stmt;
	stmt = malloc(sizeof(ast_statement));
	if(!stmt)
	{
	       printf("out of memory in %s\n", __func__);
        	return NULL;
	}
	stmt->type = expression_ST;
	stmt->expression_statement = exp;
 	
	PRINT(("%s\n", __func__));
	return stmt;
}

ast_statement*
create_compound_statement(ast_statement_list * statement_list)
{
	ast_statement *stmt;
	stmt = malloc(sizeof(ast_statement));
	if(!stmt)
	{
	       printf("out of memory in %s\n", __func__);
        	return NULL;
	}
	stmt->type = compound_ST;
	if( statement_list == NULL )
	{
		stmt->statement_list.num_of_statement = 0;
		stmt->statement_list.statement = NULL;
	}
	else
	{
		stmt->statement_list.num_of_statement = statement_list->num_of_statement;
		stmt->statement_list.statement = statement_list->statement;
	}
	PRINT(("%s\n", __func__));
	return stmt;
}

ast_statement *create_selection_statement(ast_exp *exp, ast_statement *statement)
{
	ast_statement *stmt;
	stmt = malloc(sizeof(ast_statement));
	if(!stmt)
	{
	       printf("out of memory in %s\n", __func__);
        	return NULL;
	}
	stmt->type = selection_ST;
	stmt->selection_statement.exp = exp;
	stmt->selection_statement.statement = statement;
 	
	PRINT(("%s\n", __func__));
	return stmt;
}

ast_statement *
//create_set_statement(int this_type, ast_exp *argu_list, ast_exp *exp)
create_set_statement(ast_exp *argu_list, ast_exp *exp)
{
    ast_statement * stmt;
    
    stmt = malloc(sizeof(ast_statement));
    if (!stmt) {
        printf("out of memory in %s\n", __func__);
        return NULL;
    }
    
    stmt->type = set_ST;
    //stmt->set_statement.this_type = this_type;
    stmt->set_statement.argu_list = argu_list;
    stmt->set_statement.exp = exp;
    
    PRINT(("%s\n", __func__));
    
    return stmt;
}

ast_statement_list *
create_statement_list(ast_statement *statement)
{	
	ast_statement_list * list;
	list = malloc(sizeof(ast_statement_list));
	if(!list)
	{
	       printf("out of memory in %s\n", __func__);
        	return NULL;
	}
	
	list->statement = malloc(sizeof(ast_statement*));
	if(!list->statement)
	{
	       printf("out of memory in %s\n", __func__);
        	return NULL;
	}
	
	list->statement[0] = statement;
	list->num_of_statement = 1;
	
	PRINT(("%s\n", __func__));
	return list;
}

ast_statement_list *
add_statement_list(ast_statement_list *list, ast_statement *statement)
{
	int num = list->num_of_statement + 1;
	list->statement = realloc(list->statement, num*sizeof(ast_statement*));
	if(!list->statement)
	{
	       printf("out of memory in %s\n", __func__);
        	return NULL;
	}
	
	list->statement[num-1] = statement;
	list->num_of_statement = num;

	PRINT(("%s\n", __func__));
	return list;

}

/*
ast_exp *
create_const(int value)
{
	ast_exp *p;
	p = malloc(sizeof(ast_exp));
	if (!p) {
		printf("out of memory in %s\n", __func__);
		return NULL;
	}

	p->type = typeConst;
	p->con.value = value;

    	PRINT(("%s\n", __func__));
	return p;
}
*/
ast_exp *
create_argument_expression_list(ast_exp * exp)
{
	ast_exp * p;
	p = malloc(sizeof(ast_exp));
	if(!p)
	{
		printf("out of memory in %s\n", __func__);
       		return NULL;
    	}
	
	p->type = typeArgulist;
	p->argu_list.exp = malloc(sizeof(ast_exp*));
	if(!p->argu_list.exp)
	{
		printf("out of memory in %s\n", __func__);
       		return NULL;
    	}
	p->argu_list.exp[0] = exp;
	p->argu_list.num_of_argument_expression_list = 1;
	return p;
}

ast_exp *
add_argument_expression_list(ast_exp * p, ast_exp * exp)
{
    int num_of_argument_expression_list;
    
    num_of_argument_expression_list = p->argu_list.num_of_argument_expression_list + 1;
    p->argu_list.exp = realloc(p->argu_list.exp, num_of_argument_expression_list*sizeof(ast_exp*));
    if (!p->argu_list.exp) {
        printf("out of memory in %s\n", __func__);
        return NULL;
    }

    p->argu_list.exp[num_of_argument_expression_list-1] = exp;
    p->argu_list.num_of_argument_expression_list += 1;
    
    PRINT(("%s\n", __func__));
    return p;
}

ast_currency *
create_ast_currency(int type, char *price)
{
	ast_currency *curr_node = malloc( sizeof(ast_currency) );
	if ( curr_node == NULL )
	{
		die("malloc failed in create_ast_currency()");
	}

	curr_node->curr = create_currency(type, price);

	PRINT(("%s\n", __func__));
	return curr_node;
}


ast_security *
create_ast_security(int sec_type, char * name)
{
	ast_security *security_node = malloc( sizeof(ast_security) );
	if ( security_node == NULL )
	{
		die("malloc failed in create_ast_security()");
	}

	security_node->sec = create_security(sec_type, name);

	PRINT(("%s\n", __func__));
	return security_node;
}

ast_position *
create_ast_position(ast_security *astsec, int amt, ast_currency *astpr)
{
	ast_position *position_node = malloc( sizeof(ast_position) );
	if ( position_node == NULL )
	{
		die("malloc failed in create_ast_position()");
	}

	position_node->pos = create_position(astsec->sec, amt, astpr->curr);

	PRINT(("%s\n", __func__));
	return position_node;
}


/*************strategy part********************/
ast_strategy_list *
create_strategy_list(ast_strategy * strategy)
{
    ast_strategy_list * strategy_list;
    
    strategy_list = malloc(sizeof(ast_strategy_list));
    if (!strategy_list) {
        printf("out of memory in %s\n", __func__);
        return NULL;
    }
    
    strategy_list->strategy = malloc(sizeof(ast_strategy*));
    if(!strategy_list->strategy) {
        printf("out of memory in %s\n", __func__);
        return NULL;
    }
    strategy_list->strategy[0] = strategy;
    strategy_list->num_of_strategies = 1;
    
    PRINT(("%s\n", __func__));
    
    return strategy_list;
    
}

ast_strategy_list *
add_strategy_list(ast_strategy_list * list, ast_strategy * strategy)
{
    int num_of_strategies;
    num_of_strategies = list->num_of_strategies +1;
    list->strategy = realloc(list->strategy, num_of_strategies*sizeof(ast_strategy*));
    if(!list->strategy) {
        printf("out of memory in %s\n", __func__);
        return NULL;
    }
    
    list->strategy[num_of_strategies-1] = strategy;
    list->num_of_strategies += 1;
    
    PRINT(("%s\n", __func__));
    return list;
}

ast_strategy*
create_strategy( char * name, ast_strategy_block *strategy_body, struct symbol_table * sym)
{
	ast_strategy * strategy;

	strategy = malloc(sizeof(ast_strategy));
	if (!strategy) {
		printf("out of memory in %s\n", __func__);
		return NULL;
	}

	strcpy(strategy->name, name);
	if (strategy_body->order_list) {
		strategy->num_of_orders = strategy_body->num_of_orders;
		strategy->order_list = strategy_body->order_list;
	}
	if (strategy_body->process_statement) {
	    strategy->num_of_process_statement = strategy_body->num_of_process_statement;
	    strategy->process_statement = strategy_body->process_statement;
    	}
	free(strategy_body);
	strategy->sym = sym;

	PRINT(("%s\n", __func__));

	return strategy;

}

ast_strategy_block*
create_strategy_block( int type, ast_action_list * action_list, ast_process_statement_list * process_statement_list)
{
    ast_strategy_block * strategy_block;
    
    strategy_block = malloc(sizeof(ast_strategy_block));
    if (!strategy_block) {
        printf("out of memory in %s\n", __func__);
        return NULL;
    }
    strategy_block->type = type;

    if (action_list) {
	    strategy_block->num_of_orders = action_list->num_of_orders;
	    strategy_block->order_list = action_list->order;
	    free(action_list);
    }
    if (process_statement_list) {
	    strategy_block->num_of_process_statement = process_statement_list->num_of_process_statement;
	    strategy_block->process_statement = process_statement_list->process_statement;
	    free(process_statement_list);
    }

    
    PRINT(("%s\n", __func__));
    
    return strategy_block;
    
}


ast_action_list *
create_action_list(int type, ast_order_item * order)
{
    ast_action_list * action_list;
    
    action_list = malloc(sizeof(ast_action_list));
    if (!action_list) {
        printf("out of memory in %s\n", __func__);
        return NULL;
    }
    
    action_list->order = malloc(sizeof(ast_order_item*));
    if (!action_list->order) {
        printf("out of memory in %s\n", __func__);
        return NULL;
    }

    order->type = type;
    action_list->order[0] = order;
    action_list->num_of_orders = 1;
    
    PRINT(("%s\n", __func__));
    
    return action_list;
}

ast_action_list *
add_action_list(ast_action_list* list, int type, ast_order_item * order)
{
    PRINT(("add_actionlist************"));
    int num_of_orders;
    
    num_of_orders = list->num_of_orders + 1;
    list->order = realloc(list->order, num_of_orders*sizeof(ast_order_item*));
    if (!list->order) {
        printf("out of memory in %s\n", __func__);
        return NULL;
    }

    order->type = type;
    list->order[num_of_orders-1] = order;
    list->num_of_orders += 1;
    
    PRINT(("%s\n", __func__));
    return list;
}

ast_order_type*
create_order_type( int type)
{
    ast_order_type * order_type;
    
    order_type = malloc(sizeof(ast_order_type));
    if (!order_type) {
        printf("out of memory in %s\n", __func__);
        return NULL;
    }
    
    order_type->type = type;
    
    PRINT(("%s\n", __func__));
    
    return order_type;
    
}

ast_constraint_list *
create_constraint_list(ast_constraint * constraint)
{
    ast_constraint_list * constraint_list;
    
    constraint_list = malloc(sizeof(ast_constraint_list));
    if (!constraint_list) {
        printf("out of memory in %s\n", __func__);
        return NULL;
    }
    
    constraint_list->constraint = malloc(sizeof(ast_constraint*));
    if (!constraint_list->constraint) {
        printf("out of memory in %s\n", __func__);
        return NULL;
    }
    constraint_list->constraint[0] = constraint;
    constraint_list->num_of_constraints = 1;
    
    PRINT(("%s\n", __func__));
    
    return constraint_list;
}

ast_constraint_list *
add_constraint_list(ast_constraint_list * list, ast_constraint * constraint)
{
    int num_of_constraints;
    
    num_of_constraints = list->num_of_constraints + 1;
    list->constraint = realloc(list->constraint, num_of_constraints*sizeof(ast_constraint*));
    if (!list->constraint) {
        printf("out of memory in %s\n", __func__);
        return NULL;
    }
    
    list->constraint[num_of_constraints-1] = constraint;
    list->num_of_constraints += 1;
    
    PRINT(("%s\n", __func__));
    return list;
}

ast_constraint *
create_constraint(ast_order_item * order_item)
{
    ast_constraint * constraint;
    
    constraint = malloc(sizeof(ast_constraint));
    if (!constraint) {
        printf("out of memory in %s\n", __func__);
        return NULL;
    }
    constraint->order_item = order_item;
    
    PRINT(("%s\n", __func__));
    
    return constraint;
    
}

ast_order_item *
create_order_item(ast_security *sec, ast_exp *num, ast_currency *prc, int t)
{
    ast_order_item * new_order_item = malloc(sizeof(ast_order_item));
    if (!new_order_item) {
        die("malloc failed in create_order_item");
    }

    new_order_item->sec = sec;
    new_order_item->number = num;
    new_order_item->prc = prc;
    new_order_item->type = t;

    PRINT(("%s\n", __func__));

    return new_order_item;
}

/*
ast_security *
create_security(int security_type, char * name)
{
    ast_security * security;
    
    security = malloc(sizeof(ast_security));
    if (!security) {
        printf("out of memory in %s\n", __func__);
        return NULL;
    }
    security->type = security_type;
    strcpy(security->name,  name);

    PRINT(("%s\n", __func__));
    
    return security;
    
}

ast_security_type *
create_security_type(int type)
{
    ast_security_type * security_type;
    
    security_type = malloc(sizeof(ast_security_type));
    if (!security_type) {
        printf("out of memory in %s\n", __func__);
        return NULL;
    }
    security_type->type = type;
    
    PRINT(("%s\n", __func__));
    
    return security_type;
    
}
*/
ast_process_statement_list *
create_process_statement_list(ast_process_statement * process_statement)
{
    ast_process_statement_list * process_statement_list;
    
    process_statement_list = malloc(sizeof(ast_process_statement_list));
    if(!process_statement_list)
    {
        printf("out of memory in %s\n", __func__);
        return NULL;
    }
    
    process_statement_list->process_statement = malloc(sizeof(ast_process_statement*));
    if(!process_statement_list->process_statement)
    {
        printf("out of memory in %s\n", __func__);
        return NULL;
    }
    
    process_statement_list->process_statement[0] = process_statement;
    process_statement_list->num_of_process_statement = 1;
    
    PRINT(("%s\n", __func__));
    
    return process_statement_list;
}

ast_process_statement_list *
add_process_statement_list(ast_process_statement_list * list, ast_process_statement * process_statement)
{
    int num_of_process_statement;
    
    num_of_process_statement = list->num_of_process_statement + 1;
    list->process_statement = realloc(list->process_statement, num_of_process_statement*sizeof(ast_process_statement*));
    if(!list->process_statement)
    {
        printf("out of memory in %s\n", __func__);
        return NULL;
    }
    
    list->process_statement[num_of_process_statement-1] = process_statement;
    list->num_of_process_statement = num_of_process_statement;
    
    PRINT(("%s\n", __func__));
    
    return list;
    
}

ast_process_statement *
create_process_statement(ast_exp *expression, ast_action_list *action_list)
{
    ast_process_statement * process_statement;
    
    process_statement = malloc(sizeof(ast_process_statement));
    if (!process_statement) {
        printf("out of memory in %s\n", __func__);
        return NULL;
    }
   /* process_statement->type = type;*/
    process_statement->expression = expression;
    process_statement->action_list = action_list;
    process_statement->until_exp = NULL;
    
    PRINT(("%s\n", __func__));
    
    return process_statement;
    
}

void print_ast(ast_program *program)
{	
	int num_of_algos, num_of_strats,i;
	num_of_algos = program->num_of_algos;
	num_of_strats = program->num_of_strategies;

	printf("##############################\nSTART: print ast_tree.\n");
	printf("\n\nNum of algorithms: %d\n", num_of_algos);
	for(i = 0; i<num_of_algos; i++)
	{	
		printf(">>>Algorithm %d: \n", i);
		print_algorithm(program->algo_list[i]);
	}

	printf("\n\nNum of stategies: %d\n", num_of_strats);
	for(i = 0; i<num_of_strats; i++)
	{	
		printf(">>>STRATEGY %d\n", i);		
		print_strategy(program->strategy_list[i]);
	}
	printf("\n\nEND: print ast_tree\n##############################\n");
}

void print_algorithm(ast_algorithm * algorithm)
{
	int i = 0;
	printf("Algorithm name: %s\n", algorithm->name);
	for(i = 0; i<algorithm->num_of_para; i++)
	{
		printf("parameter %d, type: %s, has sharp: %d, identifier: %s\n", i, type_tostring(algorithm->para_list[i]->type_specifier), algorithm->para_list[i]->has_sharp, algorithm->para_list[i]->name);
	}
	for(i = 0; i<algorithm->num_of_statement; i++)
	{
		printf("Algorithm compound statement %d\n", i);
		print_statement(algorithm->statement[i]);
	}
}

void print_statement(ast_statement *statement)
{
	int i;
	switch(statement->type)
	{
		case(expression_ST):
			print_exp(statement->expression_statement);
		 	break;
		case(compound_ST):
			for(i = 0; i<statement->statement_list.num_of_statement; i++)
			{
				print_statement(statement->statement_list.statement[i]);
			}	
			break;
		case(selection_ST): /*if*/
			print_exp(statement->selection_statement.exp);
			print_statement(statement->selection_statement.statement);
			break;
		case(set_ST):
			printf("What to set: \n");
			for(i = 0; i<statement->set_statement.argu_list->argu_list.num_of_argument_expression_list; i++)
			{
				print_exp(statement->set_statement.argu_list->argu_list.exp[i]);
			}
			printf("Set condition: \n");
			print_exp(statement->set_statement.exp);
			break;
		default:	
			;
	}
}


void print_strategy(ast_strategy * strategy)
{	
	int i;
	printf("Strategy name: %s\n", strategy->name);
	printf("num of orders: %d\n", strategy->num_of_orders);
	for(i = 0; i<strategy->num_of_orders; i++)
	{
		printf("ORDER %d:\n", i );
		print_order(strategy->order_list[i]);
	}
	printf("num of process_statements: %d\n", strategy->num_of_process_statement);
	for(i = 0; i<strategy->num_of_process_statement; i++)
	{
		printf("PROCESS_STATEMENT %d:\n", i );
		print_process_statement(strategy->process_statement[i]);
	}
	
}

/*
void print_order(ast_order_item * order_item)
{
	printf("Order type: %d\n", order_item->type);	
	printf("Security_type: %d	Security_name: %s\n", order_item->security_type, order_item->security_name);
	//printf("Amount: %d	Price: %s\n", order_item->number , order_item->price);
}
*/

void print_order(ast_order_item * order_item)
{
	if( order_item->type == BUY_ORDER )
	{
		printf("Order type: BUY\n");
	}
	else if( order_item->type == SELL_ORDER )
	{
		printf("Order type: SELL\n");
	}

	char *sectype = NULL;
	if( order_item->sec->sec->sec_t == EQTY_T )
		sectype = "EQTY";

	printf("Security_type: %s	Security_name: %s\n", sectype, order_item->sec->sec->sym);
	//printf("Amount: %d	Price: %s\n", order_item->number->con.int_value->value, order_item->prc->curr->p);
}



void print_process_statement(ast_process_statement * process_statement)
{	
	int i;
	printf("================>Expression tree:\n");
	print_exp(process_statement->expression);
	printf("================>End of Expression tree:\n");

	printf("\nNum of orders: %d\n", process_statement->action_list->num_of_orders);
	for(i = 0; i<process_statement->action_list->num_of_orders; i++)
	{	
		printf("ORDER %d:\n", i );
		print_order(process_statement->action_list->order[i]);
	}
}

void print_exp(ast_exp *exp)
{
	int i = 0;
	switch(exp->type)
	{
		case(typeBooleanConst):
			printf("leaf boolean const: %d\n", exp->con.bool_value->value); break;
		case(typeIntegerConst):
			printf("leaf int const: %d\n", exp->con.int_value->value); break;
		case(typeDoubleConst):
			printf("leaf double const: %.1f\n", exp->con.double_value->value); break;
		case(typePriceConst):
			printf("leaf price const: %s\n", exp->con.price_value->price); break;
		case(typeID):
			printf("leaf ID: %s\n", exp->id.value); break;
		case(typeKeyword):
			printf("leaf Keyword: %s\n", exp->key.value); break;
		case(typeArgulist):
			printf("leaf Argulist: \n"); 
			for(i=0; i<exp->argu_list.num_of_argument_expression_list; i++)
				print_exp(exp->argu_list.exp[i]);
			break;
		case(typeOper):	
			printf("Operator: %s\n", oper_type_tostring(exp->oper.oper));
			print_exp(exp->oper.op1);
			if (exp->oper.nops == 2) print_exp(exp->oper.op2); 
			break;
		case(typeAttr):
			printf("ATTR: %s\n", attr_tostring(exp->attr.value));
			break;
		case(typeSec):
			printf("Security type: %d\nSecurity name: %s\n", exp->security.sec->sec_t,exp->security.sec->sym);
			break;

		default:
			printf("Wrong exp node type.This type is %d\n", exp->type);
	}
}

int install_symbol(int id_type, const char *id, struct symbol_table *symtab)
{
	int ret = 0;
	switch( id_type )
	{
		case ACCOUNT_T:
			return symbol_table_put_value(symtab, id_type, id, (void*)create_account());
			break;
		case DATAFEED_T:
			ret = symbol_table_put_value(symtab, id_type, id, (void*)create_data_source(id, id_type));
			break;
		case DATABASE_T:
	  		ret = symbol_table_put_value(symtab, id_type, id, (void*)create_data_source(id, id_type));
			break;
		case CURRENCY_T:
			ret = symbol_table_put_value(symtab, id_type, id, (void*)create_ast_currency(USD_T, NULL));
			break;
		case SECURITY_T:
			ret = symbol_table_put_value(symtab, id_type, id, (void*)create_security(id_type, NULL));
			break;
		case INT_T:
			ret = symbol_table_put_value(symtab, id_type, id, (void*)create_integer(0));
			break;
		case DOUBLE_T:
			ret = symbol_table_put_value(symtab, id_type, id, (void*)create_double(0.0));
			break;
		default: /*TODO*/;
	  		ret = symbol_table_put_value(symtab, id_type, id, 0);
	}
	return ret;
}

ast_exp *
create_opr(int oper, int nops, ast_exp* op1, ast_exp* op2) {
	ast_exp *p;
	p = malloc(sizeof(ast_exp));
	if (!p) {
		printf("out of memory in %s\n", __func__);
		return NULL;
	}

	p->type = typeOper;
	p->oper.nops = nops;
	p->oper.oper = oper;  /*operater*/
	p->oper.op1 = op1;
	p->oper.op2 = op2;

    	PRINT(("%s\n", __func__));
	return p;
}

ast_exp *
create_id(char* value, struct symbol_table *top)
{
	ast_exp *p;
	p = malloc(sizeof(ast_exp));
	if (!p) {
		printf("out of memory in %s\n", __func__);
		return NULL;
	}

	p->type = typeID;
	strcpy (p->id.value, value);
	p->id.sym = top;

    	PRINT(("%s\n", __func__));
	return p;
}


ast_exp *
create_keyword(char* value)
{
	ast_exp *p;
	p = malloc(sizeof(ast_exp));
	if (!p) {
		printf("out of memory in %s\n", __func__);
		return NULL;
	}

	p->type = typeKeyword;
	strcpy (p->key.value, value);

    	PRINT(("%s\n", __func__));
	return p;
}

ast_exp *
create_attr(int value)
{
	ast_exp *p;
	p = malloc(sizeof(ast_exp));
	if (!p) {
		printf("out of memory in %s\n", __func__);
		return NULL;
	}

	p->type = typeAttr;
	p->attr.value = value;

    	PRINT(("%s\n", __func__));
	return p;
}


ast_exp *
create_boolean_const(int value)
{
	ast_exp *p;
	p = malloc(sizeof(ast_exp));
	if (!p) {
		printf("out of memory in %s\n", __func__);
		return NULL;
	}

	p->type = typeBooleanConst;
	p->con.bool_value = create_boolean(value);

    	PRINT(("%s\n", __func__));
	return p;
}

ast_exp *
create_integer_const(int value)
{
	ast_exp *p;
	p = malloc(sizeof(ast_exp));
	if (!p) {
		printf("out of memory in %s\n", __func__);
		return NULL;
	}

	p->type = typeIntegerConst;
	p->con.int_value = create_integer(value);

    	PRINT(("%s\n", __func__));
	return p;
}

ast_exp *
create_double_const(double value)
{
	ast_exp *p;
	p = malloc(sizeof(ast_exp));
	if (!p) {
		printf("out of memory in %s\n", __func__);
		return NULL;
	}

	p->type = typeDoubleConst;
	p->con.double_value = create_double(value);

    	PRINT(("%s\n", __func__));
	return p;
}

ast_exp *
create_price_const(char *value)
{
	ast_exp *p;
	p = malloc(sizeof(ast_exp));
	if (!p) {
		printf("out of memory in %s\n", __func__);
		return NULL;
	}

	p->type = typePriceConst;
	p->con.price_value = create_price(value);

    	PRINT(("%s\n", __func__));
	return p;
}

ast_exp *
create_security_const(ast_security *security)
{
	ast_exp *p;
	p = malloc(sizeof(ast_exp));
	if (!p) {
		printf("out of memory in %s\n", __func__);
		return NULL;
	}

	p->type = typeSec;
	p->security.sec  = malloc(sizeof(struct security));
	p->security.sec->sec_t = 0;
	strcpy(p->security.sec->sym, security->sec->sym);

    	PRINT(("%s\n", __func__));
	return p;
}



