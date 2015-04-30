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
	fprintf(stderr, "[INFO] Starting Interpreter.\n");
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

	fprintf(stderr, "[INFO] Initialization complete.\n");
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

	fprintf(stderr, "[INFO] Ending Interpreter.\n");
}


/*
 *  Handler for STRATEGY with action list only.
 */
void *strategy_order_handler(void *arg)
{
	fprintf(stderr, "[INFO] Starting STRATEGY thread (orders only).\n");
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
	fprintf(stderr, "[INFO] Starting STRATEGY action list handler thread.\n");
	struct action_list_args *args = (struct action_list_args *) arg;
	ast_action_list *actions = args->actions;

	int i;
	ast_order_item *temp_item;
	struct order *temp_order;
	////iterating over orders
	for(i = 0; i < actions->num_of_orders; i++)
	{
		temp_item = actions->order[i];
fprintf(stderr, "before create order: %s \n", temp_item->sec->sec->sym);
fprintf(stderr, "before create order: %d\n",  temp_item->number->con.int_value->value);
fprintf(stderr, "before create order: %s\n",  temp_item->prc->curr->p);


		temp_order = create_order(temp_item->sec->sec, temp_item->number->con.int_value->value, temp_item->prc->curr, temp_item->type);
		
		fprintf(stderr, "after create order\n");	
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
	
	fprintf(stderr, "[INFO] Starting STRATEGY thread (process statements) for %s.\n", strategy->name);
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


	fprintf(stderr, "[INFO] Return from process statement: Retrieving argument...\n");
	struct symbol_value *param_val = symbol_table_get_value(symtable, 0, "zbra_price");
	fprintf(stderr, "[INFO] Parameter %s is now set to: %s\n", param_val->identifier , ((struct currency *)param_val->nodePtr)->p );	
	fprintf(stderr, "[INFO] Exiting STRATEGY process thread\n");
	return (void *)0;
}


/*
 *  Handler for a process (WHEN) statement.
 */
void *process_handler(void *arg)
{
	fprintf(stderr, "[INFO] Executing process statement.\n");
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
		fprintf(stderr, "Action list bf order: %s\n", proc_st->action_list->order[0]->sec->sec->sym); 
		fprintf(stderr, "Action list bf order: %d\n", proc_st->action_list->order[0]->number->con.int_value->value);

		if(proc_st->action_list->order[0]->prc->curr->p == NULL)
			fprintf(stderr, "CURR object is null\n");	

		fprintf(stderr, "Action list bf order: %s\n", proc_st->action_list->order[0]->prc->curr->p);

		struct symbol_value *param_val = symbol_table_get_value(symtable, 0, "zbra_price");
		fprintf(stderr, "[INFO] Parameter %s is now set to: %s\n", param_val->identifier , ((struct currency *)param_val->nodePtr)->p );
		
		
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




/*
	ast_exp *when_exp = proc_st->expression;
	fprintf(stderr, "[INFO] Node type: %s\n", node_type_tostring(when_exp->type));
	ast_oper oper_node = when_exp->oper;
	fprintf(stderr, "[INFO] Operator type: %s Num of ops: %d\n", 
			oper_type_tostring(oper_node.oper), oper_node.nops);
	ast_exp *op1 = oper_node.op1;
	ast_exp *op2 = oper_node.op2;
	fprintf(stderr, "[INFO] Op1 Node type: %s\n", node_type_tostring(op1->type));
	fprintf(stderr, "[INFO] Op2 Node type: %s\n", node_type_tostring(op2->type));

	ast_oper oper1 = op1->oper;
	ast_const oper2 = op2->con;
	fprintf(stderr, "[INFO] Operator 1 type: %s Num of ops: %d\n", 
			oper_type_tostring(oper1.oper), oper1.nops);
	fprintf(stderr, "[INFO] Operator 2 value: %d\n", oper2.bool_value->value);

	ast_str id = oper1.op1->id;
	fprintf(stderr, "[INFO] Algorithm id: %s\n", id.value);

	ast_exp *algo_arglist = oper1.op1;
	fprintf(stderr, "[INFO] Algo function op1 type: %s\n", node_type_tostring(algo_arglist->type));
	fprintf(stderr, "[INFO] Algo function op2 type: %s\n", node_type_tostring(oper1.op2->type));
	ast_argument_expression_list alg_params = oper1.op2->argu_list;
	fprintf(stderr, "[INFO] Num of arguments in list=%d\n", alg_params.num_of_argument_expression_list);
	ast_exp *param = alg_params.exp[0];
	fprintf(stderr, "[INFO] Param node type: %s\n", node_type_tostring(param->type));
	ast_oper param_oper = param->oper;
	fprintf(stderr, "[INFO] Param operator type: %s Num of ops: %d\n", 
			oper_type_tostring(param_oper.oper), param_oper.nops);
	ast_exp *usharpop = param_oper.op1;
	fprintf(stderr, "[INFO] Param1 node type is %s\n", node_type_tostring(usharpop->type));
	ast_str param_id = usharpop->id;
	fprintf(stderr, "[INFO] Param1 is %s\n", param_id.value);

	struct symbol_value *param1_val = symbol_table_get_value(symtable, 0, param_id.value);
	fprintf(stderr, "[INFO] Param 1 from symbol table is %s of type %s\n", param1_val->identifier, type_tostring(param1_val->type_specifier));


	struct symbol_value *symval = symbol_table_get_value(symtable, ALGO_SYM, id.value);
	fprintf(stderr, "[INFO] From symbol table: %s is type %s\n", symval->identifier, type_tostring(symval->type_specifier));
	
	ast_algorithm *al = (ast_algorithm *)symval->nodePtr;
	fprintf(stderr, "[INFO] Retrieving algorithm node: %s num params=%d num statements = %d\n",
		       al->name, al->num_of_para, al->num_of_statement);	
	
	fprintf(stderr, "[INFO] Statement type: %s\n", statement_type_tostring(al->statement[0]->type));	
	ast_set_statement st = al->statement[0]->set_statement;

	ast_exp *argument_list = st.argu_list;
	fprintf(stderr, "[INFO] Argulist type: %s Num of arguments=%d\n", node_type_tostring(argument_list->type),
			argument_list->argu_list.num_of_argument_expression_list);
	ast_exp *arg1 = argument_list->argu_list.exp[0];
	fprintf(stderr, "[INFO] Argument list node type: %s\n", node_type_tostring(arg1->type));

	ast_oper oper3 = arg1->oper;
	fprintf(stderr, "[INFO] Operator type: %s Num of ops: %d\n", 
			oper_type_tostring(oper3.oper), oper3.nops);
	op1 = oper3.op1;
	op2 = oper3.op2;
	fprintf(stderr, "[INFO] Op1 Node type: %s\n", node_type_tostring(op1->type));
	fprintf(stderr, "[INFO] Op2 Node type: %s\n", node_type_tostring(op2->type));
	ast_str set_tgt = op1->id;
	fprintf(stderr, "[INFO] Variable to set: %s\n", set_tgt.value);
        struct symbol_value *tgt_sym = symbol_table_get_value(al->sym, 0, set_tgt.value);
	fprintf(stderr, "[INFO] Have symbol table entry for %s of type %s\n", tgt_sym->identifier, type_tostring(tgt_sym->type_specifier));
*/
	/* Link up variables */
/*	tgt_sym->nodePtr = param1_val->nodePtr;
	fprintf(stderr, "[INFO] Value to set is %s\n", op2->con.price_value->price);
	copy_name( ((struct currency *)tgt_sym->nodePtr)->p, op2->con.price_value->price);
*/	
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
	fprintf(stderr, "[INFO] EXITING ALGORITHM THREAD: CLEANING UP\n");
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
	fprintf(stderr, "[INFO] STARTING ALGORITHM THREAD\n");
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
                    printf("datafeed name is %s\n", algo->d->df_filename);
                }
                
	}

	/* To demonstrate, set local ALGO variables and return to STRATEGY */
	/*
	for(i = 0; i < num_args; i++)
	{
		int type =  argument_list[i]->oper.op1->type;

		switch( type )
		{
			case typeIntegerConst: 
				fprintf(stderr, "Type is: %s\n", node_type_tostring( type ));
				break;
			case typeDoubleConst:
				fprintf(stderr, "Type is: %s\n", node_type_tostring( type ));
				break;
			case typePriceConst:
				algo_param = symbol_table_get_value(algo_node->sym, 0, algo_node->statement[0]->set_statement.argu_list->argu_list.exp[i]->oper.op1->id.value); 
				copy_name( ((struct currency *)algo_param->nodePtr)->p, "22.33");
				fprintf(stderr, "Type is: %s\n", node_type_tostring( type ));	
				break;
			case typeID:
				algo_param = symbol_table_get_value(algo_node->sym, 0, algo_node->statement[0]->set_statement.argu_list->argu_list.exp[i]->oper.op1->id.value); 
				copy_name( ((ast_currency *)algo_param->nodePtr)->curr->p, "22.33");
				fprintf(stderr, "Type is: %s Id: %s  Value: %s \n", node_type_tostring( type ), algo_param->identifier, ((ast_currency *)algo_param->nodePtr)->curr->p); 
				break;
			case typeKeyword:
				fprintf(stderr, "Type is: %s\n", node_type_tostring( type )); 
				break;
			case typeArgulist:
				fprintf(stderr, "Type is: %s\n", node_type_tostring( type )); 
				break;
			default: ;
		}

	}
	*/

	/* Semantically, SET statement is the last one, and we handle that after this for loop*/
	for (i = 0; i < num_stmt-1; i++)
	{
		ex_stmt(algo_node->statement[i]);
	}
	set_stmt = &(algo_node->statement[i]->set_statement);
	num_stmt_in_set = set_stmt->argu_list->argu_list.num_of_argument_expression_list;
	printf("---------------------> number of stmt in set statement is %d\n", num_stmt_in_set);




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
                        printf("ticker is %s\n", ticker);
                        printf("price is %s\n", price);
                        printf("date is %s\n", date);

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

//////called in ex_strategy
/*
void *strategy_handler(void *arg)
{
    fprintf(stderr, "[INFO] Starting STRATEGY thread.\n");
    ast_strategy * strategy = (ast_strategy *)arg;
    ////iterating over orders
    if (strategy->num_of_orders > 0)
    {
        int tmp = 0;
        while (tmp < strategy->num_of_orders)
        {
            /////execute order
       //     ex_order_item(strategy->order_list[tmp]);
            tmp += 1;
        }
    }
    else
    {
        int tmp = 0;
        while (tmp < strategy->num_of_process_statement)
        {
            /////execute order
            ex_process_statement(strategy->process_statement[tmp]);
            tmp += 1;
        }
    }
}

void ex_order_item(ast_order_item * order_item)
{
     char *sym = order_item->security_name;
     int type  = order_item->security_type;
     struct security *sec = create_security(sym, type);
     ////////
     int amt = order_item->number;
     char *pr = order_item->price;
     int t = order_item->type;
     struct order *order = create_new_order(sec, amt, pr, t);/////need sec
     queue_put_order(&order_queue, order, strat_name);
}
*/

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

///////WHEN part
/*
void ex_process_statement(ast_process_statement * process_statement)
{
    ////extract algo_name from expression
    ////the expression must be IS node
    ast_exp *when_exp = process_statement->expression;
    if (when_exp->type == typeOper)
    {
        ast_exp *algo_call = when_exp->oper.op1;
        if (algo_call->type == typeOper)
        {
            ast_exp *algo_name = algo_call->oper.op1;
            if (algo_name->type == typeID)
            {
                char *name = algo_name->id.value;
                printf("calling algorithm: %s:\n", name);
                //////look up the global symbol table
                struct symbol_value* symtmp = symbol_table_get_value(root->sym, 0, name);
                void * algo_Ptr = symtmp->nodePtr;
                //////start thread for the algorithm
	        int retval;
	        void *result;
	        pthread_t algo_thr;
	        char *df_name = "df_ZBRA";
	        if( (df1 = create_data_source(df_name, DATAFEED)) == NULL )
		    die("error creating data source");
                struct algorithm *al1;
	        al1 = create_algorithm(df1); 
	        al1->num_args = 1;
	        struct currency *buy_price = (struct currency *)malloc(sizeof(struct currency));
	        if( buy_price == NULL )
		   die("malloc failed");

	        al1->args = buy_price;	
                al1->algo_Ptr = algo_Ptr;
	        //  Create the algorithm thread
	        if( pthread_create(&algo_thr, NULL, algorithm_handler, al1) != 0 )
		    die("algorithm thread create fail");
	        // ************************************** 
	        // *       Execute WHEN statement 
	        // **************************************
	        pthread_mutex_lock(&al1->mutex);
	        pthread_cond_wait(&al1->cond_true, &al1->mutex);
                /////emit order
                ast_action_list *action_list = process_statement->action_list;
                ex_action_list(action_list);
	        // * Release lock 
	        pthread_mutex_unlock(&al1->mutex);
	       //  **********************************
	        *  Tear down ALGORITHM thread
	        ********************************
	        if( pthread_cancel(algo_thr) != 0 )
		    perror("thread cancellation");

	        retval = pthread_join(algo_thr, &result);
	        if( retval != 0 )
		    perror("thread join");

	        if( result != PTHREAD_CANCELED )
		    perror("thread was not canceled");	
	
	        //  Destroy condition variable
	        pthread_cond_destroy(&al1->cond_true);	

	        if( buy_price != NULL )
		    free(buy_price);

	        fprintf(stderr, "[INFO] Exiting STRATEGY thread.\n");
                if( al1 != NULL )
		    free(al1);
            }
            else
            {
                printf("Expect algorithm name\n");
            }
        }
        else
        {
            printf("Expect function call\n");
        }
    }
    else
    {
        printf("Expect IS expression\n");
    }
}


void *algorithm_handler(void *arg)
{
    fprintf(stderr, "[INFO] STARTING ALGO THREAD\n");
    struct algorithm *algo = (struct algorithm *)arg;
    pthread_cond_signal(&algo->cond_true);
    pthread_mutex_lock(&algo->mutex);
    pthread_testcancel();
    pthread_cond_wait(&algo->cond_true, &algo->mutex);
    pthread_mutex_unlock(&algo->mutex);
    ////pthread_cleanup_pop(0);
    return (void *)0;
}

void ex_action_list(ast_action_list * action_list)
{
    int tmp = 0;
    while (tmp < action_list->num_of_orders)
    {
        /////execute order
     //   ex_order_item(action_list->order[tmp]);
        tmp += 1;
    }   
}

*/

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
			printf("--------------------------> This exp is ID\n");
			ret = (void*)symbol_table_get_value(p->id.sym, 0, p->id.value);
			return ret;
			
			break;
		case typeBooleanConst:
			printf("--------------------------> This exp is BooleanConst: %d\n", p->con.bool_value->value);
			ret = (void*)(intptr_t)p->con.bool_value->value;
			break;
		case typePriceConst:
			printf("--------------------------> This exp is PriceConst: %s\n", p->con.price_value->price );
			ret = (void*)p->con.price_value->price;
			break;
		case typeKeyword:
			printf("--------------------------> This exp is KEYword\n");
			break;

		case typeOper:
			switch(p->oper.oper) {
				case OP_LT:
					printf("--------------------------> Operator <\n");
					/* TODO: type check */

					if (price_to_long((char*)ex_exp(p->oper.op1))< price_to_long((char*)ex_exp(p->oper.op2)))
						ret = (void*)(intptr_t)1;
					else
						ret = (void*)(intptr_t)0;

					break;

				case OP_ASSIGN:
					printf("--------------------------> Operator =\n");
					struct symbol_value* symval;
					symval = (struct symbol_value*)ex_exp(p->oper.op1);
					copy_name(((ast_currency*)symval->nodePtr)->curr->p, (char*)ex_exp(p->oper.op2));
					break;

				case OP_IS:
					printf("--------------------------> Operator IS\n");
					ret = ex_exp(p->oper.op1);
					break;
				case OP_FUNC:
					printf("--------------------------> Operator FUNC\n");
					int type;
					sym_entry = (struct symbol_value*)ex_exp(p->oper.op1);
					type = sym_entry->type_specifier;
					if (type == ALGORITHM_T)
						printf("--------------------------> This is ALGO call to %s\n", sym_entry->identifier);
					else
						printf("--------------------------> This is Function call???\n");

					/* Create algorithm data structure */
					struct algorithm *algo_data = create_algorithm(df1);

					/* Get argument expr list */
					ast_argument_expression_list algo_args = p->oper.op2->argu_list;
					fprintf(stderr, "[INFO] Number of args: %d.\n", algo_args.num_of_argument_expression_list);


					fprintf(stderr, "[INFO] arg1 is : %d.\n", algo_args.exp[0]->type);
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
					   - if op2 is PRC, then return value in ex_exp(op1)->price */
					switch(p->oper.op2->attr.value){
						case PRC_T:
							printf("--------------------------> PRC_T\n");
							sym_entry = (struct symbol_value*)ex_exp(p->oper.op1);
							ret = (void*)((struct data*)sym_entry->nodePtr)->current_data.price;
							printf("------------------------------> PRC will be %s\n", (char*) ret);
							break;
						case NEXT_T:
							ret = (void*)ex_exp(p->oper.op1);
							break;
						default:
							printf("---------------------------> ATTR operator is not recognized\n");
					}
					printf("--------------------------> Operator ATTR: %s\n",  attr_tostring(p->oper.op2->attr.value));
					break;

				default:
					printf("---------------------------> Operator is not recognized: %d\n", p->oper.oper);
			}
		break;

		default:
			printf("--------------------> Expression type is not recognized: %d\n", p->type);

	}
	return ret;
}
