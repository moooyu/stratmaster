#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "symtab.h"

struct symbol_table *
symbol_table_create(void)
{
   struct symbol_table *symtab;

   symtab = calloc(1, sizeof(struct symbol_table *));
   if (symtab == NULL) {
      fprintf(stderr, "Memory allocation request failed.\n");
      exit(EXIT_FAILURE);
   }

   symtab->ht_symbols = g_hash_table_new(g_str_hash, g_str_equal);
   return symtab;
}

void
symbol_table_destroy(struct symbol_table *symtab)
{
   g_hash_table_destroy(symtab->ht_symbols);
}

	void
symbol_table_put_value(struct symbol_table *symtab,
		int type_specifier,
		const char *identifier)
{
	struct symbol_value* symval;
	symval = malloc(sizeof(struct symbol_value));
	if (!symval) {
		fprintf(stderr, "Fail to allocate memory for the symbol %s\n", identifier);
		exit (EXIT_FAILURE);
	}
	symval->type_specifier = type_specifier;
	memcpy(symval->identifier, identifier, 32); /* TODO size? */
	g_hash_table_insert(symtab->ht_symbols, (gpointer) identifier, symval);
}

struct symbol_value*
symbol_table_get_value(struct symbol_table *symtab,
                       const char *identifier)
{
   return g_hash_table_lookup(symtab->ht_symbols, identifier);
}


