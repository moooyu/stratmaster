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


struct proc_args {
	char name[NAMEBUF];
	ast_strategy *strat;
	ast_process_statement *procst;
	struct symbol_table *symtable;
};

struct action_list_args {
	char *strat_name;
	ast_action_list *actions;
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
/*     Global Variables    */
static struct queue order_queue;
static pthread_t order_handler_thread;
static struct account *ac_master;
static struct data *df1;
static struct cleanup_args ca;


void print_account_positions(struct account *acct)
{
	int i;
	int num = acct->num_positions;
	fprintf(stderr, "[ACCT-INFO] ac_master: Num. of positions: %d\n", acct->num_positions);
	for(i = 0; i < num; i++)
	{
		fprintf(stderr, "[ACCT-INFO] %s: Num. of shares: %d Cost: %s\n", acct->positions[i].sec.sym,
				acct->positions[i].amt, acct->positions[i].purch_price.p);
	}
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
	// ac_master = create_account(); 
	ac_master = (struct account *)(symbol_table_get_value(program->sym, ACCOUNT_T, "ac_master")->nodePtr);

	//print_account_positions(ac_master);
	/* Create data source */
	if( (df1 = create_data_source("../data/olddata/df_ZBRA", DATAFEED)) == NULL )
	  	die("error creating data source");

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
		queue_put_order(&order_queue, temp_order, strategy->name);

	//	fprintf(stderr, "[INFO] Starting balance in ac_master: %ld\n", get_available_cash(ac_master)); 
	//	print_account_positions(ac_master);
	}
	return (void *)0;
}


/*
 *  Handler for STRATEGY action list.
 */
void *strategy_action_list_handler(void *arg)
{
	PRINTI(("[INFO] Starting STRATEGY action list handler thread.\n"));
	struct action_list_args *args = (struct action_list_args *) arg;
	ast_action_list *actions = args->actions;

	int i;
	ast_order_item *temp_item;
	struct order *temp_order;
	////iterating over orders
	for(i = 0; i < actions->num_of_orders; i++)
	{
		temp_item = actions->order[i];
PRINTI(("before create order: %s \n", temp_item->sec->sec->sym));
PRINTI(("before create order: %d\n",  temp_item->number->con.int_value->value));
PRINTI(("before create order: %s\n",  temp_item->prc->curr->p));


		temp_order = create_order(temp_item->sec->sec, temp_item->number->con.int_value->value, temp_item->prc->curr, temp_item->type);
		
		PRINTI(("after create order\n"));	
		queue_put_order(&order_queue, temp_order, args->strat_name);

	//	fprintf(stderr, "[INFO] Starting balance in ac_master: %ld\n", get_available_cash(ac_master)); 
	//	print_account_positions(ac_master);
	}
	free(args);
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


	/* Check if value was set */


	PRINTI(("[INFO] Return from process statement: Retrieving argument...\n"));
	struct symbol_value *param_val = symbol_table_get_value(symtable, 0, "zbra_price");
	PRINTI(("[INFO] Parameter %s is now set to: %s\n", param_val->identifier , ((struct currency *)param_val->nodePtr)->p ));	
	PRINTI(("[INFO] Exiting STRATEGY process thread\n"));
	return (void *)0;
}


/*
 *  Handler for a process (WHEN) statement.
 */
void *process_handler(void *arg)
{
	PRINTI(("[INFO] Executing process statement.\n"));
	void *result;
	int retval;
	int counter = 3;	
	pthread_t algo_thread;
	pthread_t order_thread;

	/* Retrieve args: STRATEGY + symbol table + argument list */
	struct proc_args *args = (struct proc_args *)arg;
	struct symbol_table *symtable = args->symtable;	
	ast_strategy *strat = args->strat;
	ast_process_statement *proc_st = args->procst;
	
	/* We assume that we only call Algorithm at this point */

	struct algorithm *algo_data;
	algo_data = (struct algorithm*) ex_exp(args->procst->expression);
	algo_data->sym = symtable; /* This is dirty, but no way to get symtable in the ex_exp */

	/* Initialize lock & condition variable */
	if( pthread_cond_init(&(algo_data->cond_true), NULL) != 0 )
		die("in process_handler:error initializing cond var");

	if( pthread_mutex_init(&(algo_data->mutex), NULL) != 0 )
	{
		pthread_cond_destroy(&algo_data->cond_true);
		die("in process_handler: error initializing mutex");
	}

	/* Spin up algorithm thread */
	if( pthread_create(&algo_thread, NULL, algorithm_handler, algo_data) != 0 )
		die("algorithm thread create fail");

	/*****************************************************
	 *                UNTIL STATEMENT
	 *****************************************************/
	do
	{	/************************************** 
		 *       Execute WHEN statement 
		 **************************************/
		pthread_mutex_lock(&algo_data->mutex);
		pthread_cond_wait(&algo_data->cond_true, &algo_data->mutex);

		/*******************************
		 *  STRATEGY BLOCK: action-list
		 ******************************/
		/* 
		 * If we get here, the ALGORITHM has sent a "true" signal 
		 * First check if algorithm is still alive
		 */
		if( algo_data->is_dead )
			break;

		/* Release lock */
		pthread_mutex_unlock(&algo_data->mutex);

		/* Send orders to action-list thread */
		struct action_list_args *args = (struct action_list_args *)malloc(sizeof(struct action_list_args));
		if( args == NULL )
			die("malloc failed making action_list_args\n");
		args->strat_name = strat->name;
		args->actions = proc_st->action_list;

		/* Looking at action list */
		PRINTI(("Action list bf order: %s\n", proc_st->action_list->order[0]->sec->sec->sym)); 
		PRINTI(("Action list bf order: %d\n", proc_st->action_list->order[0]->number->con.int_value->value));

		if(proc_st->action_list->order[0]->prc->curr->p == NULL)
			fprintf(stderr, "CURR object is null\n");	

		PRINTI(("Action list bf order: %s\n", proc_st->action_list->order[0]->prc->curr->p));

		struct symbol_value *param_val = symbol_table_get_value(symtable, 0, "zbra_price");
		PRINTI(("[INFO] Parameter %s is now set to: %s\n", param_val->identifier , ((struct currency *)param_val->nodePtr)->p ));
		
		
		if( pthread_create(&order_thread, NULL, strategy_action_list_handler, args) != 0 )
			die("action-list thread create fail");
		if( pthread_join(order_thread, NULL) != 0 )
			perror("action_list join");
	} while( counter-- > 0 );

	if( algo_data->is_dead )
	{	/* Release lock */
		pthread_mutex_unlock(&algo_data->mutex);
	}

	if( pthread_join(algo_thread, NULL) != 0 )
		perror("algorithm thread join");


	if( pthread_cancel(order_handler_thread) != 0 )
		perror("order_handler_thread cancellation");

	retval = pthread_join(order_handler_thread, &result);
	if( retval != 0 )
		perror("order_handler_thread join");

	if( result != PTHREAD_CANCELED )
		perror("order_handler_thread was not canceled");
	pthread_mutex_destroy(&algo_data->mutex);
	pthread_cond_destroy(&algo_data->cond_true);

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
	if( cargs.f )
		fclose(cargs.f);
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
			die("paramater type mismatch");

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
	unsigned int interval = 2740;
	
/*	struct security *next_sec = create_security(EQTY, "TEST");
	struct security *test_sec = create_security(EQTY, "ZBRA");
	ca.n = next_sec;
	ca.t = test_sec;
	ca.f = algo->d->fp; */
	pthread_cleanup_push(cleanup_algorithm, &ca);

	int keep_running = 10;

	while( keep_running )
	{
		if( fgets(buf, sizeof(buf), algo->d->fp) != NULL )
		{	
			ticker = strtok_r(buf, token_separators, &bk);
			date   = strtok_r(NULL, token_separators, &bk);
			price  = strtok_r(NULL, token_separators, &bk);
                        PRINTI(("ticker is %s\n", ticker));
                        PRINTI(("price is %s\n", price));
                        PRINTI(("date is %s\n", date));

		//	copy_name(next_sec->sym, ticker);
		//	long pr = price_to_long(price);

			/* DO TEST */
		//	if( (is_equal_sec(next_sec, test_sec) == TRUE) && (pr < 2700) )
		//	{
		//		keep_running = FALSE;
		//		fprintf(stderr, "[INFO] ALGO FOUND A PRICE TARGET: $%s\n", price);
				/* Set argument value to new price for return to STRATEGY */
		//		copy_name(((struct price *)algo->args)->p, price);
			strcpy(algo->d->current_data.eqty, ticker);
			strcpy(algo->d->current_data.date, date);
			strcpy(algo->d->current_data.price, price);
	              if (ex_exp(set_stmt->exp)) {
		           for (i = 0 ; i < num_stmt_in_set; i++) {
			        ex_exp(set_stmt->argu_list->argu_list.exp[i]);
		           }
                           sleep(3);
				
				pthread_cond_signal(&algo->cond_true);

				pthread_mutex_lock(&algo->mutex);

				pthread_testcancel();
				
				pthread_cond_wait(&algo->cond_true, &algo->mutex);
				
				
				pthread_mutex_unlock(&algo->mutex);
	               }
		


			//}
			memset(buf, 0, IOBUFSIZE);
			usleep(interval);
	//		keep_running--;
		}
	//	else
	//	{
	//		keep_running = FALSE;
	//	}
	}
	algo->is_dead = 1;
/*	if( next_sec != NULL )
		free(next_sec);
	if( test_sec != NULL )
		free(test_sec); */

	if( algo->d->fp )
		fclose(algo->d->fp);
	pthread_cond_signal(&algo->cond_true);
	pthread_cleanup_pop(0);
	return (void *)0;
}


void *order_handler(void *arg)
{
	struct order_item *next_order;

	while(1)
	{
		/* Wait for order */
		next_order = queue_get_order(&order_queue);
	//	long res = can_add_position(ac_master, next_order->ord);
	//	if( res >= 0 )
	//	{
			/* Issue the order */
			fprintf(stderr, "[INFO] ISSUING ORDER.\n");
			emit_order(next_order);
			/* add the position to the account */
	/*		long res = add_position(ac_master, next_order->ord);
			if( res < 0 )
				fprintf(stderr, "ERROR: negative cash balance: order should not have gone through\n");
			fprintf(stderr, "[INFO] Account balance after order: %ld\n", get_available_cash(ac_master));
		}
		else
		{
			fprintf(stderr, "ERROR: not enough available cash--order rejected\n");
		}


		print_account_positions(ac_master);
	*/			
		/* free order_item structures */
		if( next_order->ord != NULL )
			free(next_order->ord);
		if( next_order != NULL )
			free(next_order);
	}
	return (void *)0;
}


/* TODO */
void* ex_stmt (ast_statement *statement)
{
	return NULL;
}

void* ex_exp(ast_exp *p)
{
	void* ret = NULL;
	struct symbol_value* sym_entry;
	if (!p) return 0;
	switch(p->type)
	{
		case typeID:
			PRINTI(("--------------------------> This exp is ID\n"));
			ret = (void*)symbol_table_get_value(p->id.sym, 0, p->id.value);
			return ret;
			
			break;
		case typeBooleanConst:
			PRINTI(("--------------------------> This exp is BooleanConst: %d\n", p->con.bool_value->value));
			ret = (void*)(intptr_t)p->con.bool_value->value;
			break;
		case typePriceConst:
			PRINTI(("--------------------------> This exp is PriceConst: %s\n", p->con.price_value->price ));
			ret = (void*)p->con.price_value->price;
			break;
		case typeKeyword:
			PRINTI(("--------------------------> This exp is KEYword\n"));
			break;

		case typeOper:
			switch(p->oper.oper) {

				case OP_AND:
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

				case OP_ASSIGN:
					PRINTI(("--------------------------> Operator =\n"));
					struct symbol_value* symval;
					symval = (struct symbol_value*)ex_exp(p->oper.op1);
					copy_name(((ast_currency*)symval->nodePtr)->curr->p, (char*)ex_exp(p->oper.op2));
					break;

				case OP_IS:
					PRINTI(("--------------------------> Operator IS\n"));

					/* If IS is used for algo call, we do not compare op1 and op2 */
					if (p->oper.op1->type == typeOper) {
						if (p->oper.op1->oper.oper == OP_FUNC) {
							int type;
							sym_entry = (struct symbol_value*)ex_exp(p->oper.op1->oper.op1);
							type = sym_entry->type_specifier;
							if (type == ALGORITHM_T) {
								ret = ex_exp(p->oper.op1);
								PRINTI(("--------------------------> Right child of IS is FUNC ALGO Operator\n"));
								break;
							}
						}
					}

					/* TODO: Do this once EQTY(ZBRA) is done in AST
					 * Until the, we just return true */
					ex_exp(p->oper.op1);
					ret = (void*)(intptr_t)1;

					break;
				case OP_FUNC:
					PRINTI(("--------------------------> Operator FUNC\n"));
					int type;
					sym_entry = (struct symbol_value*)ex_exp(p->oper.op1);
					type = sym_entry->type_specifier;
					if (type == ALGORITHM_T)
						PRINTI(("--------------------------> This is ALGO call to %s\n", sym_entry->identifier));
					else
						PRINTI(("--------------------------> This is Function call???\n"));

					/* Create algorithm data structure */
					struct algorithm *algo_data = create_algorithm(df1);

					/* Get argument expr list */
					ast_argument_expression_list algo_args = p->oper.op2->argu_list;
					PRINTI(("[INFO] Number of args: %d.\n", algo_args.num_of_argument_expression_list));


					PRINTI(("[INFO] arg1 is : %d.\n", algo_args.exp[0]->type));
					///fprintf(stderr, "[INFO] arg1 is : %d.\n", algo_args.exp[0]->oper.oper);
					//ast_exp *node = algo_args.exp[0]->oper.op1;
					//fprintf(stderr, "[INFO] node type is : %d.\n", node->type);
					//fprintf(stderr, "[INFO] node type name is : %s.\n", node->id.value);

					/* Set number of args , argument list pointer, & pointer to ALGORITHM AST node */
					algo_data->num_args = algo_args.num_of_argument_expression_list;
					algo_data->args     = algo_args.exp;
					algo_data->algo_ptr = sym_entry->nodePtr;

					ret = (void*)algo_data;
					break;
				case OP_ATTR:

					/*
					   1. look at the op2
					   - if op2 is NEXT, then return op1
					   - if op2 is PRC, then return value in ex_exp(op1)->price
					   - if op2 is SEC, then return value in ex_exp(op1)->eqty
					   */
					switch(p->oper.op2->attr.value){
						case PRC_T:
							PRINTI(("--------------------------> PRC_T\n"));
							sym_entry = (struct symbol_value*)ex_exp(p->oper.op1);
							ret = (void*)((struct data*)sym_entry->nodePtr)->current_data.price;
							PRINTI(("------------------------------> PRC will be %s\n", (char*) ret));
							break;
						case NEXT_T:
							ret = (void*)ex_exp(p->oper.op1);
							break;
						case SEC_T:
							sym_entry = (struct symbol_value*)ex_exp(p->oper.op1);
							ret = (void*)((struct data*)sym_entry->nodePtr)->current_data.eqty;
							PRINTI(("------------------------------> SEC will be %s\n", (char*) ret));
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
