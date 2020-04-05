#ifndef cave_h

#define cave_h
#include <stdlib.h>
#include "mpc.h"
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
    cval_ERR,
    cval_NUM,
    cval_SYM,
    cval_STR,
    cval_COM,
    cval_FUN,
    cval_SEXPR,
    cval_QEXPR
};

//forward declarations
struct cave_env;
struct cval;
typedef struct cave_env cave_env;
typedef struct cval cval;

typedef cval *(*lbuiltin)(cave_env *, cval *);

struct cval
{
    int type; //Check enum, one of 4 types
    long num; //value if num

    //Error and symbol have a string representation
    char *err;
    char *sym;
    char* str;

    //function
    lbuiltin builtin;
    cave_env* env;
    cval* formals;
    cval* body;

    //keeps track of how many cval
    int count;
    struct cval **cell;
};

struct cave_env
{
    cave_env* par;
    int count;
    char **syms;
    cval **vals;
};

//Function pointer
//cval* output
//new function pointer type: lbuiltin
//cave_env* extra pointer to the enviorment

// construct a pointer to a new number lisp value
cval *cval_num(long x);

//cval cval error
cval *cval_err(char* fmt,...);

//cval cval symvol
cval *cval_sym(char *m);

cval* cval_str(char* s);

//cval cval s-expression
cval *cval_sexpr();

//cval cval s-expression
cval *cval_qexpr(void);

//cval function
cval *cval_builtin(lbuiltin func);

//Specifies what each kind is
char* ctype_name(int t);

void cval_del(cval *v);

cval *cval_read_num(mpc_ast_t *t);

cval *cval_read_str(mpc_ast_t *t);

cval *cval_add(cval *v, cval *x);

//Adds at a specific index
cval *cval_add_at(cval *v, cval *x, int index);

cval *cval_read(mpc_ast_t *t);

cval* cval_lambda(cval* formals, cval* body);

void cval_expr_print(cval *v, char open, char close);

void cval_print(cval *v);
void cval_println(cval *v);

cval *cval_eval(cave_env *e, cval *v);
cval *cval_eval_sexpr(cave_env *e, cval *v);
cval *cval_pop(cval *v, int i);
//Similar to pop, but doesn't delete, instead it just copies it over
cval *cval_get(cval *v, int i);

cval *cval_take(cval *v, int i);

cval *cval_join(cval *x, cval *y);

cval *cval_copy(cval *v);

int cval_eq(cval *a, cval *b);

//calls user functions
cval* cval_call(cave_env* e, cval* f, cval* a);

//builtin function lookup, uses the built in class, uses correct function
cval *builtin(cave_env *e, cval *a, char *func);

cval *builtin_op(cave_env *e, cval *a, char *op);

cval *builtin_head(cave_env *e, cval *a);

cval *builtin_tail(cave_env *e, cval *a);

cval *builtin_list(cave_env *e, cval *a);

cval *builtin_eval(cave_env *e, cval *a);

cval *builtin_join(cave_env *e, cval *a);

cval *builtin_add(cave_env *e, cval *a);

cval *builtin_sub(cave_env *e, cval *a);

cval *builtin_mul(cave_env *e, cval *a);

cval *builtin_div(cave_env *e, cval *a);

void cave_env_add_builtin(cave_env *e, char *name, lbuiltin func);

void cave_env_add_builtins(cave_env *e);

cval* builtin_lambda(cave_env* e, cval* a);

cval* builtin_var(cave_env* e, cval* a, char* func);

cval* builtin_def(cave_env* e, cval* a);

cval* builtin_put(cave_env* e, cval* a);

cval* builtin_if(cave_env* e, cval* a);

cval* builtin_for(cave_env* e, cval* a);

cval* builtin_while(cave_env* e, cval* a);

cval* builtin_eq(cave_env* e, cval* a);

cval* builtin_ne(cave_env* e, cval* a);

cval* builtin_gt(cave_env* e, cval* a);

cval* builtin_lt(cave_env* e, cval* a);

cval* builtin_ge(cave_env* e, cval* a);

cval* builtin_load(cave_env* e, cval* a);

cval* builtin_print(cave_env* e, cval* a);

cval* builtin_error(cave_env* e, cval* a);

cval* builtin_insert(cave_env* e, cval* a);

cval* builtin_append(cave_env* e, cval* a);


//Enviorment function
cave_env *cave_env_new(void);

void cave_env_del(cave_env *e);

cval *cave_env_get(cave_env *e, cval *k);

void cave_env_put(cave_env *e, cval *k, cval *v);

char* cave_env_get_sym(cave_env* e, cval* k);

cave_env* cave_env_copy(cave_env* e);

//Puts in the global parent
void cave_env_def(cave_env* e, cval* k, cval* v);

//registers all assignment, global or local variables
cval* builtin_var(cave_env* e, cval* a, char* func);

#endif