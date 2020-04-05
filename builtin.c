
#include "cave.h"

/**
 * builtin functions
 * These are functions I have added (pre-library)
 */

#define assert(args, cond, fmt,...)          \
    if (!(cond))                                    \
    {                                               \
        cval* err = cval_err(fmt, ##__VA_ARGS__);   \
        cval_del(args);                             \
        return err;                                 \
    }
#define assert_type(func, args, index, expect) \
  assert(args, args->cell[index]->type == expect, \
    "Function '%s' passed incorrect type for argument %i. Got %s, Expected %s.", \
    func, index, ctype_name(args->cell[index]->type), ctype_name(expect))

#define assert_num(func, args, num) \
  assert(args, args->count == num, \
    "Function '%s' passed incorrect number of arguments. Got %i, Expected %i.", \
    func, args->count, num)

#define assert_not_empty(func, args, index) \
  assert(args, args->cell[index]->count != 0, \
    "Function '%s' passed {} for argument %i.", func, index);
 
//Built in operation
cval *builtin_op(cave_env *e, cval *a, char *op)
{
    //Cbecks to make sure all cells are numbers
    for (int i = 0; i < a->count; i++)
    {
        if (a->cell[i]->type != cval_NUM)
        {
            cval_del(a);
            return cval_err("Cannot operate on non-number");
        }
    }

    //pop the top element
    cval *x = cval_pop(a, 0);

    /* If no arguments and sub then perform unary negation */
    //Bascially handles (-5)
    if ((strcmp(op, "-") == 0) && a->count == 0)
    {
        x->num = -x->num;
    }
    while (a->count > 0)
    {
        //pop top element
        cval *y = cval_pop(a, 0);
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
                cval_del(x);
                cval_del(y);
                x = cval_err("Division By Zero!");
                break;
            }
            x->num /= y->num;
        }

        cval_del(y);
    }
    cval_del(a);
    return x;
}

cval *builtin_head(cave_env *e, cval *a)
{
    assert(a, a->count == 1, "Function 'head' passed too many arguments. Got %i, Expected %i", a->count, 1);
    assert(a, a->cell[0]->type == cval_QEXPR, "Function 'head' passed incorrect type!");
    assert(a, a->cell[0]->count != 0,
                  "Function 'head' passed {}!");

    cval *v = cval_take(a, 0);
    while (v->count > 1)
    {
        cval_del(cval_pop(v, 1));
    }
    return v;
}

cval *builtin_tail(cave_env *e, cval *a)
{
    assert(a, a->count == 1,
                  "Function 'tail' passed too many arguments. Got %i, Expected %i", a->count, 1);
    assert(a, a->cell[0]->type == cval_QEXPR,
                  "Function 'tail' passed incorrect type!");
    assert(a, a->cell[0]->count != 0,
                  "Function 'tail' passed {}!");

    cval *v = cval_take(a, 0);
    cval_del(cval_pop(v, 0));
    return v;
}

//converts from s-expression to q-expression
cval *builtin_list(cave_env *e, cval *a)
{
    a->type = cval_QEXPR;
    return a;
}

cval *builtin_eval(cave_env *e, cval *a)
{
    assert(a, a->count == 1,
                  "Function 'eval' passed too many arguments!");
    assert(a, a->cell[0]->type == cval_QEXPR,
                  "Function 'eval' passed incorrect type!");

    cval *x = cval_take(a, 0);
    x->type = cval_SEXPR;
    return cval_eval(e, x);
}

cval *builtin_join(cave_env *e, cval *a)
{
    for (int i = 0; i < a->count; i++)
    {
        assert(a, a->cell[i]->type == cval_QEXPR, "Function 'join' passed incorrect type.");
    }
    cval *x = cval_pop(a, 0);

    while (a->count)
    {
        x = cval_join(x, cval_pop(a, 0));
    }
    cval_del(a);
    return x;
}



cval *builtin(cave_env *e, cval *a, char *func)
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
    cval_del(a);
    return cval_err("Unknown Function!");
}

cval *builtin_add(cave_env *e, cval *a)
{
    return builtin_op(e, a, "+");
}

cval *builtin_sub(cave_env *e, cval *a)
{
    return builtin_op(e, a, "-");
}

cval *builtin_mul(cave_env *e, cval *a)
{
    return builtin_op(e, a, "*");
}

cval *builtin_div(cave_env *e, cval *a)
{
    return builtin_op(e, a, "/");
}


cval* builtin_if(cave_env* e, cval* a){
    print("builtin_if");
    assert_num("if", a, 3);
    //Checks that the inputs are of the correct value
    assert_type("if", a, 0, cval_NUM);
    assert_type("if", a, 1, cval_QEXPR);
    assert_type("if", a, 2, cval_QEXPR);

    //Mark both expressions as evaluable
    cval *x;
    a->cell[1]->type = cval_SEXPR;
    a->cell[2]->type = cval_SEXPR;

    if(a->cell[0]->num){
        //if condition is true evalue the first expression
        x = cval_eval(e, cval_pop(a, 1));
    }else{
        x = cval_eval(e, cval_pop(a, 2));
    }

    cval_del(a);
    return x;
}

//for loop, repeats for l times
//format: for {x in arr} {expression}
//format: for {x from 0 10} {expression} //turn expression into a function with a parameter x
// x = cell[0] | for-type = cell[1] | start = cell[2] | end = cell[3] | function inner = cell[4]
//have the value of the array or the count
cval* builtin_for(cave_env* e, cval* a){
    printf("builtin_for\n\n");
    assert_num("for", a, 2);
    //Checks that the inputs are of the correct value
    assert_type("for", a, 0, cval_QEXPR);
    assert_type("for", a, 1, cval_QEXPR);
    //Mark both expressions as evaluable
    //cval *x;
    cval* instruct = cval_pop(a, 0); //Pop the inner loop to be evaluated on its own
    
    assert_type("for", instruct, 1, cval_SYM);
    cval* variable = cval_pop(instruct, 0);
    cval* command = cval_pop(instruct, 0);
    if(strcmp(command->sym, "from")== 0){
        assert_num("for", instruct, 2);
        assert_type("for", instruct, 0, cval_NUM);
        assert_type("for", instruct, 1, cval_NUM);

        cval* from = cval_pop(instruct, 0);
        cval* to = cval_pop(instruct, 0);

        //for i range 0 10 {def {sum} (+ i sum)}
        cval *body;
        //Create a function with lable forinner. with paramtere given
        for(int i = from->num; i < to->num; i++){
            body = cval_get(a, 0);
            body->type = cval_SEXPR;
            //define variable x to be the value of value of i (inside cave_env)
            cave_env_def(e, variable, cval_num(i));
            //Preformt the function operation inside the forloop with parameter x
            print("printing for body");
            cval_println(body);
            cval_eval(e, body);
        }
    }else if (strcmp(command->sym, "in")== 0){
        assert_num("for", instruct, 1);
        cval* arr = cval_pop(instruct, 0);

        if(arr->type == cval_SYM){
            arr = cave_env_get(e, arr);
        }
        //for {i in array} {body(i)}
        cval *body;
        //Create a function with lable forinner. with paramtere given
        while(arr->count > 0){
            body = cval_get(a, 0);
            body->type = cval_SEXPR;
            //define variable x to be the value of value of i (inside cave_env)
            cave_env_def(e, variable, cval_pop(arr, 0));
            //Preform the function operation inside the forloop with parameter x
            printf("printing for body\n");
            cval_println(body);
            cval_eval(e, body);
        }
    }else{
        return cval_err("In forloop no function %s\n", command->sym);
    }
    cval_del(a);
    return cval_sexpr();
}


cval *builtin_length(cave_env *e, cval *a){
    return cval_num(a->cell[0]->count);
}

//Problem in while loops: runs infinetly because it will not save variable that is incremented within
cval* builtin_while(cave_env* e, cval* a){
    assert_num("while", a, 2);
    //Checks that the inputs are of the correct value
    assert_type("while", a, 0, cval_QEXPR);
    assert_type("while", a, 1, cval_QEXPR);
    //Mark both expressions as evaluable
    cval *condition = cval_copy(a->cell[0]);
    condition->type = cval_SEXPR;
    cval *condi = cval_eval(e,condition);
    int test = 1;
    if(condi->type == cval_NUM){
        if(condi->num == 0){
            test = 0;
        }
    }
    cval *body;
    while(test){
        body = cval_get(a, 1);
        body->type = cval_SEXPR;
        cval_del(condition);
        condition = cval_copy(a->cell[0]);
       
        condition->type = cval_SEXPR;

        cval_eval(e, body);
        condi = cval_eval(e, condition);
        
        if(condi->type == cval_NUM){
            if(condi->num == 0){
                test = 0;
            }
        }
    }

    cval_del(a);
    return cval_sexpr();
}

cval* builtin_while1(cave_env* e, cval* a){
    assert_num("while", a, 2);
    //Checks that the inputs are of the correct value
    assert_type("while", a, 0, cval_NUM);
    assert_type("while", a, 1, cval_QEXPR);

    //Mark both expressions as evaluable
    cval *x;
    cval *cond = cval_pop(a, 0);
    printf("printing condtion: ");
    cval_print(cond);
    cval *body;
    int counter = 0;
    while(cond){
        counter++;
        if(counter > 10){
            break;
        }
        //cval_print((cval_eval(e, cond)));
        body = cval_get(a, 1);
        body->type = cval_SEXPR;
        cond = cval_copy(a->cell[0]);
        cval_eval(e, body);
    }

    cval_del(a);
    x = cval_sexpr();
    return x;
}


cval* builtin_ord(cave_env* e, cval* a, char* op){
    assert(a, (a->count == 2), "Two arguments required");

    int r = 0;
    if (strcmp(op, ">")  == 0) {
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
    }else if(strcmp(op, "<=") == 0) {
        r = a->cell[0]->num + a->cell[2]->num;
    }
    cval_del(a);
    return cval_num(r);
}

cval* builtin_eq(cave_env* e, cval* a){
    return cval_num(cval_eq(a->cell[0], a->cell[1]));
}

cval* builtin_ne(cave_env* e, cval* a){
    return cval_num(!cval_eq(a->cell[0], a->cell[1]));
}

cval* builtin_gt(cave_env* e, cval* a){
    return builtin_ord(e, a, ">");
}

cval* builtin_lt(cave_env* e, cval* a){
    return builtin_ord(e, a, "<");
}

cval* builtin_ge(cave_env* e, cval* a){
    return builtin_ord(e, a, ">=");
}

cval* builtin_pe(cave_env* e, cval* a){
    return builtin_ord(e, a, "+=");
}

cval* builtin_var(cave_env* e, cval* a, char* func){
    assert(a, a->type != cval_QEXPR, "Error: to define a function or variable please add inbetween {}");

    cval* syms = a->cell[0];
    for(int i = 0; i < syms->count; i++){
        assert(a, (syms->cell[i]->type == cval_SYM),
      "Function '%s' cannot define non-symbol. "
      "Got %s, Expected %s.", func,
      ctype_name(syms->cell[i]->type),
      ctype_name(cval_SYM));
    }

    assert(a, (syms->count == a->count-1),
    "Function '%s' passed too many arguments for symbols. "
    "Got %i, Expected %i.", func, syms->count, a->count-1);

    for(int i = 0; i < syms->count; i++){
        /* If 'def' define in globally. If 'put' define in locally */
        if (strcmp(func, "def") == 0) {
            cave_env_def(e, syms->cell[i], a->cell[i+1]);
        }
    
        if (strcmp(func, "=")   == 0) {
            cave_env_put(e, syms->cell[i], a->cell[i+1]);
        }
    }
    cval_del(a);
    return cval_sexpr();
}

cval* builtin_put(cave_env* e, cval *a){
    return builtin_var(e, a, "put");
}

//Define
cval* builtin_def(cave_env* e, cval* a) {
    return builtin_var(e, a, "def");
   
}

cval* builtin_lambda(cave_env* e, cval* a){
    assert(a, a->count == 2,
                  "Function 'lambda' needs two arguments");
    assert(a, a->cell[0]->type == cval_QEXPR,
                  "Type needs to be quote expressiomn for lambda function. Not a %s", ctype_name(a->cell[0]->type));

    //check first q expressions contains only symbols
    for (int i = 0; i < a->cell[0]->count; i++) {
        assert(a, (a->cell[0]->cell[i]->type == cval_SYM),
        "Cannot define non-symbol. Got %s, Expected %s.",
        ctype_name(a->cell[0]->cell[i]->type),ctype_name(cval_SYM));
    }

    cval* formals = cval_pop(a, 0);
    cval* body = cval_pop(a, 0);
    cval_del(a);

    return cval_lambda(formals, body);
}

cval* builtin_print(cave_env* e, cval *a){
    //print each argument followed by a spce
    for(int i = 0; i < a->count; i++){
        cval_print(a->cell[i]);
        putchar(' ');
    }
    putchar('\n');
    cval_del(a);

    return cval_sexpr();
}

cval* builtin_error(cave_env* e, cval* a) {
  assert_num("error", a, 1);
  assert_type("error", a, 0, cval_STR);

  /* Construct Error from first argument */
  cval* err = cval_err(a->cell[0]->str);

  /* Delete arguments and return */
  cval_del(a);
  return err;
}

//Loads a file to run
cval* builtin_load(cave_env* e, cval* a) {
    print("Loading file");
    assert_num("load", a, 1);
    assert_type("load", a, 0, cval_STR);

    mpc_result_t r;
    if(mpc_parse_contents(a->cell[0]->str, Lispy, &r)){
        //read the contents
        cval* expr = cval_read(r.output);
        mpc_ast_delete(r.output);

        //while there is more lines to evaluate
        while(expr->count){
            cval* x = cval_eval(e, cval_pop(expr, 0));
            //cval_println(x);
            if(x->type == cval_ERR){
                cval_println(x);
            }
            cval_del(x);
        }

        cval_del(expr);
        cval_del(a);
        return cval_sexpr();
    }else{
        char *err_msg = mpc_err_string(r.error);
        mpc_err_delete(r.error);
        /* Create new error message using it */
        cval* err = cval_err("Could not load file: %s", err_msg);
        free(err_msg);
        cval_del(a);

        /* Cleanup and return error */
        return err;
    }
}

/**
 * ARRAYS:
 * Built in functions to handle ease of use with arrays
 */
cval* builtin_append(cave_env* e, cval* a) {
    printf("builtin_append\n\n");
    assert_num("append", a, 2);
    //Checks that the inputs are of the correct value
    assert_type("append", a, 0, cval_QEXPR);
    pint(1);
    cval* x = cval_add(a->cell[0], a->cell[1]);
    pint(2);
    cave_env_def(e, cval_sym(a->cell[0]->sym), x);
    pint(3);
    //cval_del(a);
    return x;
}


/**
 * add
 */
cval* builtin_insert(cave_env* e, cval* a) {
    assert_num("insert", a, 3);
    //Checks that the inputs are of the correct value
    assert_type("insert", a, 0, cval_QEXPR);
    assert_type("insert", a, 2, cval_NUM);
    cval* x = cval_add_at(a->cell[0], a->cell[1], (int)a->cell[2]->num);
    //cval_del(a);
    return x;
}
