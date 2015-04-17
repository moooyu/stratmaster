#include <stdio.h>
#include <time.h>
#include "ast.h"
#include "y.tab.h"
/*
void ex_ast_order_item(ast_order_item * order_item)
{
    char buf[64];
    time_t local_t;
    struct tm *tmp;
    time(&local_t);
    tmp = localtime(&local_t);
    strftime(buf, 64, "%Y-%m-%d %T",tmp);
    printf("[%s] YOU %s: %d SHARES OF %s AT USD %s\n\n", buf, order_item->order_type== 1 ? "BUY" : "SELL", order_item->amount,  order_item->equity_identifier, order_item->price);
}

void ex_ast_order(ast_order * order)
{
    /////printf("<ORDER: %s>\n", order->order_type== 1 ? "BUY" : "SELL");
    (order->order_item)->order_type = order->order_type;
    ex_ast_order_item(order->order_item);
}

void ex_ast_actionlist(ast_actionlist * actionlist)
{
    int i = 0;
    ///printf("<Num of orders: %d>\n", actionlist->num_of_orders);
    for (i = 0; i < actionlist->num_of_orders; i++)
	{
		ex_ast_order(actionlist->order[i]);
	}
}

void ex_ast_strat(ast_strat * strat)
{
    ////printf("<strategy name is %s>\n", strat->strategy_name);
    printf(" >>>>>> ORDER PLACED BY %s\n", strat->strategy_name);
    ex_ast_actionlist(strat->actionlist);
}

void ex_ast_stratlist(ast_stratlist * stratlist)
{
    ex_ast_strat(stratlist->strat);
}

void ex_ast_uselist(ast_uselist * uselist)
{
    ////printf("<account: %s>\n", uselist->account);
}

void ex_ast(ast_program * prog)
{
    printf("++++++++++++++++++STRATMASTER CONFIRMATION+++++++++++++++++\n");
    ex_ast_uselist(prog->uselist);
    ex_ast_stratlist(prog->stratlist);
    printf("++++++++++++++++++END CONFIRMATION+++++++++++++++++++++++++\n\n");
}

*/
/*
#if 0
void emit_order(Order *my_order)
{

	char buf[64];
	time_t local_t;
	struct tm *tmp;
	time(&local_t);
	tmp = localtime(&local_t);
	strftime(buf, 64, "%Y-%m-%d %T",tmp);


	char *order_type = "";
	switch(my_order->type)
	{
		case BUY_ORDER: order_type = "BOUGHT"; break;
		case SELL_ORDER: order_type = "SOLD"; break;
		default: order_type = "DID SOMETHING ELSE";
	}


	printf("++++++++++++++++++STRATMASTER CONFIRMATION+++++++++++++++++\n");
	printf("[%s] YOU %s: %d SHARES OF %s AT USD %s\n", buf, order_type, my_order->amt, my_order->sym, my_order->price);
	printf(" >>>>>> ORDER PLACED BY %s\n", "stratname");
	printf("++++++++++++++++++END CONFIRMATION+++++++++++++++++++++++++\n\n");
}
#endif
*/
