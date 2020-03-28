#ifndef lval_h

#define lval_h
#include <stdlib.h>
#include "mpc.h"
#include <editline/readline.h>
//#include "builtin.h"

enum
{
    LVAL_ERR,
    LVAL_NUM,
    LVAL_SYM,
    LVAL_SEXPR,
    LVAL_QEXPR
};

typedef struct lval
{
    int type; //Check enum, one of 4 types
    long num; //value if num

    //Error and symbol have a string representation
    char *err;
    char *sym;

    //keeps track of how many lval
    int count;
    struct lval **cell;
} lval;

// construct a pointer to a new number lisp value
lval *lval_num(long x);

//lval lval error
lval *lval_err(char *m);

//lval lval symvol
lval *lval_sym(char *m);

//lval lval s-expression
lval *lval_sexpr();

//lval lval s-expression
lval *lval_qexpr(void);

void lval_del(lval *v);

lval *lval_read_num(mpc_ast_t *t);

lval *lval_add(lval *v, lval *x);

lval *lval_read(mpc_ast_t *t);

void lval_expr_print(lval *v, char open, char close);

void lval_print(lval *v);
void lval_println(lval *v);

lval *lval_eval(lval *v);
lval *lval_pop(lval *v, int i);
lval *builtin_op(lval *a, char *op);
lval *lval_take(lval *v, int i);

lval *lval_eval_sexpr(lval *v);

lval *lval_join(lval *x, lval *y);

//builtin function lookup, uses the built in class, uses correct function
lval *builtin(lval *a, char *func);

//Built in operation
lval *builtin_op(lval *a, char *op);

lval *builtin_head(lval *a);

lval *builtin_tail(lval *a);

lval *builtin_list(lval *a);

lval *builtin_eval(lval *a);

lval *builtin_join(lval *a);
#endif