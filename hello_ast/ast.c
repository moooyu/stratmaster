#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include "ast.h"

//#define DEBUG 1

#ifdef DEBUG
#define PRINT(x)  do { if (DEBUG) dbg_printf x; } while (0)
#else
#define PRINT(x) do {} while(0)
#endif

void dbg_printf(const char *fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	vfprintf(stderr, fmt, args);
	va_end(args);
}

ast_program *
create_node_program(ast_uselist * uselist, ast_stratlist * stratlist)
{
	ast_program * program;

	program = malloc(sizeof(ast_program));
	if (!program) {
		yyerror("out of memory");
		return NULL;
	}

	program->uselist = uselist;
	program->stratlist = stratlist;

	PRINT(("%s\n", __func__));
	PRINT(("End of AST construction\n\n"));

	return program;

}

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
