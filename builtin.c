
#include "lisp.h"

#define assert(args, cond, fmt,...)          \
    if (!(cond))                                    \
    {                                               \
        lval* err = lval_err(fmt, ##__VA_ARGS__);   \
        lval_del(args);                             \
        return err;                                 \
    }
#define assert_type(func, args, index, expect) \
  assert(args, args->cell[index]->type == expect, \
    "Function '%s' passed incorrect type for argument %i. Got %s, Expected %s.", \
    func, index, ltype_name(args->cell[index]->type), ltype_name(expect))

#define assert_num(func, args, num) \
  assert(args, args->count == num, \
    "Function '%s' passed incorrect number of arguments. Got %i, Expected %i.", \
    func, args->count, num)

#define assert_not_empty(func, args, index) \
  assert(args, args->cell[index]->count != 0, \
    "Function '%s' passed {} for argument %i.", func, index);
 
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
    assert(a, a->count == 1, "Function 'head' passed too many arguments. Got %i, Expected %i", a->count, 1);
    assert(a, a->cell[0]->type == LVAL_QEXPR, "Function 'head' passed incorrect type!");
    assert(a, a->cell[0]->count != 0,
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
    assert(a, a->count == 1,
                  "Function 'tail' passed too many arguments. Got %i, Expected %i", a->count, 1);
    assert(a, a->cell[0]->type == LVAL_QEXPR,
                  "Function 'tail' passed incorrect type!");
    assert(a, a->cell[0]->count != 0,
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
    assert(a, a->count == 1,
                  "Function 'eval' passed too many arguments!");
    assert(a, a->cell[0]->type == LVAL_QEXPR,
                  "Function 'eval' passed incorrect type!");

    lval *x = lval_take(a, 0);
    x->type = LVAL_SEXPR;
    return lval_eval(e, x);
}

lval *builtin_join(lenv *e, lval *a)
{
    for (int i = 0; i < a->count; i++)
    {
        assert(a, a->cell[i]->type == LVAL_QEXPR, "Function 'join' passed incorrect type.");
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


lval* builtin_if(lenv* e, lval* a){
    assert_num("if", a, 3);
    //Checks that the inputs are of the correct value
    assert_type("if", a, 0, LVAL_NUM);
    assert_type("if", a, 1, LVAL_QEXPR);
    assert_type("if", a, 2, LVAL_QEXPR);

    //Mark both expressions as evaluable
    lval *x;
    a->cell[1]->type = LVAL_SEXPR;
    a->cell[2]->type = LVAL_SEXPR;

    if(a->cell[0]->num){
        //if condition is true evalue the first expression
        x = lval_eval(e, lval_pop(a, 1));
    }else{
        x = lval_eval(e, lval_pop(a, 2));
    }

    lval_del(a);
    return x;

}

lval* builtin_ord(lenv* e, lval* a, char* op){
    assert(a, (a->count == 2), "Two arguments required");

    int r = 0;
    if (strcmp(op,"==")  == 0) {
        if(a->cell[0]->type == LVAL_STR){
            r = (strcmp(a->cell[0]->str, a->cell[1]->str) == 0);
        }else{
            r = (a->cell[0]->num ==  a->cell[1]->num);
        }
    }
    else if (strcmp(op, "!=")  == 0) {
        if(a->cell[0]->type == LVAL_STR){
            r = (strcmp(a->cell[0]->str, a->cell[1]->str) != 0);
        }else{
            r = (a->cell[0]->num !=  a->cell[1]->num);
        }
    }
    else if (strcmp(op, ">")  == 0) {
        r = (a->cell[0]->num >  a->cell[1]->num);
    }
    else if (strcmp(op, "<")  == 0) {
        r = (a->cell[0]->num <  a->cell[1]->num);
    }
    else if (strcmp(op, ">=") == 0) {
        r = (a->cell[0]->num >= a->cell[1]->num);
    }
    else if (strcmp(op, "<=") == 0) {
        r = (a->cell[0]->num <= a->cell[1]->num);
    }
    lval_del(a);
    return lval_num(r);
}

lval* builtin_eq(lenv* e, lval* a){
    return builtin_ord(e, a, "==");
}

lval* builtin_ne(lenv* e, lval* a){
    return builtin_ord(e, a, "!=");
}

lval* builtin_gt(lenv* e, lval* a){
    return builtin_ord(e, a, ">");
}

lval* builtin_lt(lenv* e, lval* a){
    return builtin_ord(e, a, "<");
}

lval* builtin_ge(lenv* e, lval* a){
    return builtin_ord(e, a, ">=");
}

lval* builtin_var(lenv* e, lval* a, char* func){
    assert(a, a->type != LVAL_QEXPR, "Error: to define a function or variable please add inbetween {}");

    lval* syms = a->cell[0];
    for(int i = 0; i < syms->count; i++){
        assert(a, (syms->cell[i]->type == LVAL_SYM),
      "Function '%s' cannot define non-symbol. "
      "Got %s, Expected %s.", func,
      ltype_name(syms->cell[i]->type),
      ltype_name(LVAL_SYM));
    }

    assert(a, (syms->count == a->count-1),
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
   
}

lval* builtin_lambda(lenv* e, lval* a){
    assert(a, a->count == 2,
                  "Function 'lambda' needs two arguments");
    assert(a, a->cell[0]->type == LVAL_QEXPR,
                  "Type needs to be quote expressiomn for lambda function. Not a %s", ltype_name(a->cell[0]->type));

    //check first q expressions contains only symbols
    for (int i = 0; i < a->cell[0]->count; i++) {
        assert(a, (a->cell[0]->cell[i]->type == LVAL_SYM),
        "Cannot define non-symbol. Got %s, Expected %s.",
        ltype_name(a->cell[0]->cell[i]->type),ltype_name(LVAL_SYM));
    }

    lval* formals = lval_pop(a, 0);
    lval* body = lval_pop(a, 0);
    lval_del(a);

    return lval_lambda(formals, body);
}

lval* builtin_print(lenv* e, lval *a){
    //print each argument followed by a spce
    for(int i = 0; i < a->count; i++){
        lval_print(a->cell[i]);
        putchar(' ');
    }
    putchar('\n');
    lval_del(a);

    return lval_sexpr();
}

lval* builtin_error(lenv* e, lval* a) {
  assert_num("error", a, 1);
  assert_type("error", a, 0, LVAL_STR);

  /* Construct Error from first argument */
  lval* err = lval_err(a->cell[0]->str);

  /* Delete arguments and return */
  lval_del(a);
  return err;
}

//Loads a file to run
lval* builtin_load(lenv* e, lval* a) {
    print("Loading file");
    assert_num("load", a, 1);
    assert_type("load", a, 0, LVAL_STR);

    mpc_result_t r;
    if(mpc_parse_contents(a->cell[0]->str, Lispy, &r)){
        //read the contents
        lval* expr = lval_read(r.output);
        mpc_ast_delete(r.output);

        //while there is more lines to evaluate
        while(expr->count){
            lval* x = lval_eval(e, lval_pop(expr, 0));
            lval_println(x);
            if(x->type == LVAL_ERR){
                lval_println(x);
            }
            lval_del(x);
        }

        lval_del(expr);
        lval_del(a);
        return lval_sexpr();
    }else{
        char *err_msg = mpc_err_string(r.error);
        mpc_err_delete(r.error);
        /* Create new error message using it */
        lval* err = lval_err("Could not load Library %s", err_msg);
        free(err_msg);
        lval_del(a);

        /* Cleanup and return error */
        return err;
    }
}
