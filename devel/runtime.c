/*  COMS W4115       PLT       SPRING 2015
 *
 *               STRATMASTER
 *             Runtime manager.
 */

#include "runtime.h"

/***************** Utility Functions ***************************/
/*
 *  Helper function for errors.
 */
void die(const char *message)
{
    perror(message);
    exit(1); 
}

/*
 *  Test if two securities are equal.
 *  Returns TRUE iff the two securities have the same
 *  security type AND same symbol, and returns FALSE
 *  otherwise.
 */
int is_equal_sec(struct security *sec1, struct security *sec2)
{
	return (sec1->sec_t == sec2->sec_t) && (strcmp(sec1->sym, sec2->sym) == 0);
}	

/*
 *  Copy a name (identifier) into a buffer.
 *  If the identifier exceeds NAMEBUF length,
 *  we will truncate!
 */
void copy_name(char *buf, const char *str)
{
	if( strlen(str) > (NAMEBUF - 1) )
	{
		strncpy(buf, str, (NAMEBUF - 1));
		buf[NAMEBUF - 1] = '\0';
	}
	else
		strcpy(buf, str);
}

/*
 *  Convert a price string to a long.
 */
long price_to_long(const char *pr)
{
	char buf[NAMEBUF];
	memset(buf, 0, NAMEBUF);
	strcpy(buf, pr);
	char price[NAMEBUF];
	memset(price, 0, NAMEBUF);
	char *token_separators = ".";
	char *dollars;
	char *cents;
	char *bk;

	dollars = strtok_r(buf, token_separators, &bk);
	cents  = strtok_r(NULL, token_separators, &bk);
	strcpy(price, dollars);
	strcat(price, cents);
	return atol(price);
}


/*********** STRATMASTER RUNTIME FUNCTIONS ********************/
/*
 *   Create a new account.
 */
struct account *create_account()
{
	struct account *new_ac = (struct account *)malloc(sizeof(struct account));
	if( new_ac == NULL )
		die("malloc failed");

	/* We generously give each account $100 million to play with. */
	copy_name(new_ac->avail_cash.p, "100000000.00");
	/* Default currency: USD */
	new_ac->avail_cash.curr_t = USD_T;
	return new_ac;
}

/*
 *   Create a data source object.
 */
struct data *create_data_source(const char *dfname, int data_type)
{
	char fname[NAMEBUF];
	memset(fname, 0, NAMEBUF);
	strcpy(fname, dfname);
	char *ext = ".dat";
	strcat(fname, ext);

	struct data *new_data_source = NULL;

	/* Open data file for reading */
	FILE *datafp = fopen(fname, "r");
	if( datafp == NULL )
		die("error opening data file");

	new_data_source = (struct data *)malloc(sizeof(struct data));
	if( new_data_source == NULL )
		die("could not malloc new data source");

	/* fill in data structure */
	new_data_source->data_t = data_type;
	copy_name(new_data_source->df_filename, fname);
	new_data_source->fp = datafp;

	return new_data_source;
}

/*
 *   Create a new security object.
 */
struct security *create_security(const char *ticker, int type)
{
	struct security *new_sec = (struct security *)malloc(sizeof(struct security));
	if( new_sec == NULL )
		die("error mallocing new security");
	copy_name(new_sec->sym, ticker);
	new_sec->sec_t = type;

	return new_sec;
}

/*
 *   Create a new order.
 */
struct order *create_order(struct security *sec, int amount, const char *price, int order_type)
{
	struct order *new_order = (struct order *)malloc(sizeof(struct order));
	if( new_order == NULL )
		die("malloc failed");
	
	new_order->sec = *sec;
	free(sec);
	new_order->amt = amount;
	copy_name(new_order->pr.p, price);
	new_order->order_t = order_type;

	return new_order;
}

/*
 *  Create a new algorithm structure.
 */
struct algorithm *create_algorithm(struct data *data_src)
{
	struct algorithm *new_algo = (struct algorithm *)malloc(sizeof(struct algorithm));
	if( new_algo == NULL )
		die("malloc fail");
	memset(new_algo, 0, sizeof(struct algorithm));

	if( pthread_cond_init(&(new_algo->cond_true), NULL) != 0 )
		die("error initializing cond var");

	if( pthread_mutex_init(&(new_algo->mutex), NULL) != 0 )
	{
		pthread_cond_destroy(&new_algo->cond_true);
		die("error initializing mutex");
	}

	new_algo->d = data_src;
	new_algo->num_args = 0;
	new_algo->args = NULL;
	
	return new_algo;
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



/*
 *   Initialize members of the order queue.
 */
void order_queue_init(struct queue *q)
{
	memset(q, 0, sizeof(*q));

	if( pthread_cond_init(&(q->cond), NULL) != 0 )
		die("error initializing cond var");

	if( pthread_mutex_init(&(q->mutex), NULL) != 0 )
	{
		pthread_cond_destroy(&q->cond);
		die("error initializing mutex");
	}
		
	q->first = NULL;
	q->last  = NULL;
	q->length= 0;
}

/*
 *  Deallocate and destroy everything in the order queue.
 */
void queue_destroy(struct queue *q)
{
	struct order_item *temp;
	/* pop front until queue is empty */
	while(q->length > 0)
	{
		temp = q->first;
		q->first = temp->next;
		q->length--;
		free(temp);
	}

	q->last = NULL;
	
	pthread_mutex_destroy(&q->mutex);
	pthread_cond_destroy(&q->cond);
}

/*
 *  Put an order into the queue and wake up order handler thread.
 */
void queue_put_order(struct queue *q, struct order *ord_in, const char *strat)
{
	struct order_item *new_order_item;

	/* Create a new order_item */
	new_order_item = (struct order_item *)calloc(1, sizeof(struct order_item));
	if( new_order_item == NULL )
		die("could not malloc a new order_item");

	copy_name(new_order_item->strat, strat); 
	new_order_item->ord = ord_in;
	new_order_item->next = NULL;

	/* lock queue first */
	pthread_mutex_lock(&(q->mutex));
	
	/* add the order to end of queue */
	if(q->length == 0)
	{   /* adding to an empty queue */
		q->first = new_order_item;
		q->last  = new_order_item;
	}
	else
	{   /* adding to a non-empty queue */
		q->last->next = new_order_item;
		q->last = new_order_item;
	}

	/* If queue was empty, wake up order handler thread */
	if( q->length == 0 )
		pthread_cond_signal(&(q->cond));
	q->length++;
	/* unlock queue */
	pthread_mutex_unlock(&(q->mutex));
}

/*
 *  Take an order from the queue; block if necessary.
 * 
 */
struct order_item *queue_get_order(struct queue *q)
{
	struct order_item *next_order;

	pthread_mutex_lock(&q->mutex);

	/* If queue is empty, go back to sleep */
	while( q->first == NULL )
	{
		pthread_cond_wait(&q->cond, &q->mutex);
	}
		
	/* We can now get an order */
	next_order = q->first;
	q->first = next_order->next;
	q->length--;
	
	if(q->first == NULL)
	{
		q->last = NULL;
		q->length = 0;
	}

	next_order->next = NULL;
	pthread_mutex_unlock(&q->mutex);	

	return next_order;
}

