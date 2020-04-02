#ifndef lisp_h

#define lisp_h
#include <stdlib.h>
#include "mpc.h"
#include <editline/readline.h>
#include<stdio.h>


//Parser declaration
mpc_parser_t* Number; 
mpc_parser_t* Symbol; 
mpc_parser_t* String; 
mpc_parser_t* Comment;
mpc_parser_t* Sexpr;  
mpc_parser_t* Qexpr;  
mpc_parser_t* Expr; 
mpc_parser_t* Lispy;

void print(char *s);

void pint(int i);

enum
{
    LVAL_ERR,
    LVAL_NUM,
    LVAL_SYM,
    LVAL_STR,
    LVAL_COM,
    LVAL_FUN,
    LVAL_SEXPR,
    LVAL_QEXPR
};

//forward declarations
struct lenv;
struct lval;
typedef struct lenv lenv;
typedef struct lval lval;

typedef lval *(*lbuiltin)(lenv *, lval *);

struct lval
{
    int type; //Check enum, one of 4 types
    long num; //value if num

    //Error and symbol have a string representation
    char *err;
    char *sym;
    char* str;

    //function
    lbuiltin builtin;
    lenv* env;
    lval* formals;
    lval* body;

    //keeps track of how many lval
    int count;
    struct lval **cell;
};

struct lenv
{
    lenv* par;
    int count;
    char **syms;
    lval **vals;
};

//Function pointer
//lval* output
//new function pointer type: lbuiltin
//lenv* extra pointer to the enviorment

// construct a pointer to a new number lisp value
lval *lval_num(long x);

//lval lval error
lval *lval_err(char* fmt,...);

//lval lval symvol
lval *lval_sym(char *m);

lval* lval_str(char* s);

//lval lval s-expression
lval *lval_sexpr();

//lval lval s-expression
lval *lval_qexpr(void);

//lval function
lval *lval_builtin(lbuiltin func);

//Specifies what each kind is
char* ltype_name(int t);

void lval_del(lval *v);

lval *lval_read_num(mpc_ast_t *t);

lval *lval_read_str(mpc_ast_t *t);

lval *lval_add(lval *v, lval *x);

lval *lval_read(mpc_ast_t *t);

lval* lval_lambda(lval* formals, lval* body);

void lval_expr_print(lval *v, char open, char close);

void lval_print(lval *v);
void lval_println(lval *v);

lval *lval_eval(lenv *e, lval *v);
lval *lval_eval_sexpr(lenv *e, lval *v);
lval *lval_pop(lval *v, int i);
//Similar to pop, but doesn't delete, instead it just copies it over
lval *lval_get(lval *v, int i);

lval *lval_take(lval *v, int i);

lval *lval_join(lval *x, lval *y);

lval *lval_copy(lval *v);

int lval_eq(lval *a, lval *b);

//calls user functions
lval* lval_call(lenv* e, lval* f, lval* a);

//builtin function lookup, uses the built in class, uses correct function
lval *builtin(lenv *e, lval *a, char *func);

lval *builtin_op(lenv *e, lval *a, char *op);

lval *builtin_head(lenv *e, lval *a);

lval *builtin_tail(lenv *e, lval *a);

lval *builtin_list(lenv *e, lval *a);

lval *builtin_eval(lenv *e, lval *a);

lval *builtin_join(lenv *e, lval *a);

lval *builtin_add(lenv *e, lval *a);

lval *builtin_sub(lenv *e, lval *a);

lval *builtin_mul(lenv *e, lval *a);

lval *builtin_div(lenv *e, lval *a);

void lenv_add_builtin(lenv *e, char *name, lbuiltin func);

void lenv_add_builtins(lenv *e);

lval* builtin_lambda(lenv* e, lval* a);

lval* builtin_var(lenv* e, lval* a, char* func);

lval* builtin_def(lenv* e, lval* a);

lval* builtin_put(lenv* e, lval* a);

lval* builtin_if(lenv* e, lval* a);

lval* builtin_for(lenv* e, lval* a);

lval* builtin_while(lenv* e, lval* a);

lval* builtin_eq(lenv* e, lval* a);

lval* builtin_ne(lenv* e, lval* a);

lval* builtin_gt(lenv* e, lval* a);

lval* builtin_lt(lenv* e, lval* a);

lval* builtin_ge(lenv* e, lval* a);

lval* builtin_load(lenv* e, lval* a);

lval* builtin_print(lenv* e, lval* a);

lval* builtin_error(lenv* e, lval* a);

//Enviorment function
lenv *lenv_new(void);

void lenv_del(lenv *e);

lval *lenv_get(lenv *e, lval *k);

void lenv_put(lenv *e, lval *k, lval *v);

char* lenv_get_sym(lenv* e, lval* k);

lenv* lenv_copy(lenv* e);

//Puts in the global parent
void lenv_def(lenv* e, lval* k, lval* v);

//registers all assignment, global or local variables
lval* builtin_var(lenv* e, lval* a, char* func);

#endif