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
 *  Convert a long to a price string with commas for display purposes.
 */
void long_to_price_display(long value, char *buf)
{
	memset(buf, 0, sizeof(buf));
	char resultbuf[NAMEBUF];
	char tempbuf[NAMEBUF];

	long dollars = value / 100L;
	long cents   = value - (dollars * 100L);
	sprintf(resultbuf, "%'ld.", dollars);
	sprintf(tempbuf, "%02ld", cents);
        strcat(resultbuf, tempbuf);
	copy_name(buf, resultbuf);
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
	sprintf(resultbuf, "%ld.", dollars);
	sprintf(tempbuf, "%02ld", cents);
        strcat(resultbuf, tempbuf);
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
/*
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
*/

struct position *create_position(struct security *s, int amt, struct currency *p)
{
	struct position *new_pos = (struct position *)malloc(sizeof(struct position));
	if( new_pos == NULL )
		die("error mallocing new position");

	memcpy((void*)&(new_pos->sec), (void*)s, sizeof(struct security));
	new_pos->total_shares = amt;
	if( p == NULL )
		new_pos->total_cost = 0L;
	else
		new_pos->total_cost = amt * price_to_long(p->p);
	new_pos->next = NULL;

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

	/* We generously give each account $100,000,000 to play with. */
	struct currency *cash = create_currency(0, "100000000.00");
	new_acct->avail_cash = *cash;
	free(cash);
	new_acct->cash_bal = price_to_long(new_acct->avail_cash.p);

	new_acct->positions = NULL;

	return new_acct;
}

/*
 *  Get amount of available cash in an account.
 *  Return value: the numerical amount of cash as a long
 *  (NOTE: this does not check currency type).
 */
long get_cash_balance(struct account *acct)
{
	return acct->cash_bal;
}

/*
 *  Determine if you can add a position to an account.
 *  Return value: the amount of cash remaining if this order
 *      were placed.  A negative value indicates rejection
 *      of the order, since we would not have enough cash.
 */
long can_add_position(struct account *acct, struct order *order)
{
	return get_cash_balance(acct) - (order->amt * price_to_long(order->pr.p));
}

/*
 *  Determine if you can sell a position from an account.
 *  Return value: the number of shares left if you sold
 *  this position. A negative value indicates rejection of
 *  the order, as you either don't own the security or are
 *  trying to sell more shares than you own. 
 */
int can_sell_position(struct account *acct, struct order *order)
{
	int shares_left = -1;

	struct position *temp_pos = create_position(&order->sec, order->amt, &order->pr);
	temp_pos = get_position(acct, &temp_pos->sec);
	if( temp_pos != NULL  )
		shares_left = temp_pos->total_shares - order->amt;

	return shares_left;
}

/*
 *  Add a position to an account.
 */
void add_position(struct account *acct, struct order *order)
{
	struct position *existing_pos = NULL;	
	struct position *new_pos = create_position(&order->sec, order->amt, &order->pr);
	
	/* Update account's cash balance */
	acct->cash_bal = acct->cash_bal -  new_pos->total_cost;
	long_to_price(acct->cash_bal, acct->avail_cash.p);

	/* Add this position to the account */
	if( acct->positions == NULL )
	{
		acct->positions = new_pos; 
	}
	else
	{  //There are existing positions; check if we already own this security
		existing_pos = get_position(acct, &(new_pos->sec)); 
		if( existing_pos )
		{
			existing_pos->total_shares += new_pos->total_shares;
			existing_pos->total_cost   += new_pos->total_cost;
			free(new_pos);
		}
		else
		{
			//We don't own this position, so add to end of list	
			struct position *curr_pos = acct->positions;
			while( curr_pos->next != NULL )
			{
				curr_pos = curr_pos->next;
			}
			curr_pos->next = new_pos;
		}
	}
	//Calculate cost
	char buf[NAMEBUF];
	memset(buf, 0, NAMEBUF);
	long cost = order->amt * price_to_long(order->pr.p);
	long_to_price_display(cost, buf);
	fprintf(stdout, ">>>> Adding Position: %'d shares of %s at %s     COST: %12s\n", order->amt, order->sec.sym, order->pr.p, buf);
}

/*
 *  Get a position from an account.
 *  Returns, but does not remove, the position.
 *  Returns NULL if position is not in the account, or there are no positions.
 */
struct position *get_position(struct account *acct, struct security *sec)
{
	if( acct->positions == NULL )
		return NULL;

	struct position *pos = NULL;

	pos = acct->positions;

	while( pos )
	{
		if( !is_equal_sec(&(pos->sec), sec) )
			pos = pos->next;
		else
			break;
	}
	return pos;
}

/*
 *  Subtract from an existing position.
 *  Called when a SELL order is placed.
 *  PRECONDITION: we own this position with enough shares to sell.
 *  MUST CHECK WITH can_sell_position() first.
 *  If position completely sold off, then we remove the position 
 *  from the account.
 */
void subtract_position(struct account *acct, struct order *order)
{
	struct position *curr_pos = get_position(acct, &order->sec);
	/* Update number of shares */
	curr_pos->total_shares -= order->amt;
	if( curr_pos->total_shares == 0 )
		remove_position(acct, &order->sec);
	/* Update account's cash balance */
	long proceeds = order->amt * price_to_long(order->pr.p);
	acct->cash_bal = acct->cash_bal + proceeds;
	long_to_price(acct->cash_bal, acct->avail_cash.p);

	char buf[NAMEBUF];
	memset(buf, 0, NAMEBUF);
	long_to_price_display(proceeds, buf);
	fprintf(stdout, ">>>> Selling Position: %'d shares of %s at %s PROCEEDS: %12s\n", order->amt, order->sec.sym, order->pr.p, buf);
}

/*
 *  Remove this position from the account.
 */
struct position *remove_position(struct account *acct, struct security *sec)
{
	if( acct->positions == NULL || get_position(acct, sec) == NULL )
		return NULL;

	//We know this position exists; now we find & return it	
	struct position *curr_pos = acct->positions;
	struct position *prev_pos = acct->positions;

	if ( is_equal_sec(&(curr_pos->sec), sec) )
	{  //remove first item
		acct->positions = curr_pos->next;
	}
	else
	{  //it's somewhere down the list
		while( curr_pos )
		{
			if( !is_equal_sec(&curr_pos->sec, sec) )
			{
				prev_pos = curr_pos;
				curr_pos = curr_pos->next;
			}
			else
			{
				prev_pos->next = curr_pos->next;
				break;
			}
		}
	}
	//Update account's cash balance
//	acct->cash_bal = acct->cash_bal + curr_pos->total_cost;
//	long_to_price(acct->cash_bal, acct->avail_cash.p);

	return curr_pos;
}


/*
 *  Print summary of account holdings.
 */
void print_account_summary(struct account *acct, const char *name, struct position *pricedata, int num)
{
	fprintf(stdout, "ACCOUNT summary for %s:\n", name);
	
	char valuebuf[NAMEBUF];              //value of securities
	char cashbuf[NAMEBUF];               //cash balance remaining
	char acctbuf[NAMEBUF];               //total acct value
	memset(valuebuf, 0, NAMEBUF);
	memset(cashbuf, 0, NAMEBUF);
	memset(acctbuf, 0, NAMEBUF);
	long total_value = 0L;
	long curr_value = 0L;
	int secnum = 1;

	struct position *curr = acct->positions;
	if( curr == NULL )
		fprintf(stdout, "              ****** NO POSITIONS IN ACCOUNT *******\n");
	//Iterate over all positions	
	while( curr )
	{
		curr_value = curr->total_shares * get_curr_price(&curr->sec, pricedata, num);
		total_value += curr_value;
		long_to_price_display(curr_value, valuebuf);
		
		fprintf(stdout, "[%3d] %4s: Total Shares: %'6d Current Value: %14s\n", secnum++, curr->sec.sym, curr->total_shares, valuebuf);
		memset(valuebuf, 0, NAMEBUF);
		curr = curr->next;
	}
	long_to_price_display(total_value, valuebuf);
	long_to_price_display(get_cash_balance(acct), cashbuf);
	long_to_price_display(total_value + get_cash_balance(acct), acctbuf);

	fprintf(stdout, "\n                 Total Value of Securities: %18s\n", valuebuf);
	fprintf(stdout, "                    Cash Balance Remaining: %18s\n", cashbuf);
	fprintf(stdout, "                                           -------------------\n");
	fprintf(stdout, "                       Total Account Value: %18s\n", acctbuf);
}


/*
 *   Create a data source object.
 */
struct data *create_data_source(const char *dfname)
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
	//new_data_source->data_t = data_type;
	copy_name(new_data_source->df_filename, fname);
	new_data_source->fp = datafp;

	return new_data_source;
}


long get_moving_avg(const char *secname, int days, const char *d)
{
	long movavg = 0L;
	char buf[IOBUFSIZE];
	memset(buf, 0, IOBUFSIZE);
	char fname[NAMEBUF];
	memset(fname, 0, NAMEBUF);
	char pricebuf[NAMEBUF];
	memset(pricebuf, 0, NAMEBUF);
	strcpy(fname, "../data/");
	strcat(fname, secname);
	char *ext = ".dat";
	strcat(fname, ext);
//	fprintf(stderr, "Mov avg file name: %s\n", fname);
	/* Open data file for reading */
	FILE *fp = fopen(fname, "r");
	if( fp == NULL )
		die("error opening data file");

	char *token_separators = "\t \n";
	char *ticker;
	char *date;
	char *price;
	char *bk;
	
	long sum = 0L;
	int date_found = 0;
	int counter = 0;

	//read in file past the first days
	while( counter < days && fgets(buf, sizeof(buf), fp) != NULL )
	{	
		counter++;	
	}	

//fprintf(stderr, "counter = %d\n", counter);
	counter = 0;
	memset(buf, 0, IOBUFSIZE);
//fprintf(stderr, "days = %d\n", days);
	while( counter < days && fgets(buf, sizeof(buf), fp) != NULL )
	{	
		ticker = strtok_r(buf, token_separators, &bk);
		date   = strtok_r(NULL, token_separators, &bk);
		price  = strtok_r(NULL, token_separators, &bk);
//		fprintf(stderr, "movavg: ticker %s\n", ticker);
//		fprintf(stderr, "movavg: date %s\n", date);
//		fprintf(stderr, "movavg: price %s\n", price);

		if( !date_found )
		{
			if( strcmp(d, date) == 0 )
			{
				sprintf(pricebuf, "%s", price);
				date_found = 1;
				sum += price_to_long(price);
				counter++;
			}
		}
		
		if( date_found )
		{
			sprintf(pricebuf, "%s", price);
			sum += price_to_long(price);
			counter++;	
		}
		memset(pricebuf, 0, NAMEBUF);
		memset(buf, 0, IOBUFSIZE);
	}

	//Check if we reached EOF
	if( feof(fp) > 0 )
	{
		movavg = -1;
	}
	else
	{
		movavg = sum / days;
	}
//fprintf(stderr, "movavg = %ld\n", movavg);
	if(fp)
		fclose(fp);
	return movavg;
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

	if( pthread_cond_init(&(new_algo->algo_stop), NULL) != 0 )
		die("create_algorithm:error initializing cond var");

	if( pthread_cond_init(&(new_algo->algo_go), NULL) != 0 )
		die("create_algorithm:error initializing cond var");

	if( pthread_mutex_init(&(new_algo->mutex), NULL) != 0 ) {
		pthread_cond_destroy(&new_algo->algo_stop);
		pthread_cond_destroy(&new_algo->algo_go);
		die("in process_handler: error initializing mutex");
	}

	new_algo->d = data_src;
	new_algo->num_args = 0;
	new_algo->is_dead = 0;
	new_algo->has_result = 0;
	new_algo->can_run = 0;
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
	printf("[%s] YOU %s: %'d SHARES OF %s AT %s %s\n", buf, order_type, my_order->ord->amt, 
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


char *attr_tostring(int t)
{
	char *str = "";
	switch(t)
	{
		case SEC_T: str = "SEC"; break;
		case AMT_T: str = "AMT"; break;
		case PRC_T: str = "PRC"; break;
		case POS_T: str = "POS"; break;
		case AVAIL_CASH_T: str = "AVAIL_CASH"; break;
		case NEXT_T: str = "NEXT"; break;
		default: str = "UNKNOWN TYPE";
	}

	return str;
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
		case typePos: str = "POSITION"; break;
		case typeCurrencyConst: str = "CURRENCY"; break;
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

/*
 *  Get the current market price of a security from the price database.
 *  Returns the price as a long of the security.
 */
long get_curr_price(struct security *sec, struct position *pricedata, int num)
{
	long price = -1L;
	int i = 0;
	int found = 0;
	while( !found && i < num )
	{
		if( is_equal_sec( &pricedata[i].sec, sec ) )
		{
			price = pricedata[i].total_cost;
			found = 1;
		}
		i++;
	}
	return price;
}


