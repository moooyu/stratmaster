#ifndef SYMTAB_H
#define SYMTAB_H

#include <glib.h>

struct symbol_table {
   GHashTable *ht_symbols;
};

struct symbol_value {
	int type_specifier;
	char identifier[32];
};

struct symbol_table *symbol_table_create(void);
void symbol_table_destroy(struct symbol_table *);

void
symbol_table_put_value(struct symbol_table *symtab,
			int type_specifier,
			const char *identifier);

struct symbol_value*
symbol_table_get_value(struct symbol_table *symtab,
                       const char *identifier);

#endif /* SYMTAB_H */
