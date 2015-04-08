#include <stdio.h>
#include <time.h>
#include "ast.h"
#include "y.tab.h"


int ex_ast (nodeType *p) {
	if (!p) {
		printf("nodeType is NULL\n");
		return 0;
	}

	switch (p->type) {
	case ast_order_item_type:
		//emit_order (p->order);
		break;

	default:
		break;
	}



	return 0;
}

#if 0
void emit_order(Order *my_order)
{
	/*  Get local timestamp */
	char buf[64];
	time_t local_t;
	struct tm *tmp;
	time(&local_t);
	tmp = localtime(&local_t);
	strftime(buf, 64, "%Y-%m-%d %T",tmp);

	/* Determine order type */
	char *order_type = "";
	switch(my_order->type)
	{
		case BUY_ORDER: order_type = "BOUGHT"; break;
		case SELL_ORDER: order_type = "SOLD"; break;
		default: order_type = "DID SOMETHING ELSE";
	}

	/* Print confirmation */
	printf("++++++++++++++++++STRATMASTER CONFIRMATION+++++++++++++++++\n");
	printf("[%s] YOU %s: %d SHARES OF %s AT USD %s\n", buf, order_type, my_order->amt, my_order->sym, my_order->price);
	printf(" >>>>>> ORDER PLACED BY %s\n", "stratname");
	printf("++++++++++++++++++END CONFIRMATION+++++++++++++++++++++++++\n\n");
}
#endif
