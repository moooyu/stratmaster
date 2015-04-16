#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ast.h"

void dbg_printf(const char *fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	vfprintf(stderr, fmt, args);
	va_end(args);
}

ast_program *
create_program(ast_use_list * use_list, ast_decision_list * decision_list)
{
	ast_program * program;

	program = malloc(sizeof(ast_program));
	if (!program) {
		yyerror("out of memory");
		return NULL;
	}

	program->use_list = use_list;
    program->decision_list = decision_list;

	PRINT(("%s\n", __func__));
	PRINT(("End of AST construction\n\n"));

	return program;

}


ast_use_list *
create_use_list(char* first_acc_name, ast_use_others * use_others)
{
    ast_use_list * use_list;
    
    use_list = malloc(sizeof(ast_use_list));
    if (!use_list) {
        yyerror("out of memory");
        return NULL;
    }
    
    use_list->first_acc_name = first_acc_name;
    use_list->use_others = use_others;
    
    PRINT(("%s\n", __func__));
    
    return use_list;
    
}


ast_use_others *
create_use_others()
{
    ast_use_others * use_others;
    
    use_others = malloc(sizeof(ast_use_others));
    if (!use_others) {
        yyerror("out of memory");
        return NULL;
    }
    
    use_others->usedefine = malloc(sizeof(use_define*));
    if (!use_others->usedefine) {
        yyerror("out of memory");
        return NULL;
    }
    
    use_define *usedefine = NULL;
   // usedefine->type = -1;
   // printf("hihi\n");
   // usedefine->name = "00";
    
    use_others->usedefine[0] = usedefine;
    use_others->num_of_use_defines = 1;
    
    PRINT(("%s\n", __func__));
   
    return use_others;
    
}

ast_use_others *
add_use_others(ast_use_others *list, int type, char * name)
{
    int num_of_use_defines;
    num_of_use_defines = list->num_of_use_defines + 1;
    
    list->usedefine = realloc(list->usedefine, num_of_use_defines*sizeof(use_define*));
    if (!list->usedefine) {
        yyerror("out of memory");
        return NULL;
    }
    
    use_define *usedefine;
    usedefine->type = type;
    usedefine->name = name;
    
    list->usedefine[num_of_use_defines-1] = usedefine;
    list->num_of_use_defines += 1;
    
    PRINT(("%s\n", __func__));
    return list;
}


ast_decision_list *
create_decision_list(ast_algorithm_list * algorithm_list)
{
    ast_decision_list * decision_list;
    
    decision_list = malloc(sizeof(ast_decision_list));
    if (!decision_list) {
        yyerror("out of memory");
        return NULL;
    }
    
    decision_list->algorithm_list = algorithm_list;
    
    PRINT(("%s\n", __func__));
    return decision_list;
    
}

ast_algorithm_list *
create_algorithm_list()
{
    ast_algorithm_list * algorithm_list;
    
    algorithm_list = malloc(sizeof(ast_algorithm_list));
    if (!algorithm_list) {
        yyerror("out of memory");
        return NULL;
    }
    
    algorithm_list->algorithm_function = malloc(sizeof(ast_algorithm_list*));
    if (!algorithm_list->algorithm_function) {
        yyerror("out of memory");
        return NULL;
    }
    
//    algorithm_list->algorithm_function[0] = NULL;
    algorithm_list->num_of_algos = 1;
    
    PRINT(("%s\n", __func__));
    
    return algorithm_list;
    
}

ast_algorithm_list *
add_algorithm_list(ast_algorithm_list *list, ast_algorithm_function * algorithm_function)
{
    int num_of_algos;
    num_of_algos = list->num_of_algos + 1;
    
    list->algorithm_function = realloc(list->algorithm_function, num_of_algos*sizeof(ast_algorithm_function*));
    if (!list->algorithm_function) {
        yyerror("out of memory");
        return NULL;
    }
    
    list->algorithm_function[num_of_algos-1] = algorithm_function;
    list->num_of_algos += 1;
    
    PRINT(("%s\n", __func__));
    return list;
}


ast_algorithm_function *
create_algorithm_function(ast_algorithm_header * algorithm_header,ast_compound_statement * compound_statement)
{
    ast_algorithm_function * algorithm_function;
    
    algorithm_function = malloc(sizeof(ast_algorithm_function));
    if (!algorithm_function) {
        yyerror("out of memory");
        return NULL;
    }
    
    algorithm_function->algorithm_header = algorithm_header;
    algorithm_function->compound_statement = compound_statement;
    
    PRINT(("%s\n", __func__));
    
    return algorithm_function;
    
}

ast_algorithm_header *
create_algorithm_header(char * name, ast_algorithm_parameter_list * algorithm_parameter_list)
{
    ast_algorithm_header * algorithm_header;
    
    algorithm_header = malloc(sizeof(ast_algorithm_header));
    if (!algorithm_header) {
        yyerror("out of memory");
        return NULL;
    }
    
    algorithm_header->name = name;
    algorithm_header->algorithm_parameter_list = algorithm_parameter_list;
    
    PRINT(("%s\n", __func__));
    
    return algorithm_header;
    
}

ast_algorithm_parameter_list *
create_algorithm_parameter_list(char* name, ast_target_list * target_list)
{
    ast_algorithm_parameter_list * algorithm_parameter_list;
    
    algorithm_parameter_list = malloc(sizeof(ast_algorithm_parameter_list));
    if(!algorithm_parameter_list){
        yyerror("out of memory");
        return NULL;
    }
    
    algorithm_parameter_list->name = name;
    algorithm_parameter_list->target_list = target_list;
    
    PRINT(("%s\n",__func__));
    
    return algorithm_parameter_list;
}

ast_target_list *
create_target_list()
{
    ast_target_list * target_list;
    
    target_list = malloc(sizeof(ast_target_list));
    if (!target_list) {
        yyerror("out of memory");
        return NULL;
    }
    
    target_list->typespec = malloc(sizeof(type_spec*));
    if (!target_list->typespec) {
        yyerror("out of memory");
        return NULL;
    }
    
    //type_spec *tmp = NULL;
    
    //target_list->typespec[0] = tmp;
    target_list->num_of_type_specs = 1;
    
    PRINT(("%s\n", __func__));
    
    return target_list;
    
}

ast_target_list *
add_target_list(ast_target_list * list, ast_type_specifier *type_specifier, char * name)
{
    int num_of_type_specs;
    num_of_type_specs = list->num_of_type_specs + 1;
    
    list->typespec = realloc(list->typespec, num_of_type_specs*sizeof(type_spec*));
    if (!list->typespec) {
        yyerror("out of memory");
        return NULL;
    }
    
    type_spec *typespec;
    typespec->type_specifier = type_specifier;
    typespec->name = name;
    
    list->typespec[num_of_type_specs-1] = typespec;
    list->num_of_type_specs += 1;
    
    PRINT(("%s\n", __func__));
    return list;
}


ast_compound_statement *
create_compound_statement(ast_variable_declaration_list * variable_declaration_list, ast_statement_list * statement_list)
{
    ast_compound_statement * compound_statement;
    
    compound_statement = malloc(sizeof(ast_compound_statement));
    if (!compound_statement) {
        yyerror("out of memory");
        return NULL;
    }
    
    compound_statement->variable_declaration_list = variable_declaration_list;
    compound_statement->statement_list = statement_list;
    
    PRINT(("%s\n", __func__));
    
    return compound_statement;
    
}


ast_variable_declaration_list *
create_variable_declaration_list()
{
    ast_variable_declaration_list * variable_declaration_list;
    
    variable_declaration_list = malloc(sizeof(ast_variable_declaration_list));
    if (!variable_declaration_list) {
        yyerror("out of memory");
        return NULL;
    }
    
    variable_declaration_list->variable_declaration = malloc(sizeof(ast_variable_declaration*));
    if (!variable_declaration_list->variable_declaration) {
        yyerror("out of memory");
        return NULL;
    }
    printf("biu\n");
    variable_declaration_list->num_of_variable_declaration = 0;
    variable_declaration_list->variable_declaration[0] = NULL;
    
    PRINT(("%s\n", __func__));
    
    return variable_declaration_list;
    
}

ast_variable_declaration_list *
add_variable_declaration_list(ast_variable_declaration_list * list, ast_variable_declaration * variable_declaration)
{
    int num_of_variable_declaration;
    num_of_variable_declaration = list->num_of_variable_declaration + 1;
    
    list->variable_declaration = realloc(list->variable_declaration, num_of_variable_declaration*sizeof(ast_variable_declaration*));
    if (!list->variable_declaration) {
        yyerror("out of memory");
        return NULL;
    }
    
    list->variable_declaration[num_of_variable_declaration-1] = variable_declaration;
    list->num_of_variable_declaration += 1;
    
    PRINT(("%s\n", __func__));
    return list;
}

ast_variable_declaration *
create_variable_declaration(ast_type_specifier * type_specifier, char *name)
{
    ast_variable_declaration * variable_declaration;
    
    variable_declaration = malloc(sizeof(ast_variable_declaration));
    if (!variable_declaration) {
        yyerror("out of memory");
        return NULL;
    }
    
    variable_declaration->type_specifier = type_specifier;
    variable_declaration->name = name;
    
    PRINT(("%s\n", __func__));
    
    return variable_declaration;
    
}

ast_type_specifier *
create_type_specifier(ast_arithmetic_type * arithmetic_type)
{
    ast_type_specifier * type_specifier;
    
    type_specifier = malloc(sizeof(ast_type_specifier));
    if (!type_specifier) {
        yyerror("out of memory");
        return NULL;
    }
    
    type_specifier->arithmetic_type = arithmetic_type;
    
    PRINT(("%s\n", __func__));
    
    return type_specifier;
    
}

ast_arithmetic_type *
create_arithmetic_type(int type)
{
    ast_arithmetic_type * arithmetic_type;
    
    arithmetic_type = malloc(sizeof(ast_arithmetic_type));
    if (!arithmetic_type) {
        yyerror("out of memory");
        return NULL;
    }
    
    arithmetic_type->type = type;
    
    PRINT(("%s\n", __func__));
    
    return arithmetic_type;
    
}

ast_statement_list *
create_statement_list()
{
    ast_statement_list * statement_list;
    
    statement_list = malloc(sizeof(ast_statement_list));
    if (!statement_list) {
        yyerror("out of memory");
        return NULL;
    }
    
    statement_list->num_of_statement = 0;
    statement_list->statement = NULL;
    
    PRINT(("%s\n", __func__));
    
    return statement_list;
    
}

ast_statement_list *
add_statement_list(ast_statement_list * list, ast_statement* statement)
{
    int num_of_statement;
    num_of_statement = list->num_of_statement + 1;
    
    list->statement = realloc(list->statement, num_of_statement*sizeof(ast_statement*));
    if (!list->statement) {
        yyerror("out of memory");
        return NULL;
    }
    
    list->statement[num_of_statement-1] = statement;
    list->num_of_statement += 1;
    
    PRINT(("%s\n", __func__));
    return list;
}


ast_statement *
create_statement(ast_set_statement * set_statement)
{
    ast_statement * statement;
    
    statement = malloc(sizeof(ast_statement));
    if (!statement) {
        yyerror("out of memory");
        return NULL;
    }
    
    statement->set_statement = set_statement;
    
    PRINT(("%s\n", __func__));
    
    return statement;
    
}

ast_set_statement *
create_set_statement(int this_type, ast_argument_expression_list *argument_expression_list, ast_expression *expression)
{
    ast_set_statement * set_statement;
    
    set_statement = malloc(sizeof(ast_set_statement));
    if (!set_statement) {
        yyerror("out of memory");
        return NULL;
    }
    
    set_statement->this_type = this_type;
    set_statement->argument_expression_list = argument_expression_list;
    set_statement->expression = expression;
    
    PRINT(("%s\n", __func__));
    
    return set_statement;
    
}

ast_argument_expression_list *
create_argument_expression_list(ast_unary_expression *unary_expression,ast_logical_OR_expression *logical_OR_expression)
{
    ast_argument_expression_list * argument_expression_list;
    
    argument_expression_list = malloc(sizeof(ast_argument_expression_list));
    if (!argument_expression_list) {
        yyerror("out of memory");
        return NULL;
    }
    
    argument_expression_list->unary_expression = unary_expression;
    argument_expression_list->logical_OR_expression = logical_OR_expression;
   
    
    PRINT(("%s\n", __func__));
    
    return argument_expression_list;
    
}

ast_unary_expression *
create_unary_expression(ast_postfix_expression *postfix_expression)
{
    ast_unary_expression * unary_expression;
    
    unary_expression = malloc(sizeof(ast_unary_expression));
    if (!unary_expression) {
        yyerror("out of memory");
        return NULL;
    }
    
    unary_expression->postfix_expression = postfix_expression;
    
    PRINT(("%s\n", __func__));
    
    return unary_expression;
    
}


ast_postfix_expression *
create_postfix_expression(ast_primary_expression *primary_expression)
{
    ast_postfix_expression * postfix_expression;
    
    postfix_expression = malloc(sizeof(ast_postfix_expression));
    if (!postfix_expression) {
        yyerror("out of memory");
        return NULL;
    }
    
    postfix_expression->primary_expression = primary_expression;
    
    PRINT(("%s\n", __func__));
    
    return postfix_expression;
    
}


ast_primary_expression *
create_primary_expression(char * name)
{
    ast_primary_expression * primary_expression;
    
    primary_expression = malloc(sizeof(ast_primary_expression));
    if (!primary_expression) {
        yyerror("out of memory");
        return NULL;
    }
    
    primary_expression->name = name;
    
    PRINT(("%s\n", __func__));
    
    return primary_expression;
    
}


ast_logical_OR_expression *
create_logical_OR_expression(ast_logical_AND_expression *logical_AND_expression)
{
    ast_logical_OR_expression * logical_OR_expression;
    
    logical_OR_expression = malloc(sizeof(ast_logical_OR_expression));
    if (!logical_OR_expression) {
        yyerror("out of memory");
        return NULL;
    }
    
    logical_OR_expression->logical_AND_expression = logical_AND_expression;
    
    PRINT(("%s\n", __func__));
    
    return logical_OR_expression;
    
}

ast_logical_AND_expression *
create_logical_AND_expression(ast_equality_expression *equality_expression)
{
    ast_logical_AND_expression * logical_AND_expression;
    
    logical_AND_expression = malloc(sizeof(ast_logical_AND_expression));
    if (!logical_AND_expression) {
        yyerror("out of memory");
        return NULL;
    }
    
    logical_AND_expression->equality_expression = equality_expression;
    
    PRINT(("%s\n", __func__));
    
    return logical_AND_expression;
    
}


ast_equality_expression *
create_equality_expression(ast_relation_expression *relation_expression)
{
    ast_equality_expression * equality_expression;
    
    equality_expression = malloc(sizeof(ast_equality_expression));
    if (!equality_expression) {
        yyerror("out of memory");
        return NULL;
    }
    
    equality_expression->relation_expression = relation_expression;
    
    PRINT(("%s\n", __func__));
    
    return equality_expression;
    
}


ast_relation_expression *
create_relation_expression(ast_additive_expression *additive_expression)
{
    ast_relation_expression * relation_expression;
    
    relation_expression = malloc(sizeof(ast_relation_expression));
    if (!relation_expression) {
        yyerror("out of memory");
        return NULL;
    }
    
    relation_expression->additive_expression = additive_expression;
    
    PRINT(("%s\n", __func__));
    
    return relation_expression;
    
}


ast_additive_expression *
create_additive_expression(ast_multiplicative_expression *multiplicative_expression)
{
    ast_additive_expression * additive_expression;
    
    additive_expression = malloc(sizeof(ast_additive_expression));
    if (!additive_expression) {
        yyerror("out of memory");
        return NULL;
    }
    
    additive_expression->multiplicative_expression = multiplicative_expression;
    
    PRINT(("%s\n", __func__));
    
    return additive_expression;
    
}


ast_multiplicative_expression *
create_multiplicative_expression(ast_unary_expression *unary_expression)
{
    ast_multiplicative_expression * multiplicative_expression;
    
    multiplicative_expression = malloc(sizeof(ast_multiplicative_expression));
    if (!multiplicative_expression) {
        yyerror("out of memory");
        return NULL;
    }
    
    multiplicative_expression->unary_expression = unary_expression;
    
    PRINT(("%s\n", __func__));
    
    return multiplicative_expression;
    
}

ast_assignment_expression *
create_assignment_expression(int this_type, ast_unary_expression * unary_expression, ast_logical_OR_expression * logical_OR_expression)
{
    ast_assignment_expression * assignment_expression;
    
    assignment_expression = malloc(sizeof(ast_assignment_expression));
    if (!assignment_expression) {
        yyerror("out of memory");
        return NULL;
    }
    
    assignment_expression->this_type = this_type;
    assignment_expression->unary_expression = unary_expression;
    assignment_expression->logical_OR_expression = logical_OR_expression;

    
    PRINT(("%s\n", __func__));
    
    return assignment_expression;
    
}

ast_expression *
create_expression(ast_assignment_expression * assignment_expression)
{
    ast_expression * expression;
    
    expression = malloc(sizeof(ast_expression));
    if (!expression) {
        yyerror("out of memory");
        return NULL;
    }
    
    expression->assignment_expression = malloc(sizeof(ast_assignment_expression*));
    if(!expression->assignment_expression) {
        yyerror("out of memory");
        return NULL;
    }
    expression->assignment_expression[0] = assignment_expression;
    expression->num_of_assignment_expressions = 1;
    
    PRINT(("%s\n", __func__));
    
    return expression;
    
}

ast_expression *
add_expression(ast_expression * list, ast_assignment_expression * assignment_expression)
{
    int num_of_assignment_expressions;
    num_of_assignment_expressions = list->num_of_assignment_expressions +1;
    list->assignment_expression = realloc(list->assignment_expression, num_of_assignment_expressions*sizeof(ast_assignment_expression*));
    if(!list->assignment_expression) {
        yyerror("out of memory");
        return NULL;
    }
    
    list->assignment_expression[num_of_assignment_expressions-1] = assignment_expression;
    list->num_of_assignment_expressions += 1;
    
    PRINT(("%s\n", __func__));
    return list;
}





















/*


ast_uselist *
create_node_uselist(char * name)
{
	ast_uselist * uselist;

	uselist = malloc(sizeof(ast_uselist));
	if (!uselist) {
		yyerror("out of memory");
		return NULL;
	}
	uselist->account = name;

	PRINT(("%s\n", __func__));

	return uselist;

}

ast_stratlist *
create_node_stratlist(ast_strat * strat)
{
	ast_stratlist * stratlist;

	stratlist = malloc(sizeof(ast_stratlist));
	if (!stratlist) {
		yyerror("out of memory");
		return NULL;
	}
	stratlist->strat = strat;

	PRINT(("%s\n", __func__));

	return stratlist;

}

ast_strat *
create_node_strat(char * name, ast_actionlist * actionlist)
{
	ast_strat * strat;

	strat = malloc(sizeof(ast_strat));
	if (!strat) {
		yyerror("out of memory");
		return NULL;
	}
	strat->strategy_name = name;
	strat->actionlist = actionlist;

	PRINT(("%s\n", __func__));

	return strat;
}

ast_actionlist *
create_node_actionlist(ast_order * order)
{
	ast_actionlist * actionlist;

	actionlist = malloc(sizeof(ast_actionlist));
	if (!actionlist) {
		yyerror("out of memory");
		return NULL;
	}

	actionlist->order = malloc(sizeof(ast_order*));
	if (!actionlist->order) {
		yyerror("out of memory");
		return NULL;
	}
	actionlist->order[0] = order;
	actionlist->num_of_orders = 1;

	PRINT(("%s\n", __func__));

	return actionlist;
}

ast_actionlist *
add_order(ast_actionlist* list, ast_order * order)
{
	int num_of_orders;

	num_of_orders = list->num_of_orders + 1;
	list->order = realloc(list->order, num_of_orders*sizeof(ast_order*));
	if (!list->order) {
		yyerror("out of memory");
		return NULL;
	}

	list->order[num_of_orders-1] = order;
	list->num_of_orders += 1;

	PRINT(("%s\n", __func__));
	return list;
}
ast_order *
create_node_order(int order_type, ast_order_item * order_item)
{
	ast_order * order;

	order = malloc(sizeof(ast_order));
	if (!order) {
		yyerror("out of memory");
		return NULL;
	}
	order->order_type = order_type;
	order->order_item = order_item;

	PRINT(("%s\n", __func__));

	return order;
}

ast_order_item *
create_node_order_item(char * equity_identifier, int amount, char *price)  // should be one more para(identifier)
{
	ast_order_item * order_item;

	order_item = malloc(sizeof(ast_order_item));
	if (!order_item) {
		yyerror("out of memory");
		return NULL;
	}
	strcpy(order_item->equity_identifier, equity_identifier);
	order_item->amount = amount;
	strcpy(order_item->price, price);

	PRINT(("%s, %s, %d, %s\n", __func__, order_item->equity_identifier, order_item->amount,  order_item->price));

	return order_item;
} 

void print_tab(int level)
{
	int i = 0;
	for (i = 0; i < level ; i++)
		PRINT(("\t"));
}

void print_order_item (ast_order_item *order_item, int level)
{

	int mylevel = level;
	level++;

	print_tab(mylevel);
	PRINT(("%s start\n", __func__));

	print_tab(mylevel);
	PRINT(("<%s, %d, %s>\n", order_item->equity_identifier, order_item->amount,  order_item->price));

	print_tab(mylevel);
	PRINT(("%s end\n", __func__));

}
void print_order(ast_order *order, int level)
{
	int mylevel = level;
	level++;

	print_tab(mylevel);
	PRINT(("%s start\n", __func__));

	print_tab(mylevel);
	PRINT(("<ORDER: %s>\n", order->order_type== 1 ? "BUY" : "SELL"));
	print_order_item(order->order_item, level);

	print_tab(mylevel);
	PRINT(("%s end\n", __func__));
}

void print_actionlist(ast_actionlist *actionlist, int level)
{
	int mylevel = level;
	int i = 0;
	level++;

	print_tab(mylevel);
	PRINT(("%s start\n", __func__));
	
	print_tab(mylevel);
	PRINT(("<Num of orders: %d>\n", actionlist->num_of_orders));
	for (i = 0; i < actionlist->num_of_orders; i++)
	{
		print_order(actionlist->order[i], level);
	}

	print_tab(mylevel);
	PRINT(("%s end\n", __func__));
}

void print_strat(ast_strat *strat, int level)
{
	int mylevel = level;
	level++;
	print_tab(mylevel);
	PRINT(("%s start\n", __func__));

	print_tab(mylevel);
	PRINT(("<strategy name is %s>\n", strat->strategy_name));
	print_actionlist(strat->actionlist, level);

	print_tab(mylevel);
	PRINT(("%s end\n", __func__));
}

void print_stratlist(ast_stratlist *stratlist, int level)
{
	int mylevel = level;
	level++;

	print_tab(mylevel);
	PRINT(("%s start\n", __func__));

	print_strat(stratlist->strat, level);

	print_tab(mylevel);
	PRINT(("%s end\n", __func__));
}

void print_use_list(ast_uselist * uselist, int level)
{
	int mylevel = level;
	level++;

	print_tab(mylevel);
	PRINT(("%s start\n", __func__));

	print_tab(mylevel);
	PRINT(("<account: %s>\n", uselist->account));

	print_tab(mylevel);
	PRINT(("%s end\n", __func__));
}

void
print_ast(ast_program* prog) 
{
	int level = 0;
	level++;

	PRINT(("%s start\n", __func__));
	print_use_list(prog->uselist, level);
	PRINT(("\n"));
	print_stratlist(prog->stratlist, level);
	PRINT(("%s end\n", __func__));
}
*/
