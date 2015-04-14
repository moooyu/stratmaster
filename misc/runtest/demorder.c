/*  COMS W4115       PLT       SPRING 2015
 *
 *               STRATMASTER
 *   Demo: STRATEGY just issues orders 
 *         (i.e. HelloWorld)
 */

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

/**********  Thread handlers  **********/

/*
 *  Handle the STRATEGY thread.
 *  EX: similar to HelloWorld, except
 *  we execute 3 orders in a row.
 */
void *strategy_handler(void *arg)
{
	fprintf(stderr, "[INFO] Starting STRATEGY thread.\n");
	/*******************************
	 *  STRATEGY BLOCK: action-list
	 ******************************/
	char *strat_name = "test_a";

	/* Security A */
	char *sym_a = "AAAA";
	int type_a  = EQTY;
	struct security *sec_a = create_security(sym_a, type_a);

	/* Security B */
	char *sym_b = "BBB";
	int type_b  = EQTY;
	struct security *sec_b = create_security(sym_b, type_b);	
		
	/* Security C */
	char *sym_c = "CC";
	int type_c  = EQTY;
	struct security *sec_c = create_security(sym_c, type_c);


	/* Create orders */
	int amt_a = 1000;
	char *pr_a = "19.99";
	int t_a = BUY_ORDER;
	struct order *order_a = create_order(sec_a, amt_a, pr_a, t_a);   
	
	int amt_b = 5000;
	char *pr_b = "205.01";
	int t_b = SELL_ORDER;
	struct order *order_b = create_order(sec_b, amt_b, pr_b, t_b);

	int amt_c = 200;
	char *pr_c = "111.11";
	int t_c = BUY_ORDER;
	struct order *order_c = create_order(sec_c, amt_c, pr_c, t_c);

	/* Put orders into the order queue for execution */
	queue_put_order(&order_queue, order_a, strat_name);
	sleep(1);
	queue_put_order(&order_queue, order_b, strat_name);
	sleep(1);
	queue_put_order(&order_queue, order_c, strat_name);
	sleep(1);

	fprintf(stderr, "[INFO] Exiting STRATEGY thread.\n");
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
		if( next_order->next != NULL )
			free(next_order->next);
		if( next_order->ord != NULL )
			free(next_order->ord);
		if( next_order != NULL )
			free(next_order);	
	}
	return (void *)0;
}



/*
 *     Demo StratMaster program: HelloWorld with 3 orders.
 */
int main(int argc, char *argv[])
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
	if( pthread_create(&my_strat, NULL, strategy_handler, NULL ) != 0 )
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
	return 0;
}


