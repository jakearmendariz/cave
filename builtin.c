
#include "lval.h"
//Built in operation
lval *builtin_op(lval *a, char *op)
{
    //Cbecks to make sure all cells are numbers
    for (int i = 0; i < a->count; i++)
    {
        if (a->cell[i]->type != LVAL_NUM)
        {
            lval_del(a);
            return lval_err("Cannot operate on non-number");
        }
    }

    //pop the top element
    lval *x = lval_pop(a, 0);

    /* If no arguments and sub then perform unary negation */
    //Bascially handles (-5)
    if ((strcmp(op, "-") == 0) && a->count == 0)
    {
        x->num = -x->num;
    }
    while (a->count > 0)
    {
        //pop top element
        lval *y = lval_pop(a, 0);
        if (strcmp(op, "+") == 0)
        {
            x->num += y->num;
        }
        if (strcmp(op, "-") == 0)
        {
            x->num -= y->num;
        }
        if (strcmp(op, "*") == 0)
        {
            x->num *= y->num;
        }
        if (strcmp(op, "/") == 0)
        {
            if (y->num == 0)
            {
                lval_del(x);
                lval_del(y);
                x = lval_err("Division By Zero!");
                break;
            }
            x->num /= y->num;
        }

        lval_del(y);
    }
    lval_del(a);
    return x;
}
