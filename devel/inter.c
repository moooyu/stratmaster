#include <stdio.h>
#include <time.h>
#include "ast.h"
#include "y.tab.h"
#include "runtime.h"
#include "symtab.h"
#include <stdint.h>

#define MAX_STRATEGIES 8
#define MAX_ALGORITHMS 8
#define MAX_PROCSTATEMENTS 8
#define NUMPRICES 100

struct proc_args {
	char name[NAMEBUF];
	ast_strategy *strat;
	ast_process_statement *procst;
	struct symbol_table *symtable;
};

struct action_list_args {
	char strat_name[NAMEBUF];
	ast_action_list *order_list;
};


struct cleanup_args {
	struct security *n;
	struct security *t;
	FILE *f;
};

/*  Function Declarations  */
void *strategy_process_handler(void *arg);
void *strategy_order_handler(void *arg);
void *order_handler(void *arg);
void *process_handler(void *arg);
void cleanup_algorithm(void *arg);
void *algorithm_handler(void *arg);
void* ex_exp(ast_exp *p);
void* ex_stmt (ast_statement *statement);


/*     Global Variables    */
static struct queue order_queue;
static pthread_t order_handler_thread;
static struct account *ac_master;
static struct data *df1;
static struct cleanup_args ca;
struct symbol_table *global_symtable;
static struct data *db_prices;
static struct position pricedata[NUMPRICES];
char *tk; //ticker
char *dt; //date

GHashTable *algo_map;
int algo_map_init()
{
	algo_map = g_hash_table_new(g_str_hash, g_str_equal);
}

void run_interp(ast_program * program)
{
	PRINTI(("[INFO] Starting Interpreter.\n"));
	/***************************
	 *    INITIALIZATION
	 **************************/ 
	int retval;
	void *result;
	pthread_t strat_thread[MAX_STRATEGIES];
	pthread_t algo_thread[MAX_ALGORITHMS];
	setlocale(LC_NUMERIC, "");

	algo_map_init();

	db_prices = create_data_source("../data/db_prices");

	/* Load prices */
	char *token_separators = "\t \n";
	char *ticker;
	char *price;
	char buf[IOBUFSIZE];
	memset(buf, 0, IOBUFSIZE);
	int index = 0;
	while(  (fgets(buf, sizeof(buf), db_prices->fp) != NULL) && index < NUMPRICES )
	{
		ticker = strtok(buf, token_separators);
		price  = strtok(NULL, token_separators);

		pricedata[index].sec = *create_security(0, ticker);
		pricedata[index].total_cost = price_to_long(price);
		index++;
		memset(buf, 0, IOBUFSIZE);
	}

	char pricebuf[NAMEBUF];
	memset(pricebuf, 0, NAMEBUF);

	/* initialize the order queue */
	order_queue_init(&order_queue);

	/* create the order handler thread */
	if( pthread_create(&order_handler_thread, NULL, order_handler, NULL ) != 0 )
		die("could not create order_handler_thread");

	PRINTI(("[INFO] Initialization complete.\n"));
	/* ************************
	 *        USE-LIST 
	 * *************************/
	/* Create account */
	ac_master = (struct account *)(symbol_table_get_value(program->sym, ACCOUNT_T, "ac_master")->nodePtr);
	//initialize mutex in account
	if( pthread_mutex_init(&(ac_master->lock), NULL) != 0 ) 
	{
		free(ac_master);
		die("initializing ac_master lock");
	}


	//print_account_positions(ac_master);
	/* Create data source */
//	if( (df1 = create_data_source("../data/olddata/df_ZBRA", DATAFEED)) == NULL )
//	  	die("error creating data source");

	/* ************************
	 *    EXECUTE STRATEGIES 
	 * *************************/
	int i;
	/*  Iterate over all STRATEGYs */
	for(i = 0; i < program->num_of_strategies; i++)
	{
		if(program->strategy_list[i]->num_of_process_statement == 0)
		{
			if( pthread_create(&strat_thread[i], NULL, strategy_order_handler, program->strategy_list[i]) != 0 )
				die("could not create strategy_order_handler");
		}
		else
		{
			if( pthread_create(&strat_thread[i], NULL, strategy_process_handler, program->strategy_list[i]) != 0 )
				die("could not create strategy_process_handler");
		}
	}
	
	for(i = 0; i < program->num_of_strategies; i++)
	{
		if( pthread_join(strat_thread[i], NULL) != 0 )
			perror("strategy_handler join");
	}

	/* ************************
	 *  TEAR DOWN THREAD & CLEAN UP 
	 * *************************/
	PRINTI(("[INFO] Cleaning up.\n"));
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
	/* Print account summary before exit */
	print_account_summary(ac_master, "ac_master", pricedata, NUMPRICES);
	pthread_mutex_destroy(&ac_master->lock);
	/* free account */
	if( ac_master != NULL)
		free(ac_master);
	/*if( df1 != NULL )
	  free(df1);*/

	PRINTI(("[INFO] Ending Interpreter.\n"));
}


/*
 *  Handler for STRATEGY with action list only.
 */
void *strategy_order_handler(void *arg)
{
	PRINTI(("[INFO] Starting STRATEGY thread (orders only).\n"));
	ast_strategy * strategy = (ast_strategy *)arg;

	int i;
	ast_order_item *temp_item;
	struct order *temp_order;
	////iterating over orders
	for(i = 0; i < strategy->num_of_orders; i++)
	{
		temp_item = strategy->order_list[i];
		temp_order = create_order(temp_item->sec->sec, temp_item->number->con.int_value->value, temp_item->prc->curr, temp_item->type);
		//queue_put_order(&order_queue, temp_order, strategy->name);

		//Check if we can place the order
		switch( temp_order->order_t )
		{
			case BUY_ORDER:
				if( can_add_position(ac_master, temp_order) < 0 )
					fprintf(stderr, "ERROR: %s: cannot add position\n", temp_order->sec.sym);
				else
				{
					pthread_mutex_lock(&ac_master->lock);
					add_position(ac_master, temp_order);
					pthread_mutex_unlock(&ac_master->lock);
					queue_put_order(&order_queue, temp_order, strategy->name);
				}
				break;

			case SELL_ORDER:
				if( can_sell_position(ac_master, temp_order) < 0 )
					fprintf(stderr, "ERROR: %s: cannot sell position\n", temp_order->sec.sym);
				else
				{
					pthread_mutex_lock(&ac_master->lock);	
					subtract_position(ac_master, temp_order);
					pthread_mutex_unlock(&ac_master->lock);
					queue_put_order(&order_queue, temp_order, strategy->name);
				}
				break;
			default: fprintf(stderr, "ERROR: unknown order type.\n"); break;
		}
	}
	return (void *)0;
}

/*
 *   Handler for STRATEGY with process statements.
 */
void *strategy_process_handler(void *arg)
{
	pthread_t proc_thread[MAX_PROCSTATEMENTS];
	ast_strategy * strategy = (ast_strategy *)arg;
	
	PRINTI(("[INFO] Starting STRATEGY thread (process statements) for %s.\n", strategy->name));
	struct symbol_table *symtable = strategy->sym;

	/* Iterate over all process statements  */
	int i;
	int num_proc_sts;

	num_proc_sts = strategy->num_of_process_statement;
	for(i = 0; i < num_proc_sts; i++)
	{
		struct proc_args *args = (struct proc_args *)malloc(sizeof(struct proc_args));
		if( args == NULL )
			die("malloc failed making proc_args\n");
		args->procst = strategy->process_statement[i];
		args->strat = strategy;
		args->symtable = strategy->sym;
		copy_name(args->name, strategy->name);

		/*  execute process statements */
		if( pthread_create(&proc_thread[i], NULL, process_handler, args) != 0 )
			die("could not create strategy_order_handler");
	}

	for(i = 0; i < num_proc_sts; i++)
	{
		if( pthread_join(proc_thread[i], NULL) != 0 )
			perror("strategy_handler join");
	}

	PRINTI(("[INFO] Exiting STRATEGY process thread\n"));
	return (void *)0;
}

void ex_action_list(ast_action_list *actions, char * strat_name)
{
	ast_order_item *temp_item;
	//int ret = 0;
	int i = 0;
	struct order *temp_order;

	for(i = 0; i < actions->num_of_orders; i++) {
		temp_item = actions->order[i];
		temp_order = create_order(temp_item->sec->sec, temp_item->number->con.int_value->value, temp_item->prc->curr, temp_item->type);

		//Check if we can place the order
		switch( temp_order->order_t )
		{
			case BUY_ORDER:
				if( can_add_position(ac_master, temp_order) < 0 )
					/*fprintf(stderr, "ERROR: cannot add position\n")*/;
				else
				{
					add_position(ac_master, temp_order);
					queue_put_order(&order_queue, temp_order, strat_name);
				}
				break;

			case SELL_ORDER:
				if( can_sell_position(ac_master, temp_order) < 0 )
					/* fprintf(stderr, "ERROR: cannot sell position\n") */;
				else
				{
					subtract_position(ac_master, temp_order);
					queue_put_order(&order_queue, temp_order, strat_name);
				}
				break;
			default: fprintf(stderr, "ERROR: unknown order type.\n"); break;
		}	
	}

}

void terminate_algo_thread(struct algorithm *algo_data)
{
	void *result;
	int retval;

	if (!algo_data)
		return;

	if( algo_data->is_dead )
	{
		PRINTI(("[INFO] ALGORITHM thread has died.\n"));
		retval = pthread_join(algo_data->algo_thread, &result);
		if( retval != 0 )
			perror("thread join");
	}
	else
	{      /* ALGO is not dead; need to cancel */
		PRINTI(("[INFO] Canceling ALGORITHM thread.\n"));
		if( pthread_cancel(algo_data->algo_thread) != 0 )
			perror("thread cancellation");
		retval = pthread_join(algo_data->algo_thread, &result);
		if( retval != 0 )
			perror("thread join");
		if( result != PTHREAD_CANCELED )
			perror("thread was not canceled");
	}

	pthread_mutex_destroy(&algo_data->mutex);
	pthread_cond_destroy(&algo_data->algo_stop);
	pthread_cond_destroy(&algo_data->algo_go);
}

/*
 *  Handler for a process (WHEN) statement.
 */
void *process_handler(void *arg)
{
	PRINTI(("[INFO] Executing process statement.\n"));

	int retval;
	struct proc_args *args = (struct proc_args *)arg;
	char* algo_id;
	struct algorithm *algo_data;

	algo_id = args->procst->expression->oper.op1->oper.op1->id.value;

	do {

		if ( ex_exp(args->procst->expression) ) {
			/* algo_data is created only after the first call to algo */
			algo_data = g_hash_table_lookup(algo_map, algo_id);
			//Check that ALGORITHM has a result for us AND is still alive
			if (algo_data->has_result && !algo_data->is_dead)
			{
				//Set has_result to 0 so we don't keep
				//issuing orders before ALGO thread resumes
				algo_data->has_result = 0;
				pthread_mutex_lock(&ac_master->lock);
				ex_action_list(args->procst->action_list, args->strat->name);
				pthread_mutex_unlock(&ac_master->lock);
				//NOW we release the lock--after issuing the orders
				pthread_mutex_unlock(&algo_data->mutex);
			}
		}

	} while ( !algo_data->is_dead && (args->procst->until_exp != NULL) && !ex_exp(args->procst->until_exp) );
	
	terminate_algo_thread(algo_data);
	free(args);

	return (void *)0;
}


/**********  Thread handlers  **********/
/*
 *  Thread exit cleanup handler.
 *  When we cancel the ALGORITHM thread, this
 *  cleanup code is executed.
 */
void cleanup_algorithm(void *arg)
{
	PRINTI(("[INFO] EXITING ALGORITHM THREAD: CLEANING UP\n"));
	struct cleanup_args cargs = *(struct cleanup_args *)arg;
	if( cargs.n != NULL )
		free(cargs.n);
	if( cargs.t != NULL )
		free(cargs.t);
//	if( cargs.f )
//`		fclose(cargs.f);
}

/*
 *   	ALGORITHM thread handler.
 */
void *algorithm_handler(void *arg)
{
	PRINTI(("[INFO] STARTING ALGORITHM THREAD\n"));
	struct algorithm *algo = (struct algorithm *)arg;

	/*  Retrieve structures from arg */
	int num_args = algo->num_args;
	ast_exp **argument_list = (ast_exp **)algo->args;
	ast_algorithm *algo_node = (ast_algorithm *)algo->algo_ptr;
	struct symbol_table *symt = (struct symbol_table *)algo->sym;
	int num_stmt = algo_node->num_of_statement;
	int num_stmt_in_set;
	ast_set_statement *set_stmt;
	
	/* Link ALGORITHM parameters with STRATEGY arguments */
	int i;
	struct symbol_value *algo_param;  //this is a param in ALGO fcn def
	struct symbol_value *strat_argu;  //this is corresponding arg in STRAT fcn call to ALGO
	char *algo_id; 
	char *strat_id;

	for(i = 0; i < num_args; i++)
	{
		algo_id = algo_node->para_list[i]->name;
		algo_param = symbol_table_get_value(algo_node->sym, 0, algo_id);
                /////check the node type for argument
                if (argument_list[i]->type == typeOper){
                   strat_id = argument_list[i]->oper.op1->id.value;
                }
                else {
                   strat_id = argument_list[i]->id.value;
                }
		
		strat_argu = symbol_table_get_value(symt, 0, strat_id);

		/* Check if types are the same */
		if( algo_param->type_specifier != strat_argu->type_specifier )
		{
			//TODO: we shouldn't die here; need to return error message
			die("parameter type mismatch");
		}

		/* make the assignment to effect the link */
		algo_param->nodePtr = strat_argu->nodePtr;
                if(algo_param->type_specifier == DATAFEED_T){
                    algo->d = algo_param->nodePtr;
                    PRINTI(("datafeed name is %s\n", algo->d->df_filename));
                }
                
	}

	/* Semantically, SET statement is the last one, and we handle that after this for loop*/
	for (i = 0; i < num_stmt-1; i++)
	{
		ex_stmt(algo_node->statement[i]);
	}
	set_stmt = &(algo_node->statement[i]->set_statement);
	num_stmt_in_set = set_stmt->argu_list->argu_list.num_of_argument_expression_list;
	PRINTI(("---------------------> number of stmt in set statement is %d\n", num_stmt_in_set));

	char *token_separators = "\t \n";
	char *ticker;
	char *date;
	char *price;
	char *bk;
        char buf[IOBUFSIZE];
	memset(buf, 0, IOBUFSIZE);

	/* 
	 *   Sleep interval in microseconds.
	 *   This is 1 sec per year of data.
	 */ 
	unsigned int interval = 2760;
	
/*	struct security *next_sec = create_security(EQTY, "TEST");
	struct security *test_sec = create_security(EQTY, "ZBRA");
*/
//	ca.n = next_sec;
//	ca.t = test_sec;
	ca.f = algo->d->fp;
	pthread_cleanup_push(cleanup_algorithm, &ca);
	int found;
	while( 1 )
	{
		//Start of ALGORITHM (producer) sequence
		pthread_mutex_lock(&algo->mutex);
		pthread_testcancel();
		algo->has_result = 0;

		while( !algo->can_run )
			pthread_cond_wait(&algo->algo_go, &algo->mutex);

		//if we get here, ALGORITHM was told to go ahead by STRATEGY
		memset(buf, 0, IOBUFSIZE);
		found = 0;
		while( !found && (fgets(buf, sizeof(buf), algo->d->fp) != NULL) )
		{	
			ticker = strtok_r(buf, token_separators, &bk);
			date   = strtok_r(NULL, token_separators, &bk);
			price  = strtok_r(NULL, token_separators, &bk);
			PRINTI(("ticker is %s\n", ticker));
			PRINTI(("price is %s\n", price));
			PRINTI(("date is %s\n", date));

			tk = ticker;
			dt = date;

			strcpy(algo->d->current_data.eqty, ticker);
			strcpy(algo->d->current_data.date, date);
			strcpy(algo->d->current_data.price, price);
			if (ex_exp(set_stmt->exp)) 
			{
				for (i = 0 ; i < num_stmt_in_set; i++) 
				{
					ex_exp(set_stmt->argu_list->argu_list.exp[i]);
				}

				algo->is_dead = 0;
				algo->has_result = 1;
				pthread_cond_signal(&algo->algo_stop);
				//Set can_run to 0 so ALGO doesn't keep
				//running before STRATEGY can reset this
				algo->can_run = 0;
				pthread_mutex_unlock(&algo->mutex);
				found = 1;
			}

			memset(buf, 0, IOBUFSIZE);
			usleep(interval);
		}

		//Check if we reached EOF
		if( feof(algo->d->fp) > 0 )
		{
			PRINTI(("Reached EOF in ALGORITHM\n"));
			break;
		}
	}

	/*	if( next_sec != NULL )
		free(next_sec);
		if( test_sec != NULL )
		free(test_sec); */

//	if( algo->d->fp )
//		fclose(algo->d->fp);
	//Need to set has_result = 1 since STRATEGY is sitting
	//on a while loop checking this variable
	algo->has_result = 1;
	algo->is_dead = 1;
	//Signal STRATEGY and unlock mutex
	pthread_cond_signal(&algo->algo_stop);
	pthread_mutex_unlock(&algo->mutex);
	pthread_cleanup_pop(0);
	PRINTI(("Returning from ALGORITHM handler: ALGORITHM has died\n"));
	return (void *)0;
}


void *order_handler(void *arg)
{
	struct order_item *next_order;

	while(1)
	{
		/* Wait for order */
		next_order = queue_get_order(&order_queue);
		PRINTI(("[INFO] ISSUING ORDER.\n"));
		emit_order(next_order);

		/* free order_item structures */
//		if( next_order->ord != NULL )
//`			free(next_order->ord);
//		if( next_order != NULL )
//			free(next_order);
	
	}
	return (void *)0;
}


/* TODO */
void* ex_stmt (ast_statement *statement)
{
	return NULL;
}

/* Caller is thread which handles WHEN statement*/
int call_algo(ast_exp *p)
{
	int ret = 1; /* Algorithm returns 1 always */
	struct symbol_value* sym_entry;
	struct algorithm *algo_data;
	char* algo_id = p->oper.op1->id.value;
	algo_data = g_hash_table_lookup(algo_map, algo_id);
	if (!algo_data) {
		algo_data = create_algorithm(df1);

		/* Get argument expr list */
		ast_argument_expression_list algo_args = p->oper.op2->argu_list;

		/* Set number of args , argument list pointer, & pointer to ALGORITHM AST node */
		algo_data->num_args = algo_args.num_of_argument_expression_list;
		algo_data->args     = algo_args.exp;
		sym_entry = (struct symbol_value*)ex_exp(p->oper.op1);
		algo_data->algo_ptr = sym_entry->nodePtr;
		algo_data->sym = p->oper.op1->id.sym;

		if( pthread_create(&algo_data->algo_thread, NULL, algorithm_handler, algo_data) != 0 )
			die("algorithm thread create fail");
		
		g_hash_table_insert(algo_map, (gpointer) algo_id, algo_data);
	}

	pthread_mutex_lock(&algo_data->mutex);
	//Let ALGORITHM know it's OK to run
	algo_data->can_run = 1;
	pthread_cond_signal(&algo_data->algo_go);
	while( !algo_data->has_result )
		pthread_cond_wait(&algo_data->algo_stop, &algo_data->mutex);

	//if we get here, ALGORITHM has signaled TRUE
	algo_data->can_run = 0;

	//But check if ALGORITHM thread died; if alive
	//we need to unlock mutex
//	if( !algo_data->is_dead )
//		pthread_mutex_unlock(&algo_data->mutex);

	
	return ret;
}

/* return 1 if this exp is algo.
 * return 0 otherwise
 */
int is_algo(ast_exp *p)
{
	if (p->type == typeOper) {
		if (p->oper.oper == OP_FUNC) {
			int type;
			struct symbol_value* sym_entry;
			sym_entry = (struct symbol_value*)ex_exp(p->oper.op1);
			type = sym_entry->type_specifier;
			if (type == ALGORITHM_T)
				return 1;
		}
	}
	return 0;
}

void* ex_exp(ast_exp *p)
{
	void* ret = NULL;
	struct symbol_value* sym_entry;
	char buf[NAMEBUF];
	memset(buf, 0, NAMEBUF);
	int days = 0;
	if (!p) return 0;
	switch(p->type)
	{
		case typeID:
			PRINTI(("--------------------------> This exp is ID\n"));
			ret = (void*)symbol_table_get_value(p->id.sym, 0, p->id.value);
			return ret;
			break;
		case typeIntegerConst:
			PRINTI(("--------------------------> This exp is IntegerConst: %d\n", p->con.int_value->value));
			ret = (void*)(intptr_t)p->con.int_value->value;
			break;
		case typeBooleanConst:
			PRINTI(("--------------------------> This exp is BooleanConst: %d\n", p->con.bool_value->value));
			ret = (void*)(intptr_t)p->con.bool_value->value;
			break;
		case typePriceConst:
			PRINTI(("--------------------------> This exp is PriceConst: %s\n", p->con.price_value->price ));
			ret = (void*)p->con.price_value->price;
			break;
	/*	case typeCurrencyConst:
			PRINTI(("--------------------------> This exp is CurrencyConst: %s\n", p->con.curr_value->p ));
			ret = (void*)p->con.curr_value->p;
			break;  */
		case typeKeyword:
			PRINTI(("--------------------------> This exp is KEYword\n"));
			break;
                case typeSec:
                        PRINTI(("--------------------------> This exp is Security: %s\n", p->security.sec->sym));
                        ret = (void*)p->security.sec->sym;
                        break;
	/*	case typePos:
                        PRINTI(("--------------------------> This exp is Position: %s\n", p->position.pos->sec.sym));
                        ret = (void*)p->position.pos->sec.sym;
                        break; */
		case typeOper:
			switch(p->oper.oper) {

				case OP_AND:
					PRINTI(("--------------------------> Operator AND\n"));
					if (ex_exp(p->oper.op1) && (ex_exp(p->oper.op2))) {
						ret = (void*)(intptr_t)1;
						break;
					}
					ret = (void*)(intptr_t)0;
					break;

				case OP_LT:
					PRINTI(("--------------------------> Operator <\n"));
					/* TODO: type check */

					if (price_to_long((char*)ex_exp(p->oper.op1))< price_to_long((char*)ex_exp(p->oper.op2)))
						ret = (void*)(intptr_t)1;
					else
						ret = (void*)(intptr_t)0;

					break;

				case OP_GT:
					PRINTI(("--------------------------> Operator >\n"));
					/* TODO: type check */
					if( p->oper.op2->type == typeIntegerConst )
					{
						return (void*)(intptr_t) ( ex_exp(p->oper.op1) > ex_exp(p->oper.op2) ) ;
					}
					else if (price_to_long((char*)ex_exp(p->oper.op1))> price_to_long((char*)ex_exp(p->oper.op2)))
						ret = (void*)(intptr_t)1;
					else
						ret = (void*)(intptr_t)0;

					break;


				case OP_ASSIGN:
					PRINTI(("--------------------------> Operator =\n"));
					struct symbol_value* symval;
					symval = (struct symbol_value*)ex_exp(p->oper.op1);
					copy_name(((ast_currency*)symval->nodePtr)->curr->p, (char*)ex_exp(p->oper.op2));
					break;

				case OP_IS:
					PRINTI(("--------------------------> Operator IS\n"));

					if(is_algo(p->oper.op1)) {
						ret = ex_exp(p->oper.op1);
						PRINTI(("--------------------------> Left child of IS is FUNC ALGO Operator\n"));
						break;
					}

                                        if (strcmp((char*)ex_exp(p->oper.op1), (char*)ex_exp(p->oper.op2)) == 0){
                                             ret = (void*)(intptr_t)1;
                                         }
					else {
                                             ret = (void*)(intptr_t)0;
                                         }

					break;

				case OP_FUNC:
					PRINTI(("--------------------------> Operator FUNC\n"));
					int type;
					sym_entry = (struct symbol_value*)ex_exp(p->oper.op1);
					type = sym_entry->type_specifier;
					if (type == ALGORITHM_T) {
						PRINTI(("--------------------------> This is ALGO call to %s\n", sym_entry->identifier));
						ret = (void*)(intptr_t)call_algo(p);
					}
					if( type == FUNC_SYM )
					{
						PRINTI(("--------------------------> This is FUNCTION call to %s\n", sym_entry->identifier));
						if( strcmp( "Get_Mov_Avg", sym_entry->identifier) == 0 )
						{
							days = (intptr_t)p->oper.op2->argu_list.exp[1]->con.int_value->value;
							long_to_price(get_moving_avg(tk, days, dt), buf);
							ret = (void*)buf;
							if( ret < 0 )
								fprintf(stderr, "UNABLE TO GET MOVING AVGS\n");
						}
						else
							ret = (void*)0;
					}				
					
					break;

				case OP_ATTR:

					/*
					   1. look at the op2
					   - if op2 is NEXT, then return op1
					   - if op2 is PRC, then return value in ex_exp(op1)->price
					   - if op2 is SEC, then return value in ex_exp(op1)->eqty
					   - if op2 is POS, then call get_position
					   - if op2 is AMT, then get total_shares from position
					   */
					switch(p->oper.op2->attr.value){
						case PRC_T:
							PRINTI(("--------------------------> PRC_T\n"));
							sym_entry = (struct symbol_value*)ex_exp(p->oper.op1);
							ret = (void*)((struct data*)sym_entry->nodePtr)->current_data.price;
							PRINTI(("------------------------------> PRC will be %s\n", (char*) ret));
							break;
						case AVAIL_CASH_T:
							
							sym_entry = (struct symbol_value*)ex_exp(p->oper.op1);
							ret = (void*)((struct account*)sym_entry->nodePtr)->avail_cash.p;
							PRINTI(("--------------------------> AVAIL_CASH_T %s\n", (char*)ret));
							break;

						case NEXT_T:
							ret = (void*)ex_exp(p->oper.op1);
							break;
						case SEC_T:
							sym_entry = (struct symbol_value*)ex_exp(p->oper.op1);
							ret = (void*)((struct data*)sym_entry->nodePtr)->current_data.eqty;
							PRINTI(("------------------------------> SEC will be %s\n", (char*) ret));
							break;
						case POS_T:
							pthread_mutex_lock(&ac_master->lock);
				//			fprintf(stderr,"in POS_T: %s\n",  (p->oper.op2->position).pos->sec.sym);
				//			ret = (void*)(struct position *)get_position(ac_master, &(p->oper.op2->position).pos); 
							pthread_mutex_unlock(&ac_master->lock);
				//			PRINTI(("------------------------------> POS will be %s\n", (char*)((struct position *)ret)->sec.sym ));
							break;

						case AMT_T:
							//fprintf(stderr, "the type of op1= %s op2 = %s\n",node_type_tostring(p->oper->type));
							//ret = (void*)(intptr_t)p->oper.op1->position.pos->total_shares;
							break;
						default:
							PRINTI(("---------------------------> ATTR operator is not recognized\n"));
					}
					PRINTI(("--------------------------> Operator ATTR: %s\n",  attr_tostring(p->oper.op2->attr.value)));
					break;

				default:
					PRINTI(("---------------------------> Operator is not recognized: %d\n", p->oper.oper));
			}
		break;

		default:
			PRINTI(("--------------------> Expression type is not recognized: %d\n", p->type));

	}
	return ret;
}
