#ifndef SYMTAB_H
#define SYMTAB_H

#include <glib.h>

enum { ACCT_SYM = 1000, STRAT_SYM, ALGO_SYM, FUNC_SYM, PRICE_SYM};


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

void
symbol_table_put_value(struct symbol_table *symtab,
			int type_specifier,
			const char *identifier,
			void* nodePtr);

struct symbol_value*
symbol_table_get_value(struct symbol_table *symtab,
			int type_specifier,
                       const char *identifier);

#endif /* SYMTAB_H */
