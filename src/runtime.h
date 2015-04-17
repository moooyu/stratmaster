/*  COMS W4115       PLT       SPRING 2015
 *
 *               STRATMASTER
 *     Header file for runtime manager.
 */

#ifndef _RUNTIME_H_
#define _RUNTIME_H_

#include <stdio.h>      /* for printf() and fprintf() */
#include <stdlib.h>     /* for atoi() and exit() */
#include <string.h>     /* for memset() */
#include <unistd.h>     /* for close() */
#include <time.h>       /* for time() */
#include <signal.h>     /* for signal() */
#include <sys/stat.h>   /* for stat() */
#include <pthread.h>    /* for threads */

#define NAMEBUF    32      /* Max size of an identifier or name */
#define IOBUFSIZE  4096   /* size of read buffer */

/******** Enumerations **********/
typedef enum { FALSE, TRUE } boolean;
typedef enum { EQTY_V = 2, BOND } security_type;
typedef enum { BUY_ORDER = 4, SELL_ORDER } order_type;
typedef enum { DATAFEED = 6, DATABASE } data_type;

/****** Structures ****************/
struct price {
	char p[NAMEBUF];
};

struct security {
	int   sec_t;            /* type of security */
	char  sym[NAMEBUF];     /* ticker symbol */
};

struct position {
	struct security sec;            /* the security object  */
	int    amt;                     /* quantity (e.g. number of shares) */
	struct price purch_price;       /* purchase price of this position */
};

struct account {
	struct position *positions;     /* array of position objects */
	struct price avail_cash;        /* available cash to buy securities */
};

struct data {
	int  data_t;                    /* type of data source (DATAFEED or DATABASE) */
	FILE *fp;                       /* file pointer for data file */
	int  sec_t;                     /* type of securities in this data file */
	char df_filename[NAMEBUF];      /* file name: e.g. "quotes.dat" */
};

struct data_item {
	struct security sec;            /* the security object */
	struct price curr_price;        /* the security's current price */
};

struct order {
	struct security sec;            /* the security object */
	int    amt;                     /* quantity of order  */
	int    order_t;                 /* type of order */
	struct price pr;                /* price */
};

struct order_item {
	struct order *ord;             /* the order object */
	char  strat[NAMEBUF];          /* name of strategy placing order */
	struct order_item *next;       /* next order in list */
};

struct algorithm {
	pthread_mutex_t mutex;         /* lock */
	pthread_cond_t cond_true;      /* condition variable */
	struct data *d;                /* the data source */
	int  num_args;                 /* number of arguments */
	void *args;                    /* list of arguments to ALGORITHM */
};

struct queue {
    pthread_mutex_t mutex;            /* mutex used to protect the queue */
    pthread_cond_t cond;              /* condition variable for thread to sleep on */
    struct order_item *first;         /* first order in the queue */
    struct order_item *last;          /* last order in the queue */
    unsigned int length;              /* number of orders in the queue */
};


/********** Function Declarations *********************/
/*  Utility functions */
static void die(const char *message);
static int is_equal_sec(struct security *sec1, struct security *sec2);
static void copy_name(char *buf, const char *str);
static long price_to_long(const char *pr);
/*  Runtime helper functions */
static struct algorithm *create_algorithm(struct data *data_src);
static struct account *create_account();
static struct data *create_data_source(const char *fname, int data_type, int sec_type);
static struct security *create_security(const char *ticker, int type);
static struct order *create_order(struct security *sec, int amount, const char *price, int order_type);
void emit_order(struct order_item *my_order);
/*  Order queue functions */
static void order_queue_init(struct queue *q);
static void queue_destroy(struct queue *q);
static void queue_put_order(struct queue *q, struct order *ord_in, const char *strat);
static struct order_item *queue_get_order(struct queue *q);

/***************** Utility Functions ***************************/
/*
 *  Helper function for errors.
 */
static void die(const char *message)
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
static int is_equal_sec(struct security *sec1, struct security *sec2)
{
	return (sec1->sec_t == sec2->sec_t) && (strcmp(sec1->sym, sec2->sym) == 0);
}	

/*
 *  Copy a name (identifier) into a buffer.
 *  If the identifier exceeds NAMEBUF length,
 *  we will truncate!
 */
static void copy_name(char *buf, const char *str)
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
static long price_to_long(const char *pr)
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
static struct account *create_account()
{
	struct account *new_ac = (struct account *)malloc(sizeof(struct account));
	if( new_ac == NULL )
		die("malloc failed");

	/* We generously give each account $100 million to play with. */
	copy_name(new_ac->avail_cash.p, "100000000.00");

	return new_ac;
}

/*
 *   Create a data source object.
 */
static struct data *create_data_source(const char *dfname, int data_type, int sec_type)
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
	new_data_source->sec_t = sec_type;
	new_data_source->fp = datafp;

	return new_data_source;
}

/*
 *   Create a new security object.
 */
static struct security *create_security(const char *ticker, int type)
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
static struct order *create_order(struct security *sec, int amount, const char *price, int order_type)
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
static struct algorithm *create_algorithm(struct data *data_src)
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
 *   Initialize members of the order queue.
 */
static void order_queue_init(struct queue *q)
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
static void queue_destroy(struct queue *q)
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
static void queue_put_order(struct queue *q, struct order *ord_in, const char *strat)
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
static struct order_item *queue_get_order(struct queue *q)
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

	pthread_mutex_unlock(&q->mutex);	

	return next_order;
}


#endif /*   _RUNTIME_H_  */

