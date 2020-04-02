//LispValue.c
#include "lisp.h"


// construct a pointer to a new number lisp value
lval *lval_num(long x)
{
    lval *v = (lval *)malloc(sizeof(lval));
    v->type = LVAL_NUM;
    v->num = x;
    return v;
}

lval *lval_comment(){
    lval *v = (lval *)malloc(sizeof(lval));
    v->type = LVAL_COM;
    return v;
}

//lval lval error
lval *lval_err(char *fmt,...)
{
    lval *v = (lval *)malloc(sizeof(lval));
    v->type = LVAL_ERR;

    va_list va;
    va_start(va, fmt);

    v->err = malloc(512);

    vsnprintf(v->err, 511, fmt, va);

    v->err = realloc(v->err, strlen(v->err)+1);

    va_end(va);
    return v;
}

//lval lval symbol
lval *lval_sym(char *m)
{
    lval *v = (lval *)malloc(sizeof(lval));
    v->type = LVAL_SYM;
    v->sym = (char *)malloc(strlen(m) + 1);
    strcpy(v->sym, m);
    return v;
}

//String
lval* lval_str(char* s){
    lval *v = malloc(sizeof(lval));
    v->type = LVAL_STR;
    v->str = malloc(strlen(s) +1);
    strcpy(v->str, s);
    return v;
}

lval *lval_builtin(lbuiltin func)
{
    lval *v = malloc(sizeof(lval));
    v->type = LVAL_FUN;
    v->builtin = func;
    return v;
}

//lval lval s-expression
lval *lval_sexpr()
{
    lval *v = (lval *)malloc(sizeof(lval));
    v->type = LVAL_SEXPR;
    v->count = 0;
    v->cell = NULL;
    return v;
}

//lval lval s-expression
lval *lval_qexpr(void)
{
    lval *v = (lval *)malloc(sizeof(lval));
    v->type = LVAL_QEXPR;
    v->count = 0;
    v->cell = NULL;
    return v;
}

char* ltype_name(int t) {
  switch(t) {
    case LVAL_FUN: return "Function";
    case LVAL_NUM: return "Number";
    case LVAL_ERR: return "Error";
    case LVAL_SYM: return "Symbol";
    case LVAL_SEXPR: return "S-Expression";
    case LVAL_QEXPR: return "Q-Expression";
    case LVAL_STR: return "String";
    default: return "Unknown";
  }
}


void lval_del(lval *v)
{
    switch (v->type)
    {
    case LVAL_NUM:
        break;
    case LVAL_ERR:
        free(v->err);
        break;
    case LVAL_SYM:
        free(v->sym);
        break;
    case LVAL_QEXPR:
    //continue
    case LVAL_SEXPR:
        for (int i = 0; i < v->count; i++)
        {
            lval_del(v->cell[i]);
        }
        free(v->cell);
        break;
    case LVAL_FUN:
        if(!v->builtin){
            lenv_del(v->env);
            lval_del(v->formals);
            lval_del(v->body);
        }
        break;
    case LVAL_STR:
        free(v->str);
        break;
    }
    free(v);
}

lval *lval_read_num(mpc_ast_t *t)
{
    errno = 0;
    //convert string to a number base 10
    long x = strtol(t->contents, NULL, 10);
    //if errno != ERANGE (theres no error) then its a number, else its
    return errno != ERANGE ? lval_num(x) : lval_err("invalid number");
}

//adds x to the list of cells
lval *lval_add(lval *v, lval *x)
{
    if(x->type == LVAL_COM){
        return v;
    }
    v->count++;
    v->cell = (lval **)realloc(v->cell, sizeof(lval *) * v->count);
    v->cell[v->count - 1] = x;
    return v;
}

lval *lval_read_str(mpc_ast_t *t){
    /* Cut off the final quote character */
    t->contents[strlen(t->contents)-1] = '\0';
    /* Copy the string missing out the first quote character */
    char* unescaped = malloc(strlen(t->contents+1)+1);
    strcpy(unescaped, t->contents+1);
    /* Pass through the unescape function */
    unescaped = mpcf_unescape(unescaped);
    /* Construct a new lval using the string */
    lval* str = lval_str(unescaped);
    /* Free the string and return */
    free(unescaped);
    return str;
}


lval *lval_read(mpc_ast_t *t)
{
    /* If Symbol or Number return conversion to that type */
    if (strstr(t->tag, "number"))
    {
        return lval_read_num(t);
    }
    if (strstr(t->tag, "symbol"))
    {
        //printf("symbol:%s\n", t->contents);
        return lval_sym(t->contents);
        
    }
    if (strstr(t->tag, "string"))
    {
        return lval_read_str(t);
    }
    if (strstr(t->tag, "comment"))
    {
        print("Comment detected in begining");
        //return lval_comment();
    }

    /* If root (>) or sexpr then create empty list */
    lval *x = NULL;
    if (strcmp(t->tag, ">") == 0)
    {
        x = lval_sexpr();
    }
    if (strstr(t->tag, "sexpr"))
    {
        x = lval_sexpr();
    }
    if (strstr(t->tag, "qexpr"))
    {
        x = lval_qexpr();
    }
    
    /* Fill this list with any valid expression contained within */
    for (int i = 0; i < t->children_num; i++)
    {
        if (strcmp(t->children[i]->contents, "(") == 0)
        {
            continue;
        }
        if (strcmp(t->children[i]->contents, ")") == 0)
        {
            continue;
        }
        if (strcmp(t->children[i]->contents, "{") == 0)
        {
            continue;
        }
        if (strcmp(t->children[i]->contents, "}") == 0)
        {
            continue;
        }
        if (strcmp(t->children[i]->tag, "regex") == 0)
        {
            continue;
        }
        if (strstr(t->children[i]->tag, "comment")) 
        { 
            print("comment in read");
            continue; 
        }
        x = lval_add(x, lval_read(t->children[i]));
    }
    return x;
}

void lval_expr_print(lval *v, char open, char close);

void lval_print(lval *v)
{
    switch (v->type)
    {
     case LVAL_COM:
        //printf("printing lisp value:");
        print("print comment");
        break;
    case LVAL_NUM:
        //printf("printing lisp value:");
        printf("%li", v->num);
        break;
    case LVAL_ERR:
        printf("Error: %s", v->err);
        break;
    case LVAL_SYM:
        //printf("printing lisp symbol:");
        printf("%s", v->sym);
        break;
    case LVAL_STR:
        printf("");
        char* esc = malloc(strlen(v->str) + 1);
        strcpy(esc, v->str);
        print(esc);
        esc = (char *)mpcf_unescape(esc);
        printf("\"%s\"", esc);
        free(esc);
        break;
    case LVAL_SEXPR:
        //printf("printing Symbol Expression:");
        lval_expr_print(v, '(', ')');
        break;
    case LVAL_QEXPR:
        //printf("printing Quote Expression:");
        lval_expr_print(v, '{', '}');
        break;
    case LVAL_FUN:
        if (v->builtin) {
            printf("builtin function");
        } else {
            //printf("function:%s\n", v->formals->cell[0]->sym);
            printf("%s\n", lenv_get_sym(v->env, v));
            printf("(\\ "); lval_print(v->formals);
            putchar(' '); lval_print(v->body); putchar(')');
        }
        break;
    }
}
void lval_println(lval *v)
{
    lval_print(v);
    putchar('\n');
}

void lval_expr_print(lval *v, char open, char close)
{
    putchar(open);
    for (int i = 0; i < v->count; i++)
    {
        lval_print(v->cell[i]);

        if (i != v->count - 1)
        {
            putchar(' ');
        }
    }
    putchar(close);
    return;
}

lval *lval_eval(lenv *e, lval *v)
{
    //printf("type in eval:");
    if(v->type == LVAL_COM){
        print("Comment block in eval, assuming comment");
        return v;
    }
    if (v->type == LVAL_SYM)
    {
        //print("symbol in eval");
        lval *x = lenv_get(e, v);
       
        //printf("symbol:%s\n", v->sym);
        lval_del(v);
        //lval_println(v);
        return x;
    }
    // Evaluate S-expressions
    if (v->type == LVAL_SEXPR)
    {
        //If it has any children, evaluate
        print("S-expression");
        return lval_eval_sexpr(e, v);
    }
    //else
    return v;
}

//called inside of eval function
lval *lval_eval_sexpr(lenv *e, lval *v)
{
    //printf("count:");
    for (int i = 0; i < v->count; i++)
    {
        v->cell[i] = lval_eval(e, v->cell[i]);
    }
    for (int i = 0; i < v->count; i++)
    {
        if (v->cell[i]->type == LVAL_ERR)
        {
            return lval_take(v, 0);
        }
    }
    if (v->count == 0)
    {
        print("Comment, do nothing");
        return v;
    }
    else if (v->count == 1)
    {
        return lval_take(v, 0);
    }
    lval *f = lval_pop(v, 0);
    if (f->type != LVAL_FUN)
    {
        lval_del(v);
        lval_del(f);
        return lval_err("First element is not a function");
    }

    //Calls the function
    lval *result = lval_call(e,f, v);
    lval_del(f);
    return result;
}
//int i indicates which cell to pop
//removes the element, moves array over
//returned popped element
lval *lval_pop(lval *v, int i)
{
    lval *x = v->cell[i];

    /* Shift memory after the item at "i" over the top */
    memmove(&v->cell[i], &v->cell[i + 1], sizeof(lval *) * (v->count - i - 1));

    v->count--;

    v->cell = realloc(v->cell, sizeof(lval *) * v->count);
    return x;
}

//Similar to pop, but doesn't delete, instead it just copies it over
lval *lval_get(lval *v, int i){
    return lval_copy(v->cell[i]);
}

//Deletes the list, takes only i out
lval *lval_take(lval *v, int i)
{
    lval *x = lval_pop(v, i);
    lval_del(v);
    return x;
}

//join
lval *lval_join(lval *x, lval *y)
{
    while (y->count)
    {
        x = lval_add(x, lval_pop(y, 0));
    }

    lval_del(y);
    return x;
}

/**
 * copies the lval with each of its cells
 */
lval *lval_copy(lval *v)
{
    lval *x = malloc(sizeof(lval));
    x->type = v->type;

    switch (v->type)
    {
    case LVAL_NUM:
        x->num = v->num;
        break;
    case LVAL_ERR:
        x->err = malloc(sizeof(v->err) + 1);
        strcpy(x->err, v->err);
        break;
    case LVAL_SYM:
        x->sym = malloc(sizeof(v->sym) + 1);
        strcpy(x->sym, v->sym);
        break;
    case LVAL_STR:
        x->str = malloc(sizeof(v->str) + 1);
        strcpy(x->str, v->str);
        break;
    case LVAL_SEXPR:
    //Do the same for q and s expressions
    case LVAL_QEXPR:
        x->count = v->count;
        //Loop through recursivly adding each node, works from root to leaves
        x->cell = malloc(sizeof(lval*) * x->count);
        for (int i = 0; i < x->count; i++)
        {
            x->cell[i] = lval_copy(v->cell[i]);
        }
        break;
    case LVAL_FUN:
        if (v->builtin) {
            //if not null
            x->builtin = v->builtin;
        } else {
            x->builtin = NULL;
            x->env = lenv_copy(v->env);
            x->formals = lval_copy(v->formals);
            x->body = lval_copy(v->body);
        }
        break;
    }
    return x;
}

/**
 * type of function
 * formals are the parameters
 * body is the function body
 */
lval* lval_lambda(lval* formals, lval* body) {
    lval* v = malloc(sizeof(lval));
    v->type = LVAL_FUN;

    v->builtin = NULL;

    v->env = lenv_new();

    v->formals = formals;
    v->body = body;
    return v;
}


//Calls functions
lval* lval_call(lenv* e, lval* f, lval* a){
    //If function is built in
    if(f->builtin){
        print("builtin function - lval_call");
        return f->builtin(e, a);
    }
    print("user-fed function - lval_call");
    int given = a->count;
    int total = f->formals->count;
    
    //while arguments still remain to be processed
    while(a->count){
        if(f->formals->count == 0){
            lval_del(a);
            return lval_err("Function passed too many arguments. Got: %i, expected: %i\n", given, total);
        }

        lval* sym = lval_pop(f->formals, 0);

        //Special case for ','
        if (f->formals->count > 0 && strcmp(sym->sym, ",") == 0) {
            //Ensure & is followed by another symbol
            if(f->formals->count != 1){
                lval_del(a);
                return lval_err("Function format invalid. Symbol ',' not followed by single symbol");
            }
            lval* nsym = lval_pop(f->formals, 0);
            lenv_put(f->env, nsym, builtin_list(e, a));
            lval_del(sym);
            lval_del(nsym);
            break;
        }

        lval* val = lval_pop(a, 0);

        lenv_put(f->env, sym, val);

        lval_del(sym);
        lval_del(val);
    }
    lval_del(a);
    //If all formals have been bound evaluate
    if(f->formals->count == 0){
        f->env->par = e;
        //Evaluate function
        return builtin_eval(f->env, lval_add(lval_sexpr(), lval_copy(f->body)));
    }else{
        return lval_copy(f);
    }
}


int lval_eq(lval *a, lval *b){
    if(a->type != b->type){
        return 0;
    }
    switch(a->type){
        case LVAL_NUM:
            return a->num == b->num;
        case LVAL_STR:
            return (strcmp(a->str,b->str) == 0);
        case LVAL_SYM:
            return  (strcmp(a->sym,b->sym) == 0);
        case LVAL_ERR:
            return (strcmp(a->err,b->err) == 0);
       case LVAL_FUN:
            if(a->builtin || b->builtin){
                return a->builtin == b->builtin;
            }
            return lval_eq(a->formals, b->formals) && lval_eq(a->body, b->body);
        case LVAL_QEXPR:
        case LVAL_SEXPR:
            if(a->count != b->count){
                return 0;
            }
            for(int i = 0; i < a->count; i++){
                if(!lval_eq(a->cell[i], b->cell[i])){
                    return 0;
                }
            }
            return 1;
    }
    return 0;

}