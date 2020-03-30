#include "lisp.h"
//lisp enviorment
/**
* Enviorment
* A way to encode a relationship between names and values
* char** syms, will be an array of size count, keeping track of the names for symbols
* lval **vals is a paralell array. This will keep track of the values inside the enviorment corresponding with each symbol.
*/

lenv *lenv_new(void)
{
    lenv *e = malloc(sizeof(lenv));
    e->par = NULL;
    e->count = 0;
    e->syms = NULL;
    e->vals = NULL;
    return e;
}

void lenv_del(lenv *e)
{
    for (int i = 0; i < e->count; i++)
    {
        free(e->syms[i]);
        lval_del(e->vals[i]);
    }
    free(e->syms);
    free(e->vals);
    free(e);
}

//check if any existing cell matches the enviorments commands, if nothing return an error
lval* lenv_get(lenv* e, lval* k) {

  /* Iterate over all items in environment */
  for (int i = 0; i < e->count; i++) {
    /* Check if the stored string matches the symbol string */
    /* If it does, return a copy of the value */
    if (strcmp(e->syms[i], k->sym) == 0) {
      return lval_copy(e->vals[i]);
    }
  }
  //if parent is not null, then check it for the symbol
  if(e->par){
      return lenv_get(e->par, k);
  }else{//No symbol found in parent
    return lval_err("Unbound Symbol '%s'", k->sym);
  }
}

//Put in k symbol and v value
void lenv_put(lenv *e, lval *k, lval *v)
{
    for (int i = 0; i < e->count; i++)
    {
        if (strcmp(e->syms[i], k->sym) == 0)
        {
            lval_del(e->vals[i]);
            e->vals[i] = lval_copy(v);
            return;
        }
    }
    //No existing entry, allocte space for new entry
    e->count++;
    e->vals = realloc(e->vals, sizeof(lval *) * e->count);
    e->syms = realloc(e->syms, sizeof(char *) * e->count);
    // copy ontents of lval and symbol to new location
    e->vals[e->count - 1] = lval_copy(v);
    e->syms[e->count - 1] = malloc(strlen(k->sym) + 1);
    strcpy(e->syms[e->count - 1], k->sym);
}


//Put into the parent's (root) global enviorment
void lenv_def(lenv* e, lval* k, lval* v){
    //iterate while e has a parent
    while(e->par){
        e = e->par;
    }
    //put value in the global enviorment
    lenv_put(e, k, v);
}

//copy
lenv* lenv_copy(lenv* e){
    lenv* n = malloc(sizeof(lenv));
    n->par = e->par;
    n->count = e->count;
    n->syms = malloc(sizeof(char*) * n->count);
    n->vals = malloc(sizeof(lval*) * n->count);
    for (int i = 0; i < e->count; i++) {
        n->syms[i] = malloc(strlen(e->syms[i]) + 1);
        strcpy(n->syms[i], e->syms[i]);
        n->vals[i] = lval_copy(e->vals[i]);
    }
    return n;
}



void lenv_add_builtin(lenv *e, char *name, lbuiltin func)
{
    lval *k = lval_sym(name);
    lval *v = lval_builtin(func);
    lenv_put(e, k, v);
    lval_del(k);
    lval_del(v);
}

void lenv_add_builtins(lenv *e)
{
    /* List Functions */
    lenv_add_builtin(e, "list", builtin_list);
    lenv_add_builtin(e, "head", builtin_head);
    lenv_add_builtin(e, "tail", builtin_tail);
    lenv_add_builtin(e, "eval", builtin_eval);
    lenv_add_builtin(e, "join", builtin_join);

    /* Mathematical Functions */
    lenv_add_builtin(e, "+", builtin_add);
    lenv_add_builtin(e, "-", builtin_sub);
    lenv_add_builtin(e, "*", builtin_mul);
    lenv_add_builtin(e, "/", builtin_div);

    /* Variable Functions */
    lenv_add_builtin(e, "def",  builtin_def);
    lenv_add_builtin(e, "=",   builtin_put);

    /* Functions */
    lenv_add_builtin(e, "\\", builtin_lambda);

    /* Comparison Functions */
    lenv_add_builtin(e, "if", builtin_if);
    lenv_add_builtin(e, "==", builtin_eq);
    lenv_add_builtin(e, "!=", builtin_ne);
    lenv_add_builtin(e, ">",  builtin_gt);
    lenv_add_builtin(e, "<",  builtin_lt);
    lenv_add_builtin(e, ">=", builtin_ge);

    //string functions
    lenv_add_builtin(e, "load",  builtin_load);
    lenv_add_builtin(e, "error", builtin_error);
    lenv_add_builtin(e, "print", builtin_print);
}