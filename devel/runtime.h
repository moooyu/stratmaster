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
typedef enum { FALSE_T, TRUE_T } boolean;
typedef enum { EQTY_T = 2, BOND_T } security_type;
typedef enum { BUY_ORDER = 4, SELL_ORDER } order_type;
//typedef enum { DATAFEED = 6, DATABASE } data_type;
typedef enum { USD_T = 8, EUR_T, JPY_T } curr_type;

/****** Structures ****************/
struct price {
	int  curr_t;          /* type of currency */
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
void die(const char *message);
int is_equal_sec(struct security *sec1, struct security *sec2);
void copy_name(char *buf, const char *str);
long price_to_long(const char *pr);
/*  Runtime helper functions */
struct algorithm *create_algorithm(struct data *data_src);
struct account *create_account();
struct data *create_data_source(const char *fname, int data_type);
struct security *create_security(const char *ticker, int type);
struct order *create_order(struct security *sec, int amount, const char *price, int order_type);
void emit_order(struct order_item *my_order);
/*  Order queue functions */
void order_queue_init(struct queue *q);
void queue_destroy(struct queue *q);
void queue_put_order(struct queue *q, struct order *ord_in, const char *strat);
struct order_item *queue_get_order(struct queue *q);

#endif /*   _RUNTIME_H_  */
