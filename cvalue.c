//LispValue.c
#include "cave.h"


// construct a pointer to a new number lisp value
cval *cval_num(long x)
{
    cval *v = (cval *)malloc(sizeof(cval));
    v->type = cval_NUM;
    v->num = x;
    return v;
}

cval *cval_comment(){
    cval *v = (cval *)malloc(sizeof(cval));
    v->type = cval_COM;
    return v;
}

//cval cval error
cval *cval_err(char *fmt,...)
{
    cval *v = (cval *)malloc(sizeof(cval));
    v->type = cval_ERR;

    va_list va;
    va_start(va, fmt);

    v->err = malloc(512);

    vsnprintf(v->err, 511, fmt, va);

    v->err = realloc(v->err, strlen(v->err)+1);

    va_end(va);
    return v;
}

//cval cval symbol
cval *cval_sym(char *m)
{
    cval *v = (cval *)malloc(sizeof(cval));
    v->type = cval_SYM;
    v->sym = (char *)malloc(strlen(m) + 1);
    strcpy(v->sym, m);
    return v;
}

//String
cval* cval_str(char* s){
    cval *v = malloc(sizeof(cval));
    v->type = cval_STR;
    v->str = malloc(strlen(s) +1);
    strcpy(v->str, s);
    return v;
}

cval *cval_builtin(lbuiltin func)
{
    cval *v = malloc(sizeof(cval));
    v->type = cval_FUN;
    v->builtin = func;
    return v;
}

//cval cval s-expression
cval *cval_sexpr()
{
    cval *v = (cval *)malloc(sizeof(cval));
    v->type = cval_SEXPR;
    v->count = 0;
    v->cell = NULL;
    return v;
}

//cval cval s-expression
cval *cval_qexpr(void)
{
    cval *v = (cval *)malloc(sizeof(cval));
    v->type = cval_QEXPR;
    v->count = 0;
    v->cell = NULL;
    return v;
}

char* ltype_name(int t) {
  switch(t) {
    case cval_FUN: return "Function";
    case cval_NUM: return "Number";
    case cval_ERR: return "Error";
    case cval_SYM: return "Symbol";
    case cval_SEXPR: return "S-Expression";
    case cval_QEXPR: return "Q-Expression";
    case cval_STR: return "String";
    default: return "Unknown";
  }
}


void cval_del(cval *v)
{
    switch (v->type)
    {
    case cval_NUM:
        break;
    case cval_ERR:
        free(v->err);
        break;
    case cval_SYM:
        free(v->sym);
        break;
    case cval_QEXPR:
    //continue
    case cval_SEXPR:
        for (int i = 0; i < v->count; i++)
        {
            cval_del(v->cell[i]);
        }
        free(v->cell);
        break;
    case cval_FUN:
        if(!v->builtin){
            cave_env_del(v->env);
            cval_del(v->formals);
            cval_del(v->body);
        }
        break;
    case cval_STR:
        free(v->str);
        break;
    }
    free(v);
}

cval *cval_read_num(mpc_ast_t *t)
{
    errno = 0;
    //convert string to a number base 10
    long x = strtol(t->contents, NULL, 10);
    //if errno != ERANGE (theres no error) then its a number, else its
    return errno != ERANGE ? cval_num(x) : cval_err("invalid number");
}

//adds x to the list of cells
cval *cval_add(cval *v, cval *x)
{
    if(x->type == cval_COM){
        return v;
    }
    v->count++;
    v->cell = (cval **)realloc(v->cell, sizeof(cval *) * v->count);
    v->cell[v->count - 1] = x;
    return v;
}

cval *cval_read_str(mpc_ast_t *t){
    /* Cut off the final quote character */
    t->contents[strlen(t->contents)-1] = '\0';
    /* Copy the string missing out the first quote character */
    char* unescaped = malloc(strlen(t->contents+1)+1);
    strcpy(unescaped, t->contents+1);
    /* Pass through the unescape function */
    unescaped = mpcf_unescape(unescaped);
    /* Construct a new cval using the string */
    cval* str = cval_str(unescaped);
    /* Free the string and return */
    free(unescaped);
    return str;
}


cval *cval_read(mpc_ast_t *t)
{
    /* If Symbol or Number return conversion to that type */
    if (strstr(t->tag, "number"))
    {
        return cval_read_num(t);
    }
    if (strstr(t->tag, "symbol"))
    {
        //printf("symbol:%s\n", t->contents);
        return cval_sym(t->contents);
        
    }
    if (strstr(t->tag, "string"))
    {
        return cval_read_str(t);
    }
    if (strstr(t->tag, "comment"))
    {
        print("Comment detected in begining");
        //return cval_comment();
    }

    /* If root (>) or sexpr then create empty list */
    cval *x = NULL;
    if (strcmp(t->tag, ">") == 0)
    {
        x = cval_sexpr();
    }
    if (strstr(t->tag, "sexpr"))
    {
        x = cval_sexpr();
    }
    if (strstr(t->tag, "qexpr"))
    {
        x = cval_qexpr();
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
        x = cval_add(x, cval_read(t->children[i]));
    }
    return x;
}

void cval_expr_print(cval *v, char open, char close);

void cval_print(cval *v)
{
    switch (v->type)
    {
     case cval_COM:
        //printf("printing lisp value:");
        print("print comment");
        break;
    case cval_NUM:
        //printf("printing lisp value:");
        printf("%li", v->num);
        break;
    case cval_ERR:
        printf("Error: %s", v->err);
        break;
    case cval_SYM:
        //printf("printing lisp symbol:");
        printf("%s", v->sym);
        break;
    case cval_STR:
        printf("");
        char* esc = malloc(strlen(v->str) + 1);
        strcpy(esc, v->str);
        print(esc);
        esc = (char *)mpcf_unescape(esc);
        printf("\"%s\"", esc);
        free(esc);
        break;
    case cval_SEXPR:
        //printf("printing Symbol Expression:");
        cval_expr_print(v, '(', ')');
        break;
    case cval_QEXPR:
        //printf("printing Quote Expression:");
        cval_expr_print(v, '{', '}');
        break;
    case cval_FUN:
        if (v->builtin) {
            printf("builtin function");
        } else {
            //printf("function:%s\n", v->formals->cell[0]->sym);
            printf("%s\n", cave_env_get_sym(v->env, v));
            printf("(\\ "); cval_print(v->formals);
            putchar(' '); cval_print(v->body); putchar(')');
        }
        break;
    }
}
void cval_println(cval *v)
{
    cval_print(v);
    putchar('\n');
}

void cval_expr_print(cval *v, char open, char close)
{
    putchar(open);
    for (int i = 0; i < v->count; i++)
    {
        cval_print(v->cell[i]);

        if (i != v->count - 1)
        {
            putchar(' ');
        }
    }
    putchar(close);
    return;
}

cval *cval_eval(cave_env *e, cval *v)
{
    //printf("type in eval:");
    if(v->type == cval_COM){
        print("Comment block in eval, assuming comment");
        return v;
    }
    if (v->type == cval_SYM)
    {
        //print("symbol in eval");
        cval *x = cave_env_get(e, v);
       
        //printf("symbol:%s\n", v->sym);
        cval_del(v);
        //cval_println(v);
        return x;
    }
    // Evaluate S-expressions
    if (v->type == cval_SEXPR)
    {
        //If it has any children, evaluate
        print("S-expression");
        return cval_eval_sexpr(e, v);
    }
    //else
    return v;
}

//called inside of eval function
cval *cval_eval_sexpr(cave_env *e, cval *v)
{
    //printf("count:");
    for (int i = 0; i < v->count; i++)
    {
        v->cell[i] = cval_eval(e, v->cell[i]);
    }
    for (int i = 0; i < v->count; i++)
    {
        if (v->cell[i]->type == cval_ERR)
        {
            return cval_take(v, 0);
        }
    }
    if (v->count == 0)
    {
        print("Comment, do nothing");
        return v;
    }
    else if (v->count == 1)
    {
        return cval_take(v, 0);
    }
    cval *f = cval_pop(v, 0);
    if (f->type != cval_FUN)
    {
        cval_del(v);
        cval_del(f);
        return cval_err("First element is not a function");
    }

    //Calls the function
    cval *result = cval_call(e,f, v);
    cval_del(f);
    return result;
}
//int i indicates which cell to pop
//removes the element, moves array over
//returned popped element
cval *cval_pop(cval *v, int i)
{
    cval *x = v->cell[i];

    /* Shift memory after the item at "i" over the top */
    memmove(&v->cell[i], &v->cell[i + 1], sizeof(cval *) * (v->count - i - 1));

    v->count--;

    v->cell = realloc(v->cell, sizeof(cval *) * v->count);
    return x;
}

//Similar to pop, but doesn't delete, instead it just copies it over
cval *cval_get(cval *v, int i){
    return cval_copy(v->cell[i]);
}

//Deletes the list, takes only i out
cval *cval_take(cval *v, int i)
{
    cval *x = cval_pop(v, i);
    cval_del(v);
    return x;
}

//join
cval *cval_join(cval *x, cval *y)
{
    while (y->count)
    {
        x = cval_add(x, cval_pop(y, 0));
    }

    cval_del(y);
    return x;
}

/**
 * copies the cval with each of its cells
 */
cval *cval_copy(cval *v)
{
    cval *x = malloc(sizeof(cval));
    x->type = v->type;

    switch (v->type)
    {
    case cval_NUM:
        x->num = v->num;
        break;
    case cval_ERR:
        x->err = malloc(sizeof(v->err) + 1);
        strcpy(x->err, v->err);
        break;
    case cval_SYM:
        x->sym = malloc(sizeof(v->sym) + 1);
        strcpy(x->sym, v->sym);
        break;
    case cval_STR:
        x->str = malloc(sizeof(v->str) + 1);
        strcpy(x->str, v->str);
        break;
    case cval_SEXPR:
    //Do the same for q and s expressions
    case cval_QEXPR:
        x->count = v->count;
        //Loop through recursivly adding each node, works from root to leaves
        x->cell = malloc(sizeof(cval*) * x->count);
        for (int i = 0; i < x->count; i++)
        {
            x->cell[i] = cval_copy(v->cell[i]);
        }
        break;
    case cval_FUN:
        if (v->builtin) {
            //if not null
            x->builtin = v->builtin;
        } else {
            x->builtin = NULL;
            x->env = cave_env_copy(v->env);
            x->formals = cval_copy(v->formals);
            x->body = cval_copy(v->body);
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
cval* cval_lambda(cval* formals, cval* body) {
    cval* v = malloc(sizeof(cval));
    v->type = cval_FUN;

    v->builtin = NULL;

    v->env = cave_env_new();

    v->formals = formals;
    v->body = body;
    return v;
}


//Calls functions
cval* cval_call(cave_env* e, cval* f, cval* a){
    //If function is built in
    if(f->builtin){
        print("builtin function - cval_call");
        return f->builtin(e, a);
    }
    print("user-fed function - cval_call");
    int given = a->count;
    int total = f->formals->count;
    
    //while arguments still remain to be processed
    while(a->count){
        if(f->formals->count == 0){
            cval_del(a);
            return cval_err("Function passed too many arguments. Got: %i, expected: %i\n", given, total);
        }

        cval* sym = cval_pop(f->formals, 0);

        //Special case for ','
        if (f->formals->count > 0 && strcmp(sym->sym, ",") == 0) {
            //Ensure & is followed by another symbol
            if(f->formals->count != 1){
                cval_del(a);
                return cval_err("Function format invalid. Symbol ',' not followed by single symbol");
            }
            cval* nsym = cval_pop(f->formals, 0);
            cave_env_put(f->env, nsym, builtin_list(e, a));
            cval_del(sym);
            cval_del(nsym);
            break;
        }

        cval* val = cval_pop(a, 0);

        cave_env_put(f->env, sym, val);

        cval_del(sym);
        cval_del(val);
    }
    cval_del(a);
    //If all formals have been bound evaluate
    if(f->formals->count == 0){
        f->env->par = e;
        //Evaluate function
        return builtin_eval(f->env, cval_add(cval_sexpr(), cval_copy(f->body)));
    }else{
        return cval_copy(f);
    }
}


int cval_eq(cval *a, cval *b){
    if(a->type != b->type){
        return 0;
    }
    switch(a->type){
        case cval_NUM:
            return a->num == b->num;
        case cval_STR:
            return (strcmp(a->str,b->str) == 0);
        case cval_SYM:
            return  (strcmp(a->sym,b->sym) == 0);
        case cval_ERR:
            return (strcmp(a->err,b->err) == 0);
       case cval_FUN:
            if(a->builtin || b->builtin){
                return a->builtin == b->builtin;
            }
            return cval_eq(a->formals, b->formals) && cval_eq(a->body, b->body);
        case cval_QEXPR:
        case cval_SEXPR:
            if(a->count != b->count){
                return 0;
            }
            for(int i = 0; i < a->count; i++){
                if(!cval_eq(a->cell[i], b->cell[i])){
                    return 0;
                }
            }
            return 1;
    }
    return 0;

}