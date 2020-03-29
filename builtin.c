
#include "lisp.h"
#define error_message(args, cond, fmt,...)          \
    if (!(cond))                                    \
    {                                               \
        lval* err = lval_err(fmt, ##__VA_ARGS__);   \
        lval_del(args);                             \
        return err;                                 \
    }
//Built in operation
lval *builtin_op(lenv *e, lval *a, char *op)
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

lval *builtin_head(lenv *e, lval *a)
{
    error_message(a, a->count == 1, "Function 'head' passed too many arguments. Got %i, Expected %i", a->count, 1);
    error_message(a, a->cell[0]->type == LVAL_QEXPR, "Function 'head' passed incorrect type!");
    error_message(a, a->cell[0]->count != 0,
                  "Function 'head' passed {}!");

    lval *v = lval_take(a, 0);
    while (v->count > 1)
    {
        lval_del(lval_pop(v, 1));
    }
    return v;
}

lval *builtin_tail(lenv *e, lval *a)
{
    error_message(a, a->count == 1,
                  "Function 'tail' passed too many arguments. Got %i, Expected %i", a->count, 1);
    error_message(a, a->cell[0]->type == LVAL_QEXPR,
                  "Function 'tail' passed incorrect type!");
    error_message(a, a->cell[0]->count != 0,
                  "Function 'tail' passed {}!");

    lval *v = lval_take(a, 0);
    lval_del(lval_pop(v, 0));
    return v;
}

//converts from s-expression to q-expression
lval *builtin_list(lenv *e, lval *a)
{
    a->type = LVAL_QEXPR;
    return a;
}

lval *builtin_eval(lenv *e, lval *a)
{
    error_message(a, a->count == 1,
                  "Function 'eval' passed too many arguments!");
    error_message(a, a->cell[0]->type == LVAL_QEXPR,
                  "Function 'eval' passed incorrect type!");

    lval *x = lval_take(a, 0);
    x->type = LVAL_SEXPR;
    return lval_eval(e, x);
}

lval *builtin_join(lenv *e, lval *a)
{
    for (int i = 0; i < a->count; i++)
    {
        error_message(a, a->cell[i]->type == LVAL_QEXPR, "Function 'join' passed incorrect type.");
    }
    lval *x = lval_pop(a, 0);

    while (a->count)
    {
        x = lval_join(x, lval_pop(a, 0));
    }
    lval_del(a);
    return x;
}

lval *builtin(lenv *e, lval *a, char *func)
{
    if (strcmp("list", func) == 0)
    {
        return builtin_list(e, a);
    }
    if (strcmp("head", func) == 0)
    {
        return builtin_head(e, a);
    }
    if (strcmp("tail", func) == 0)
    {
        return builtin_tail(e, a);
    }
    if (strcmp("join", func) == 0)
    {
        return builtin_join(e, a);
    }
    if (strcmp("eval", func) == 0)
    {
        return builtin_eval(e, a);
    }
    if (strstr("+-/*", func))
    {
        return builtin_op(e, a, func);
    }
    lval_del(a);
    return lval_err("Unknown Function!");
}

lval *builtin_add(lenv *e, lval *a)
{
    return builtin_op(e, a, "+");
}

lval *builtin_sub(lenv *e, lval *a)
{
    return builtin_op(e, a, "-");
}

lval *builtin_mul(lenv *e, lval *a)
{
    return builtin_op(e, a, "*");
}

lval *builtin_div(lenv *e, lval *a)
{
    return builtin_op(e, a, "/");
}

lval* builtin_var(lenv* e, lval* a, char* func){
    error_message(a, a->type != LVAL_QEXPR, "Error: to define a function or variable please add inbetween {}");

    lval* syms = a->cell[0];
    for(int i = 0; i < syms->count; i++){
        error_message(a, (syms->cell[i]->type == LVAL_SYM),
      "Function '%s' cannot define non-symbol. "
      "Got %s, Expected %s.", func,
      ltype_name(syms->cell[i]->type),
      ltype_name(LVAL_SYM));
    }

    error_message(a, (syms->count == a->count-1),
    "Function '%s' passed too many arguments for symbols. "
    "Got %i, Expected %i.", func, syms->count, a->count-1);

    for(int i = 0; i < syms->count; i++){
        /* If 'def' define in globally. If 'put' define in locally */
        if (strcmp(func, "def") == 0) {
            lenv_def(e, syms->cell[i], a->cell[i+1]);
        }
    
        if (strcmp(func, "=")   == 0) {
            lenv_put(e, syms->cell[i], a->cell[i+1]);
        }
    }
    lval_del(a);
    return lval_sexpr();
}

lval* builtin_put(lenv* e, lval *a){
    return builtin_var(e, a, "put");
}

//Define
lval* builtin_def(lenv* e, lval* a) {
    return builtin_var(e, a, "def");
    /**
  error_message(a, a->cell[0]->type == LVAL_QEXPR,
    "Function 'def' passed incorrect type!");

  lval* syms = a->cell[0];
  for (int i = 0; i < syms->count; i++) {
    lval_println(syms->cell[i]);
    error_message(a, syms->cell[i]->type == LVAL_SYM,
      "Function 'def' cannot define non-symbol");
  }
  error_message(a, syms->count == a->count-1,
    "Function 'def' cannot define incorrect "
    "number of values to symbols");

  for (int i = 0; i < syms->count; i++) {
    lval_println(a->cell[i+1]);
    lenv_put(e, syms->cell[i], a->cell[i+1]);
  }

  lval_del(a);
  
  return lval_sexpr();
  */
}

lval* builtin_lambda(lenv* e, lval* a){
    error_message(a, a->count == 2,
                  "Function 'lambda' needs two arguments");
    error_message(a, a->cell[0]->type == LVAL_QEXPR,
                  "Type needs to be quote expressiomn for lambda function. Not a %s", ltype_name(a->cell[0]->type));

    //check first q expressions contains only symbols
    for (int i = 0; i < a->cell[0]->count; i++) {
        error_message(a, (a->cell[0]->cell[i]->type == LVAL_SYM),
        "Cannot define non-symbol. Got %s, Expected %s.",
        ltype_name(a->cell[0]->cell[i]->type),ltype_name(LVAL_SYM));
    }

    lval* formals = lval_pop(a, 0);
    lval* body = lval_pop(a, 0);
    lval_del(a);

    return lval_lambda(formals, body);
}
