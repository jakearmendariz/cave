#include "cave.h"
//lisp enviorment
/**
* Enviorment
* A way to encode a relationship between names and values
* char** syms, will be an array of size count, keeping track of the names for symbols
* cval **vals is a paralell array. This will keep track of the values inside the enviorment corresponding with each symbol.
*/

cave_env *cave_env_new(void)
{
    cave_env *e = malloc(sizeof(cave_env));
    e->par = NULL;
    e->count = 0;
    e->syms = NULL;
    e->vals = NULL;
    return e;
}

void cave_env_del(cave_env *e)
{
    for (int i = 0; i < e->count; i++)
    {
        free(e->syms[i]);
        cval_del(e->vals[i]);
    }
    free(e->syms);
    free(e->vals);
    free(e);
}

//check if any existing cell matches the enviorments commands, if nothing return an error
cval* cave_env_get(cave_env* e, cval* k) {

  /* Iterate over all items in environment */
  for (int i = 0; i < e->count; i++) {
    /* Check if the stored string matches the symbol string */
    /* If it does, return a copy of the value */
    if (strcmp(e->syms[i], k->sym) == 0) {
      //return cval_copy(e->vals[i]);
      cval *cp = cval_copy(e->vals[i]);
      cp->sym = malloc(sizeof(k->sym)+1);
      cp->num = 1;
      strcpy(cp->sym, k->sym);
      return cp;
      /*
      printf("k->count:%d\n", k->count);
      if(strcmp(k->sym, "while") == 0){
        k->cell[0]->type = cval_QEXPR;
      }
      return x;
      */
    }
  }
  //if parent is not null, then check it for the symbol
  if(e->par){
      return cave_env_get(e->par, k);
  }else{//No symbol found in parent
    return cval_err("Unbound Symbol '%s'", k->sym);
  }
}

char* cave_env_get_sym(cave_env* e, cval* k) {

  /* Iterate over all items in environment */
  for (int i = 0; i < e->count; i++) {
    /* Check if the stored string matches the symbol string */
    /* If it does, return a copy of the value */
    if (cval_eq(e->vals[i], k) == 0) {
      return e->syms[i];
    }
  }
  //if parent is not null, then check it for the symbol
  if(e->par){
      return cave_env_get_sym(e->par, k);
  }else{//No symbol found in parent
    return "Unbound Symbol";
  }
}

//Put in k symbol and v value
void cave_env_put(cave_env *e, cval *k, cval *v)
{
    for (int i = 0; i < e->count; i++)
    {
        if (strcmp(e->syms[i], k->sym) == 0)
        {
            cval_del(e->vals[i]);
            e->vals[i] = cval_eval(e, cval_copy(v));
            return;
        }
    }
    //No existing entry, allocte space for new entry
    e->count++;
    e->vals = realloc(e->vals, sizeof(cval *) * e->count);
    e->syms = realloc(e->syms, sizeof(char *) * e->count);
    // copy ontents of cval and symbol to new location
    e->vals[e->count - 1] = cval_eval(e, cval_copy(v));
    e->syms[e->count - 1] = malloc(strlen(k->sym) + 1);
    strcpy(e->syms[e->count - 1], k->sym);
}


//Put into the parent's (root) global enviorment
void cave_env_def(cave_env* e, cval* k, cval* v){
    //iterate while e has a parent
    while(e->par){
        e = e->par;
    }
    //put value in the global enviorment
    cave_env_put(e, k, v);
}

//copy
cave_env* cave_env_copy(cave_env* e){
    cave_env* n = malloc(sizeof(cave_env));
    n->par = e->par;
    n->count = e->count;
    n->syms = malloc(sizeof(char*) * n->count);
    n->vals = malloc(sizeof(cval*) * n->count);
    for (int i = 0; i < e->count; i++) {
        n->syms[i] = malloc(strlen(e->syms[i]) + 1);
        strcpy(n->syms[i], e->syms[i]);
        n->vals[i] = cval_copy(e->vals[i]);
    }
    return n;
}



void cave_env_add_builtin(cave_env *e, char *name, lbuiltin func)
{
    cval *k = cval_sym(name);
    cval *v = cval_builtin(func);
    cave_env_put(e, k, v);
    cval_del(k);
    cval_del(v);
}

void cave_env_add_builtins(cave_env *e)
{
    /* List Functions */
    cave_env_add_builtin(e, "list", builtin_list);
    cave_env_add_builtin(e, "head", builtin_head);
    cave_env_add_builtin(e, "tail", builtin_tail);
    cave_env_add_builtin(e, "eval", builtin_eval);
    cave_env_add_builtin(e, "join", builtin_join);

    /* Mathematical Functions */
    cave_env_add_builtin(e, "+", builtin_add);
    cave_env_add_builtin(e, "-", builtin_sub);
    cave_env_add_builtin(e, "*", builtin_mul);
    cave_env_add_builtin(e, "/", builtin_div);
    cave_env_add_builtin(e, "^", builtin_pow);

    /* Variable Functions */
    cave_env_add_builtin(e, "def",  builtin_def);
    cave_env_add_builtin(e, "=",   builtin_def);

    /* Functions */
    cave_env_add_builtin(e, "\\", builtin_lambda);

    /* Comparison Functions */
    cave_env_add_builtin(e, "if", builtin_if);
    cave_env_add_builtin(e, "for", builtin_for);
    cave_env_add_builtin(e, "while", builtin_while);
    cave_env_add_builtin(e, "==", builtin_eq);
    cave_env_add_builtin(e, "!=", builtin_ne);
    cave_env_add_builtin(e, ">",  builtin_gt);
    cave_env_add_builtin(e, "<",  builtin_lt);
    cave_env_add_builtin(e, ">=", builtin_ge);
    cave_env_add_builtin(e, "<=", builtin_le);

    //string functions
    cave_env_add_builtin(e, "load",  builtin_load);
    cave_env_add_builtin(e, "error", builtin_error);
    cave_env_add_builtin(e, "print", builtin_print);

    //array functions
    cave_env_add_builtin(e, "append",  builtin_append);
    cave_env_add_builtin(e, "insert", builtin_insert);
}