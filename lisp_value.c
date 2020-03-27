#include "lisp_value.h"

/* Create a new number type lval */
lval lval_num(long x)
{
    lval v;
    v.type = LVAL_NUM;
    v.num = x;
    return v;
}
/* Create a new error type lval */
lval lval_err(int x)
{
    lval v;
    v.type = LVAL_ERR;
    v.err = x;
    return v;
}
/* Print an "lval" */
void lval_print(lval v)
{
    if (v.type == LVAL_NUM)
    {
        printf("%li", v.num);
    }
    else
    {
        /* Check what type of error it is and print it */
        if (v.err == LERR_DIV_ZERO)
        {
            printf("Error: Division By Zero!");
        }
        if (v.err == LERR_BAD_OP)
        {
            printf("Error: Invalid Operator!");
        }
        if (v.err == LERR_BAD_NUM)
        {
            printf("Error: Invalid Number!");
        }
    }
}
/* Print an "lval" followed by a newline */
void lval_println(lval v)
{
    lval_print(v);
    putchar('\n');
}

lval eval_op(lval x, char *op, lval y)
{
    /* If either value is an error return it */
    if (x.type == LVAL_ERR)
    {
        return x;
    }
    if (y.type == LVAL_ERR)
    {
        return y;
    }

    /* Otherwise do maths on the number values */
    if (strcmp(op, "+") == 0)
    {
        return lval_num(x.num + y.num);
    }
    if (strcmp(op, "-") == 0)
    {
        return lval_num(x.num - y.num);
    }
    if (strcmp(op, "*") == 0)
    {
        return lval_num(x.num * y.num);
    }
    if (strcmp(op, "/") == 0)
    {
        /* If second operand is zero return error */
        // condition ? then : else
        return y.num == 0
                   ? lval_err(LERR_DIV_ZERO)
                   : lval_num(x.num / y.num);
    }
    if (strcmp(op, "^") == 0)
    {
        long a = 1;
        for (int i = 0; i < y.num; i++)
        {
            a *= x.num;
        }
        return lval_num(a);
    }
    return lval_err(LERR_BAD_OP);
}

lval eval(mpc_ast_t *t)
{
    if (strstr(t->tag, "number"))
    {
        errno = 0;                              //Indicates there is no error
        long x = strtol(t->contents, NULL, 10); //string to long

        // if(0 is not ERANGE) then return lval_num(x) else return lval_err(LERR_BAD_NUM);
        return errno != ERANGE ? lval_num(x) : lval_err(LERR_BAD_NUM);
    }

    //Operation is always in this position
    char *op = t->children[1]->contents;
    lval x = eval(t->children[2]);

    int i = 3;
    //while the tag is "expr" there is more input in string
    while (strstr(t->children[i]->tag, "expr"))
    {
        x = eval_op(x, op, eval(t->children[i]));
        i++;
    }
    return x;
}