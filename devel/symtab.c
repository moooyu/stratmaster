#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "symtab.h"
#include "runtime.h"

void print_symtab(struct symbol_table* top)
{
	GHashTableIter iter;
	gpointer key, value;
	struct symbol_table *cur_table;
	struct symbol_value* symval;

	g_hash_table_iter_init (&iter, top->ht_symbols);
	PRINTS(("symtab start\n"));
	while (g_hash_table_iter_next (&iter, &key, &value))
	{
		struct symbol_value *val = (struct symbol_value *)value;
		// do something with key and value
		PRINTS(("%s %s\n", val->identifier, type_tostring(val->type_specifier)));
		if( val->type_specifier == ACCOUNT_T )
		{
			struct account *ac = (struct account*)val->nodePtr;
			PRINTS(("Available cash = %s\n", ac->avail_cash.p));

		}
		if( val->type_specifier == DATAFEED_T )
		{
			struct data *data_src = (struct data*)val->nodePtr;
			PRINTS(("Data source = %s\n", data_src->df_filename));

		}

	}
	PRINTS(("symtab end\n"));

	for (cur_table = top->parent; cur_table != NULL; cur_table = cur_table->parent) {
		PRINTS(("Up one level\n"));
		print_symtab(cur_table);
	}
}

struct symbol_table *
symbol_table_create(struct symbol_table* parent)
{
	struct symbol_table *symtab;

	symtab = calloc(1, sizeof(struct symbol_table *));
	if (symtab == NULL) {
		die("Memory allocation request failed.\n");
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

int
symbol_table_put_value(struct symbol_table *symtab,
		int type_specifier,
		const char *identifier,
		void* nodePtr)
{
	struct symbol_value* symval;
	char *id;
	symval = g_hash_table_lookup(symtab->ht_symbols, identifier);
	if (symval != NULL) {
		return 1;
	}

	symval = malloc(sizeof(struct symbol_value));
	if (!symval) {
		die("Fail to allocate memory for the symbol entry\n");
	}

	symval->type_specifier = type_specifier;
	symval->nodePtr = nodePtr;
	memcpy(symval->identifier, identifier, NAMEBUF);

	id = malloc(NAMEBUF); /* TODO free */
	if (!id) {
		die("Fail to allocate memory for the symbol entry\n");
	}
	strcpy(id, identifier);

	PRINTS(("Symbol %s is put\n", id));
	g_hash_table_insert(symtab->ht_symbols, (gpointer) id, symval);
	return 0;
}

struct symbol_value*
symbol_table_get_value(struct symbol_table *symtab,
		int type_specifier,
		const char *identifier)
{
	struct symbol_table *cur_table = symtab;
	struct symbol_value* symval;

	for (; cur_table != NULL; cur_table = cur_table->parent) {
		symval = g_hash_table_lookup(cur_table->ht_symbols, identifier);
		if (symval != NULL)
			return symval;
	}

	return NULL;
}

