#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "symtab.h"

void print_symtab(struct symbol_table* top)
{
	GHashTableIter iter;
	gpointer key, value;
	struct symbol_table *cur_table;
	struct symbol_value* symval;

	g_hash_table_iter_init (&iter, top->ht_symbols);
	printf("symtab start\n");
	while (g_hash_table_iter_next (&iter, &key, &value))
	{
		printf("%s\n", (char*)key);
		// do something with key and value
	}
	printf("symtab end\n");

	for (cur_table = top->parent; cur_table != NULL; cur_table = cur_table->parent) {
		printf("Up one level\n");
		print_symtab(cur_table);
	}
}

struct symbol_table *
symbol_table_create(struct symbol_table* parent)
{
   struct symbol_table *symtab;

   symtab = calloc(1, sizeof(struct symbol_table *));
   if (symtab == NULL) {
      fprintf(stderr, "Memory allocation request failed.\n");
      exit(EXIT_FAILURE);
   }

   symtab->ht_symbols = g_hash_table_new(g_str_hash, g_str_equal);
   symtab->parent = parent;
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
		const char *identifier,
		void* nodePtr)
{
	struct symbol_value* symval;
	char *id;
	symval = g_hash_table_lookup(symtab->ht_symbols, identifier);
	if (symval != NULL) {
		fprintf(stderr, "Duplicated symbol %s\n", identifier);
		exit (EXIT_FAILURE);
	}

	symval = malloc(sizeof(struct symbol_value));
	if (!symval) {
		fprintf(stderr, "Fail to allocate memory for the symbol %s\n", identifier);
		exit (EXIT_FAILURE);
	}

	symval->type_specifier = type_specifier;
	symval->nodePtr = nodePtr;
	memcpy(symval->identifier, identifier, 32); /* TODO size? */

	id = malloc(32); /* TODO free */
	if (!id) {
		fprintf(stderr, "Fail to allocate memory for the symbol %s\n", identifier);
		exit (EXIT_FAILURE);
	}
	strcpy(id, identifier);
	
	g_hash_table_insert(symtab->ht_symbols, (gpointer) id, symval);
}

struct symbol_value*
symbol_table_get_value(struct symbol_table *symtab,
		int type_specifier,
		const char *identifier)
{
	struct symbol_table *cur_table = symtab;
	struct symbol_value* symval;
	for (; cur_table != NULL; cur_table = cur_table->parent) {
		symval = g_hash_table_lookup(symtab->ht_symbols, identifier);
		if (symval != NULL)
			return symval;
	}

	fprintf(stderr, "Symbol not found: %s\n", identifier);
	exit (EXIT_FAILURE);
	return NULL;
}

