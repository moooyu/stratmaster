/*  COMS W4115       PLT       SPRING 2015
 *
 *               STRATMASTER
 *        Demo: a simple ALGORITHM.
 */

#include "runtime.h"

struct cleanup_args {
	struct security *n;
	struct security *t;
	FILE *f;
};

/* Global Variables */
static struct queue order_queue;
static pthread_t order_handler_thread;
static pthread_t algo_thr1;
static struct cleanup_args ca;

/*  
 *    Program-specific variables 
 *    These variables would come
 *    from the symbol table.
 */
static struct account *ac_master;
static struct data *df1;
static struct algorithm *al1;


/**********  Thread handlers  **********/
/*
 *  Thread exit cleanup handler.
 *  When we cancel the ALGORITHM thread, this
 *  cleanup code is executed.
 */
void cleanup_algorithm(void *arg)
{
	fprintf(stderr, "[INFO] EXITING ALGO THREAD: CLEANING UP\n");
	struct cleanup_args cargs = *(struct cleanup_args *)arg;
	if( cargs.n != NULL )
		free(cargs.n);
	if( cargs.t != NULL )
		free(cargs.t);
	if( cargs.f )
		fclose(cargs.f);
}

/*
 *   ALGORITHM thread handler.
 *   This algorithm tests whether the price of ZBRA
 *   drops below $27.00; if so, it returns that price.
 */
void *algorithm_handler(void *arg)
{
	fprintf(stderr, "[INFO] STARTING ALGO THREAD\n");
	struct algorithm *algo = (struct algorithm *)arg;

	char *token_separators = "\t \n";
	char *ticker;
	char *date;
	char *price;
	char *bk;

	/* 
	 *   Sleep interval in microseconds.
	 *   This is 1 sec per year of data.
	 */ 
	unsigned int interval = 2740;
	
	struct security *next_sec = create_security("TEST", EQTY);
	struct security *test_sec = create_security("ZBRA", EQTY);

	ca.n = next_sec;
	ca.t = test_sec;
	ca.f = algo->d->fp;

	pthread_cleanup_push(cleanup_algorithm, &ca);

	char buf[IOBUFSIZE];
	memset(buf, 0, IOBUFSIZE);

	int keep_running = TRUE;
	while( keep_running )
	{
		if( fgets(buf, sizeof(buf), algo->d->fp) != NULL )
		{	
			ticker = strtok_r(buf, token_separators, &bk);
			date   = strtok_r(NULL, token_separators, &bk);
			price  = strtok_r(NULL, token_separators, &bk);

			copy_name(next_sec->sym, ticker);
			long pr = price_to_long(price);

			/* DO TEST */
			if( (is_equal_sec(next_sec, test_sec) == TRUE) && (pr < 2700) )
			{
				keep_running = FALSE;
				fprintf(stderr, "[INFO] ALGO FOUND A PRICE TARGET: $%s\n", price);
				/* Set argument value to new price for return to STRATEGY */
				copy_name(((struct price *)algo->args)->p, price);

				pthread_cond_signal(&algo->cond_true);
				pthread_mutex_lock(&algo->mutex);
				pthread_testcancel();
				pthread_cond_wait(&algo->cond_true, &algo->mutex);
				keep_running = TRUE;
				pthread_mutex_unlock(&algo->mutex);
			}
			memset(buf, 0, IOBUFSIZE);
			usleep(interval);
		}
		else
		{
			keep_running = FALSE;
		}
	}
	if( next_sec != NULL )
		free(next_sec);
	if( test_sec != NULL )
		free(test_sec);
	if( algo->d->fp )
		fclose(algo->d->fp);
	pthread_cond_signal(&algo->cond_true);
	pthread_cleanup_pop(0);
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
		fprintf(stderr, "[INFO] ISSUING ORDER...\n\n");
		emit_order(next_order);

		/* free order_item structures */
		if( next_order->ord != NULL)
			free(next_order->ord);
		if( next_order != NULL)
			free(next_order);	
	}
	return (void *)0;
}

/*
 *  Handle the STRATEGY thread.
 *  This demo shows the use of an ALGORITHM. 
 */
void *strategy_handler(void *arg)
{
	fprintf(stderr, "[INFO] Starting STRATEGY thread.\n");
	int retval;
	void *result;
	
	/*******************************
	 *  STRATEGY BLOCK: process-statement-list
	 ******************************/
	pthread_t algo_thr = (pthread_t)arg;
	
	/* Set up ALGORITHM args */
	al1->num_args = 1;

	struct price *buy_price = (struct price *)malloc(sizeof(struct price));
	if( buy_price == NULL )
		die("malloc failed");

	al1->args = buy_price;

	/* Create the algorithm thread */	
	if( pthread_create(&algo_thr, NULL, algorithm_handler, al1) != 0 )
		die("algorithm thread create fail");

	/************************************** 
	 *       Execute WHEN statement 
	 **************************************/
	pthread_mutex_lock(&al1->mutex);
	pthread_cond_wait(&al1->cond_true, &al1->mutex);

	/*******************************
	 *  STRATEGY BLOCK: action-list
	 ******************************/
	/* If we get here, the ALGORITHM has sent a "true" signal */
	char *strat_name = "my_buy_strat";

	/* create security */
	char *sym_a = "ZBRA";
	int type_a  = EQTY;
	struct security *sec_a = create_security(sym_a, type_a);

	/* Create orders */
	int amt_a = 1000;
	char *pr_a = buy_price->p;
	int t_a = BUY_ORDER;
	struct order *order_a = create_order(sec_a, amt_a, pr_a, t_a);   

	/* Put order into the order queue for execution */
	queue_put_order(&order_queue, order_a, strat_name);
	/* Release lock */
	pthread_mutex_unlock(&al1->mutex);

	/**********************************
	 *  Tear down ALGORITHM thread
	 *********************************/
	if( pthread_cancel(algo_thr) != 0 )
		perror("thread cancellation");

	retval = pthread_join(algo_thr, &result);
	if( retval != 0 )
		perror("thread join");

	if( result != PTHREAD_CANCELED )
		perror("thread was not canceled");	
	
	/* Destroy condition variable */
	pthread_cond_destroy(&al1->cond_true);	

	if( buy_price != NULL )
		free(buy_price);

	fprintf(stderr, "[INFO] Exiting STRATEGY thread.\n");
	return (void *)0;
}


/*
 *  Demo StratMaster program: simple ALGORITHM.
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

	/* Create data source */
	char *df_name = "df_ZBRA";
	if( (df1 = create_data_source(df_name, DATAFEED, EQTY)) == NULL )
		die("error creating data source");

	/* ************************
	 *      ALGORITHM-LIST 
	 * *************************/
	/* Create a new algorithm structure */
	al1 = create_algorithm(df1); 

	/* ************************
	 *      STRATEGY-LIST 
	 * *************************/
	pthread_t my_strat;
	
	if( pthread_create(&my_strat, NULL, strategy_handler, NULL) != 0 )
		die("could not create strategy_handler");
	if( pthread_join(my_strat, NULL) != 0 )
		perror("strategy_handler join");

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
	if( df1 != NULL )
		free(df1);
	if( al1 != NULL )
		free(al1);

	fprintf(stderr, "[INFO] Ending program.\n");
	return 0;
}

