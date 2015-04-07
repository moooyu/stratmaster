#ifndef <_AST_H>
#define <_AST_H>


typedef struct {
    ast_uselist * uselist;
    ast_stratlist * stratlist
}ast_program;


typedef struct {
    char * account;
}ast_uselist;


typedef struct {
    ast_strat * strat;
}ast_stratlist;


typedef struct {
    char * strategy_name;
    ast_actionlist * actionlist;
}ast_strat;


typedef struct {
    ast_order * order;
}ast_actionlist;


typedef struct {
    int order_type;
    ast_orderitem * order_item;
}ast_order;


typedef struct {
    char * equity_identifier;
    int amount;
    int price;
}ast_order_item;

/*
typedef struct nodeTypeTag {
    nodeEnum type;
    union {
        conNodeType con;
        idNodeType id;
        oprNodeType opr;
    };
} nodeType;
*/


struct ast_program *
create_node_program(ast_uselist * uselist, ast_stratlist * stratlist);


struct ast_uselist *
create_node_uselist(char * name);


struct ast_stratlist *
create_node_stratlist(ast_strat * strat);


struct ast_strat *
create_node_strat(char * name, ast_actionlist * actionlist);


struct ast_actionlist *
create_node_actionlist(ast_order * order);


struct ast_order *
create_node_order(int order_type, ast_orderitem * order_item);


struct ast_order_item *
create_node_order_type(char * equity_identifier, int amount, int price);  // should be one more para(identifier)


#endif