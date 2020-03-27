#include "mpc.h"
typedef struct
{
    int type;
    long num;
    int err;
} lval;

/* Create a new number type lval */
lval lval_num(long x);
/* Create a new error type lval */
lval lval_err(int x);
/* Print an "lval" */
void lval_print(lval v);
/* Print an "lval" followed by a newline */
void lval_println(lval v);
lval eval_op(lval x, char *op, lval y);
lval eval(mpc_ast_t *t);

enum
{
    LVAL_NUM,
    LVAL_ERR
};
enum
{
    LERR_DIV_ZERO,
    LERR_BAD_OP,
    LERR_BAD_NUM
};