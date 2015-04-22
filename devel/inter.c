#include <stdio.h>
#include <time.h>
#include "ast.h"
#include "y.tab.h"
#include "runtime.h"

/*     Global Variables    */
static struct queue order_queue;
static pthread_t order_handler_thread;
static struct account *ac_master;
///static struct data *df1;
static char *strat_name;

void run_interp(ast_program * program)
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

     /* Create data source */
     /*char *df_name = "df_ZBRA";
     if( (df1 = create_data_source(df_name, DATAFEED)) == NULL )
         die("error creating data source");*/


    /////start execute startegy_list node
    int tmp =0;
    while (tmp < program->num_of_strategies)
    {
         strat_name = (program->strategy_list[tmp])->name;
         ex_strategy(program->strategy_list[tmp]);
         tmp += 1;
    }
    /* ************************
     *  TEAR DOWN THREAD & CLEAN UP 
     * *************************/
    fprintf(stderr, "[INFO] Cleaning up.\n");
    /* Tear down threads */
    sleep(1);
    if( pthread_cancel(order_handler_thread) != 0 )
        perror("order_handler_thread cancellation");

    retval = pthread_join(order_handler_thread, &result);
    if( retval != 0 )
        perror("order_handler_thread join");

    if( result != PTHREAD_CANCELED )
        perror("order_handler_thread was not canceled");

    queue_destroy(&order_queue);	

    /* free account */
    if( ac_master != NULL)
	free(ac_master);
    /*if( df1 != NULL )
	free(df1);*/

    fprintf(stderr, "[INFO] Ending program.\n");

}


void ex_strategy(ast_strategy * strategy)
{
    ////start strategy thread
    pthread_t my_strat;
    if( pthread_create(&my_strat, NULL, strategy_handler, strategy) != 0 )
	die("could not create strategy_handler");
    if( pthread_join(my_strat, NULL) != 0 )
	perror("strategy_handler join");
   ////
}

//////called in ex_strategy
void *strategy_handler(void *arg)
{
    fprintf(stderr, "[INFO] Starting STRATEGY thread.\n");
    ast_strategy * strategy = (ast_strategy *)arg;
    ////iterating over orders
    int tmp = 0;
    while (tmp < strategy->num_of_orders)
    {
        /////execute order
        ex_order_item(strategy->order_list[tmp]);
        tmp += 1;
    }
}

void ex_order_item(ast_order_item * order_item)
{
     char *sym = order_item->security_name;
     int type  = order_item->security_type;
     struct security *sec = create_new_security(sym, type);
     ////////
     int amt = order_item->number;
     char *pr = order_item->price;
     int t = order_item->type;
     struct order *order = create_new_order(sec, amt, pr, t);/////need sec
     queue_put_order(&order_queue, order, strat_name);
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














































