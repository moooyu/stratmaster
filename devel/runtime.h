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
#include "ast.h"

#define NAMEBUF    32      /* Max size of an identifier or name */
#define IOBUFSIZE  4096   /* size of read buffer */

/******** Enumerations **********/
typedef enum { FALSE_T, TRUE_T } boolean;
typedef enum { EQTY_T = 2, BOND_T } security_type;
typedef enum { BUY_ORDER = 4, SELL_ORDER } order_type;
//typedef enum { DATAFEED = 6, DATABASE } data_type;
typedef enum { USD_T = 8, EUR_T, JPY_T } curr_type;
typedef enum { OP_IS = 256, OP_ISNOT, OP_FUNC, OP_ASSIGN, 
		OP_OR, OP_AND, OP_ADD, OP_SUB, OP_MULT, OP_DIV,
                OP_LT, OP_GT, OP_LTEQ, OP_GTEQ, OP_ATTR } oper_type;
typedef enum { OP_UNARY_MINUS = 512, OP_UNARY_SHARP, OP_UNARY_NOT } oper_unary;

/****** Structures ****************/
struct boolean_object {
	int value;
};

struct integer_object {
	int value;
};

struct double_object {
	double value;
};

struct price_object {
	char price[NAMEBUF];
};

struct currency {
	int  curr_t;          /* type of currency */
	char p[NAMEBUF];      /* amount (e.g. 29.99) */
};

struct security {
	int   sec_t;            /* type of security */
	char  sym[NAMEBUF];     /* ticker symbol */
};

struct position {
	struct security sec;            /* the security object  */
	int    amt;                     /* quantity (e.g. number of shares) */
	struct currency purch_price;       /* purchase price of this position */
};

struct account {
	int    num_positions;           /* number of positions  */
	struct position *positions;     /* array of position objects */
	struct currency avail_cash;     /* available cash to buy securities */
};

struct data {
	int  data_t;                    /* type of data source (DATAFEED or DATABASE) */
	FILE *fp;                       /* file pointer for data file */
	char df_filename[NAMEBUF];      /* file name: e.g. "quotes.dat" */
};

struct data_item {
	struct security sec;            /* the security object */
	struct currency curr_price;        /* the security's current price */
};

struct order {
	struct security sec;            /* the security object */
	int    amt;                     /* quantity of order  */
	int    order_t;                 /* type of order */
	struct currency pr;             /* price */
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
	int  is_dead;                  /* is algorithm dead? */
	int  num_args;                 /* number of arguments */
	void *args;                    /* list of arguments to ALGORITHM */
        void *algo_ptr;      	      /* Pointer to ast node*/
	struct symbol_table *sym;     /* STRATEGY symbol table */
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
void long_to_price(long value, char *buf);
char *type_tostring(int t);
char *oper_type_tostring(int t);
char *node_type_tostring(int t);
char *statement_type_tostring(int t);
/*  Runtime helper functions */
struct boolean_object *create_boolean(int val);
struct integer_object *create_integer(int val);
struct double_object *create_double(double val);
struct price_object *create_price(const char *val);
struct currency *create_currency(int type, const char *prc);
struct security *create_security(int type, const char *ticker);
struct position *create_position(struct security *s, int amt, struct currency *p);
struct account *create_account();
long get_available_cash(struct account *acct);
long can_add_position(struct account *acct, struct order *order);
long add_position(struct account *acct, struct order *order);
struct algorithm *create_algorithm(struct data *data_src);
struct data *create_data_source(const char *fname, int data_type);
struct order *create_order(struct security *security, int amount, struct currency *price, int order_type);
void emit_order(struct order_item *my_order);
/*  Order queue functions */
void order_queue_init(struct queue *q);
void queue_destroy(struct queue *q);
void queue_put_order(struct queue *q, struct order *ord_in, const char *strat);
struct order_item *queue_get_order(struct queue *q);

void* ex_exp(ast_exp *p);
void* ex_stmt (ast_statement *statement);
#endif /*   _RUNTIME_H_  */
