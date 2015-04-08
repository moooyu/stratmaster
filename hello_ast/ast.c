
#include <stdio.h>
#include <stdlib.h>
#include "ast.h"


ast_program *
create_node_program(ast_uselist * uselist, ast_stratlist * stratlist)
{
	ast_program * program;
	program = malloc(sizeof(ast_program));
    program->uselist = uselist;
	program->stratlist = stratlist;
    
    return program;

}

ast_uselist *
create_node_uselist(char * name)
{
   	 ast_uselist * uselist;
	 uselist = malloc(sizeof(ast_uselist));
     uselist->account = name;
 
	 return uselist;

}

ast_stratlist *
create_node_stratlist(ast_strat * strat)
{
	ast_stratlist * stratlist;
	stratlist = malloc(sizeof(ast_stratlist));
    stratlist->strat = strat;
    
    return stratlist;

}


ast_strat *
create_node_strat(char * name, ast_actionlist * actionlist)
{
	ast_strat * strat;
	strat = malloc(sizeof(ast_strat));
    strat->strategy_name = name;
    strat->actionlist = actionlist;
	
	return strat;
}

ast_actionlist *
create_node_actionlist(ast_order * order)
{
	ast_actionlist * actionlist;
	actionlist = malloc(sizeof(ast_actionlist));
	actionlist->order = order;

	return actionlist;
}

ast_order *
create_node_order(int order_type, ast_order_item * order_item)
{
	ast_order * order;
	order = malloc(sizeof(ast_order));
	order->order_type = order_type;
	order->order_item = order_item;

	return order;
}


ast_order_item *
create_node_order_item(char * equity_identifier, int amount, int price)  // should be one more para(identifier)
{
	ast_order_item * order_item;
	order_item = malloc(sizeof(ast_order_item));
	order_item->equity_identifier = equity_identifier;
	order_item->amount = amount;
	order_item->price = price;
} 


