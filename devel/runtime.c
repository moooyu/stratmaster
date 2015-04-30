/*  COMS W4115       PLT       SPRING 2015
 *
 *               STRATMASTER
 *             Runtime manager.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "symtab.h"
#include "ast.h"
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

/*
 * Convert a long to a price string.
 */
void long_to_price(long value, char *buf)
{
	memset(buf, 0, sizeof(buf));
	char resultbuf[NAMEBUF];
	char tempbuf[NAMEBUF];

	long dollars = value / 100L;
	long cents   = value - (dollars * 100L);
	//fprintf(stderr, "DOLLARS = %ld CENTS = %02ld\n", dollars, cents);	
	sprintf(resultbuf, "%ld.", dollars);
	sprintf(tempbuf, "%02ld", cents);
        strcat(resultbuf, tempbuf);

	//fprintf(stderr, "DOLLARS = %s CENTS = %s\n", resultbuf, tempbuf);
	copy_name(buf, resultbuf);
}


/*********** STRATMASTER RUNTIME FUNCTIONS ********************/
/*
 *   Create a BOOLEAN object.
 */
struct boolean_object *create_boolean(int val)
{
	struct boolean_object *new_bool = (struct boolean_object *)malloc(sizeof(struct boolean_object));
	if( new_bool == NULL )
		die("malloc failed in create_boolean");

	new_bool->value = val;

	return new_bool;
}


/*
 *   Create an INT object.
 */
struct integer_object *create_integer(int val)
{
	struct integer_object *new_int = (struct integer_object *)malloc(sizeof(struct integer_object));
	if( new_int == NULL )
		die("malloc failed in create_integer");

	new_int->value = val;

	return new_int;
}

/*
 *   Create a DOUBLE object.
 */
struct double_object *create_double(double val)
{
	struct double_object *new_double = (struct double_object *)malloc(sizeof(struct double_object));
	if( new_double == NULL )
		die("malloc failed in create_double");

	new_double->value = val;

	return new_double;
}

/*
 *   Create a price object.
 */
struct price_object *create_price(const char *val)
{
	struct price_object *new_price = (struct price_object *)malloc(sizeof(struct price_object));
	if( new_price == NULL )
		die("malloc failed in create_price");

	copy_name(new_price->price, val);

	return new_price;
}

/*
 *   Create a new currency object.
 *   PARAMS: 
 *      curr: the currency type; if 0, use default USD
 *      prc : the price
 * 
 */
struct currency *create_currency(int type, const char *prc)
{
	struct currency *new_curr = (struct currency *)malloc(sizeof(struct currency));
	if( new_curr == NULL )
		die("malloc failed");

	/* copy price */
	if( prc == NULL )
	{ /* Default value: 0.00 */
		copy_name(new_curr->p, "0.00");
	}
	else
		copy_name(new_curr->p, prc);
	
	/* set currency type */
	if( type == 0 )
	{     /* Default currency: USD */
		new_curr->curr_t = USD_T;
	}
	else
		new_curr->curr_t = type;

	return new_curr;
}

/*
 *   Create a new security object.
 */

struct security *create_security(int type, const char *ticker)
{
	struct security *new_sec = (struct security *)malloc(sizeof(struct security));
	if( new_sec == NULL )
		die("error mallocing new security");

	if( ticker == NULL )
	{
		copy_name(new_sec->sym, " ");
	}
	else
		copy_name(new_sec->sym, ticker);
	
	/* Set security type */
	if( type == 0 )
	{  /*  Default security: EQTY */
		new_sec->sec_t = EQTY_T;
	}
	else
		new_sec->sec_t = type;

	return new_sec;
}

/*
 *   Create a new position object.
 */

struct position *create_position(struct security *s, int amt, struct currency *p)
{
	struct position *new_pos = (struct position *)malloc(sizeof(struct position));
	if( new_pos == NULL )
		die("error mallocing new position");
	new_pos->sec = *s;
	new_pos->amt = amt;
	new_pos->purch_price = *p;

	return new_pos;
}



/*
 *   Create a new account.
 */
struct account *create_account()
{
	struct account *new_acct = (struct account *)malloc(sizeof(struct account));
	if( new_acct == NULL )
		die("malloc failed");

	/* We generously give each account $10,000 to play with. */
	struct currency *cash = create_currency(0, "10000.00");
	new_acct->avail_cash = *cash;
	free(cash);

	new_acct->num_positions = 0;
	new_acct->positions = NULL;

	return new_acct;
}

/*
 *  Get amount of available cash in an account.
 *  Return value: the numerical amount of cash as a long
 *  (NOTE: this does not check currency type).
 */
long get_available_cash(struct account *acct)
{
	return price_to_long(acct->avail_cash.p);
}

/*
 *  Determine if you can add a position to an account.
 *  Return value: the amount of cash remaining if this order
 *      were placed.  A negative value indicates rejection
 *      of the order, since we would not have enough cash.
 */
long can_add_position(struct account *acct, struct order *order)
{
	long cash_bal = get_available_cash(acct);
	return cash_bal - (order->amt * price_to_long(order->pr.p));
}

/*
 *  Add a position to an account.
 */
long add_position(struct account *acct, struct order *order)
{
	int i = acct->num_positions;

	if( i == 0 )
	{    /* need to create a brand new position */
		acct->positions = (struct position *)malloc(sizeof(struct position));
		if( acct->positions == NULL )
			die("malloc failed in add_position\n");

		memcpy(&(acct->positions[0].sec), &order->sec, sizeof(struct security));
		memcpy(&(acct->positions[0].amt), &order->amt, sizeof(int));
		memcpy(&(acct->positions[0].purch_price), &order->pr, sizeof(struct currency));
		fprintf(stderr, "[INFO] New position added: %s : %d shares at %s\n", acct->positions[0].sec.sym, acct->positions[0].amt, acct->positions[0].purch_price.p);
	}
	else
	{      /* Search for an existing position with this security */
		int found = 0;
		struct position *temp_pos;
		while( i > 0 )
		{
			if( is_equal_sec(&acct->positions[i].sec, &order->sec) )
			{
				temp_pos = &acct->positions[i];
				found = 1;
				break;
			}
			i--;
		}

		if( found )
		{  /* add to an existing position */
			temp_pos->amt += order->amt;
			copy_name(temp_pos->purch_price.p, order->pr.p);

		}
		else
		{  /* need to add a brand new position */
			int size = acct->num_positions + 1;
			acct->positions = (struct position *)realloc(acct->positions, sizeof(struct position) * size);
			if( acct->positions == NULL )
				die("malloc failed in add_position\n");

			memcpy(&(acct->positions[acct->num_positions].sec), &order->sec, sizeof(struct security));
			memcpy(&(acct->positions[acct->num_positions].amt), &order->amt, sizeof(int));
			memcpy(&(acct->positions[acct->num_positions].purch_price), &order->pr, sizeof(struct currency));
			fprintf(stderr, "[INFO] New position added: %s : %d shares at %s\n", acct->positions[acct->num_positions].sec.sym, 
					acct->positions[acct->num_positions].amt, acct->positions[acct->num_positions].purch_price.p);

		}

	}

	acct->num_positions++;

	/* update account available cash balance */
	long new_bal = get_available_cash(acct) - (order->amt * price_to_long(order->pr.p)); 
	long_to_price(new_bal, acct->avail_cash.p);

	return new_bal;
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
 *   Create a new order.
 */
struct order *create_order(struct security *security, int amount, struct currency *price, int order_type)
{
	struct order *new_order = (struct order *)malloc(sizeof(struct order));
	if( new_order == NULL )
		die("malloc failed");
	
	new_order->sec = *security;
	free(security);
	new_order->amt = amount;
	new_order->pr = *price;
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
	new_algo->is_dead = 0;
	new_algo->args = NULL;
        new_algo->algo_ptr = NULL;
	
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

	/* Determine currency type */
	char *curr_type = "";
	switch(my_order->ord->pr.curr_t)
	{
		case USD_T: curr_type = "USD"; break; 
		case EUR_T: curr_type = "EUR"; break;
		case JPY_T: curr_type = "JPY"; break;
		default: curr_type = "XXX";
	}

	/* Print confirmation */
	printf("++++++++++++++++++STRATMASTER CONFIRMATION+++++++++++++++++\n");
	printf("[%s] YOU %s: %d SHARES OF %s AT %s %s\n", buf, order_type, my_order->ord->amt, 
			my_order->ord->sec.sym, curr_type, my_order->ord->pr.p);
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

char *type_tostring(int t)
{
	char *str = "";
	switch(t)
	{
		case INT_T: str = "INT"; break;
		case DOUBLE_T: str = "DOUBLT"; break;
		case BOOLEAN_T: str = "BOOLEAN"; break;
		case SECURITY_T: str = "SECURITY"; break;
		case CURRENCY_T: str = "CURRENCY"; break;
		case VOID_T: str = "VOID"; break;
		case ACCOUNT_T: str = "ACCOUNT"; break;
		case DATAFEED_T: str = "DATAFEED"; break;
		case DATABASE_T: str = "DATABASE"; break;
		case EXCHANGE_T: str = "EXCHANGE"; break;
		case STRATEGY_T: str = "STRATEGY"; break;
		case ALGORITHM_T: str = "ALGORITHM"; break;
		case FUNCTION_T: str = "FUNCTION"; break;
		default: str = "UNKNOWN TYPE";
	}

	return str;
}


char *statement_type_tostring(int t)
{
	char *str = "";
	switch(t)
	{
		case expression_ST: str = "EXPRESSION STATEMENT"; break;
		case compound_ST: str = "COMPOUND STATEMENT"; break;
		case selection_ST: str = "SELECTION STATEMENT"; break;
		case iteration_ST: str = "ITERATION STATEMENT"; break;
		case set_ST: str = "SET STATEMENT"; break;
		default: str = "UNKNOWN STATEMENT TYPE";
	}

	return str;
}


char *node_type_tostring(int t)
{
	char *str = "";
	switch(t)
	{
		case typeIntegerConst: str = "INTEGER CONSTANT"; break;
		case typeBooleanConst: str = "BOOLEAN CONSTANT"; break;
		case typeDoubleConst: str = "DOUBLE CONSTANT"; break;
		case typePriceConst: str = "PRICE CONSTANT"; break;
		case typeOper: str = "OPERATOR"; break;
		case typeID: str = "ID"; break;
		case typeKeyword: str = "KEYWORD"; break;
		case typeArgulist: str = "ARGUMENT LIST"; break;
		default: str = "UNKNOWN EXP NODE TYPE";
	}

	return str;
}

char *oper_type_tostring(int t)
{
	char *str = "";
	switch(t)
	{
		case OP_IS: str = "OP_IS"; break;
		case OP_ISNOT: str = "OP_ISNOT"; break;
		case OP_FUNC: str = "OP_FUNC"; break;
		case OP_ASSIGN: str = "OP_ASSIGN"; break;
		case OP_OR: str = "OP_OR"; break;
		case OP_AND: str = "OP_AND"; break;
		case OP_ADD: str = "OP_ADD"; break;
		case OP_SUB: str = "OP_SUB"; break;
		case OP_MULT: str = "OP_MULT"; break;
		case OP_DIV: str = "OP_DIV"; break;
		case OP_LT: str = "OP_LT"; break;
		case OP_GT: str = "OP_GT"; break;
		case OP_LTEQ: str = "OP_LTEQ"; break;
		case OP_GTEQ: str = "OP_GTEQ"; break;
		case OP_UNARY_SHARP: str = "OP_UNARY_SHARP"; break;
		case OP_UNARY_MINUS: str = "OP_UNARY_MINUS"; break; 
		case OP_UNARY_NOT: str = "OP_UNARY_NOT"; break;
		case OP_ATTR: str = "OP_ATTR"; break;
		default: str = "UNKNOWN OPERATOR TYPE";
	}

	return str;
}

