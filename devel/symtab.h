#ifndef SYMTAB_H
#define SYMTAB_H

#include <glib.h>

//#define DEBUGS 1

#ifdef DEBUGS
#define PRINTS(x)  do { if (DEBUGS) dbg_printf x; } while (0)
#else
#define PRINTS(x) do {} while(0)
#endif

void dbg_printf(const char *fmt, ...);

enum { ACCT_SYM = 1000, DTF_SYM, DTB_SYM, EXG_SYM, STRAT_SYM, ALGO_SYM, FUNC_SYM, PRICE_SYM};
enum { INT_T = 100, DOUBLE_T, BOOLEAN_T, SECURITY_T, CURRENCY_T, VOID_T, ACCOUNT_T, DATAFEED_T, DATABASE_T, EXCHANGE_T, } type_specifier; 

struct symbol_table {
   GHashTable *ht_symbols;
   struct symbol_table* parent;
};

struct symbol_value {
	int type_specifier;
	char identifier[32];
	void* nodePtr;
};

struct symbol_table *symbol_table_create(struct symbol_table* parent);
void symbol_table_destroy(struct symbol_table *);

int
symbol_table_put_value(struct symbol_table *symtab,
			int type_specifier,
			const char *identifier,
			void* nodePtr);

struct symbol_value*
symbol_table_get_value(struct symbol_table *symtab,
			int type_specifier,
                       const char *identifier);

#endif /* SYMTAB_H */
