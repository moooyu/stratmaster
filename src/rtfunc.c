/*  COMS W4115       PLT       SPRING 2015
 *
 *               STRATMASTER
 *   Demo: STRATEGY just issues orders 
 *         (i.e. HelloWorld)
 */
#include <stdio.h>
#include <time.h>
#include "ast.h"
#include "y.tab.h"
#include "runtime.h"

/*     Global Variables    */
static struct queue order_queue;
static pthread_t order_handler_thread;

/*  
 *    Program-specific variables 
 *    These variables would come
 *    from the symbol table.
 */
static struct account *ac_master;

////combined with ex_ast_order_item
void add_order_item(ast_order_item * order_item)
{
     ////a fake security
     char *strat_name = "test_a";
     char *sym_a = order_item->equity_identifier;
     int type_a  = EQTY;
     struct security *sec_a = create_security(sym_a, type_a);
     ////////
     int amt = order_item->amount;
     char *pr = order_item->price;
     int t = (order_item->order_type== 1 ? BUY_ORDER : SELL_ORDER);
     struct order *order = create_order(sec_a, amt, pr, t);/////need sec
     queue_put_order(&order_queue, order, strat_name);
     ///sleep(1);
     fprintf(stderr, "[INFO] Exiting STRATEGY thread.\n");
     return;
}

/////start strategy from the ast_strat node
void *strategy_handler(void *arg)
{
	ast_strat * strat = (ast_strat *)arg;
        ex_ast_strat(strat);
	return (void *)0;
}

/*
 *  Handle an order.
 *  This is the handler function for the order handler thread.
 *
 *  Thread waits on a condition variable to acquire a lock
 *  on the order queue in order to retrieve the next order
 *  that needs to be emitted.
 *
 */
void *order_handler(void *arg)
{
	struct order_item *next_order;

	while(1)
	{
		/* Wait for order */
		next_order = queue_get_order(&order_queue);

		/* Issue the order */
		fprintf(stderr, "[INFO] ISSUING ORDER.\n");
		emit_order(next_order);

		/* free order_item structures */
		
		if( next_order->ord != NULL )
			free(next_order->ord);
		if( next_order != NULL )
			free(next_order);	
	}
	return (void *)0;
}



void strat_start(ast_strat * main_strat)
{
	fprintf(stderr, "[INFO] Starting program.\n");
	/***************************
	 *    INITIALIZATION
	 **************************/ 
	int retval;
	void *result;

	/* initialize the order queue */
	order_queue_init(&order_queue);

	/* create the order handler thread */
	if( pthread_create(&order_handler_thread, NULL, order_handler, NULL ) != 0 )
		die("could not create order_handler_thread");

	fprintf(stderr, "[INFO] Initialization complete.\n");
	/* ************************
	 *        USE-LIST 
	 * *************************/
	/* Create account */
	ac_master = create_account(); 


	/* ************************
	 *      STRATEGY-LIST 
	 * *************************/
	pthread_t my_strat;
	if( pthread_create(&my_strat, NULL, strategy_handler, main_strat ) != 0 )
		die("could not create strategy_handler");
	if( pthread_join(my_strat, NULL) != 0 )
		perror("strategy_handler join");


	/* ************************
	 *  TEAR DOWN THREAD & CLEAN UP 
	 * *************************/
	fprintf(stderr, "[INFO] Cleaning up.\n");
	/* Tear down threads */
	sleep(2);
	if( pthread_cancel(order_handler_thread) != 0 )
		perror("order_handler_thread cancellation");

	retval = pthread_join(order_handler_thread, &result);
	if( retval != 0 )
		perror("order_handler_thread join");

	if( result != PTHREAD_CANCELED )
		perror("order_handler_thread was not canceled");

	queue_destroy(&order_queue);	

	/* free account */
	free(ac_master);

	fprintf(stderr, "[INFO] Ending program.\n");
	
}


/*
 *   Issue an order.
 */
void emit_order(struct order_item *my_order)
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
	switch(my_order->ord->order_t)
	{
		case BUY_ORDER: order_type = "BOUGHT"; break; 
		case SELL_ORDER: order_type = "SOLD"; break; 
		default: order_type = "DID SOMETHING ELSE";
	}

	/* Print confirmation */
	printf("++++++++++++++++++STRATMASTER CONFIRMATION+++++++++++++++++\n");
	printf("[%s] YOU %s: %d SHARES OF %s AT USD %s\n", buf, order_type, my_order->ord->amt, 
			my_order->ord->sec.sym, my_order->ord->pr.p);
	printf(" >>>>>> ORDER PLACED BY %s\n", my_order->strat);
	printf("++++++++++++++++++END CONFIRMATION+++++++++++++++++++++++++\n\n");
}
