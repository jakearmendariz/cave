#include "lisp.h"
//cc -std=c99 -Wall mpc.c lval.c builtin.c env.c main.c -ledit -o run
//./run
int main(int argc, char **argv)
{
    mpc_parser_t *Number = mpc_new("number");
    mpc_parser_t *Symbol = mpc_new("symbol");
    mpc_parser_t *Sexpr = mpc_new("sexpr");
    mpc_parser_t *Qexpr = mpc_new("qexpr");
    mpc_parser_t *Expr = mpc_new("expr");
    mpc_parser_t *Lispy = mpc_new("lispy");

    mpca_lang(MPCA_LANG_DEFAULT,
              "                                         \
    number : /-?[0-9]+/ ;                               \
    symbol : /[a-zA-Z0-9_+\\-*\\/\\\\=<>!&]+/ ;         \
    sexpr  : '(' <expr>* ')' ;                          \
    qexpr  : '{' <expr>* '}' ;                          \
    expr   : <number> | <symbol> | <sexpr> | <qexpr> ;  \
    lispy  : /^/ <expr>* /$/ ;                          \
  ",
              Number, Symbol, Sexpr, Qexpr, Expr, Lispy);
    puts("Lispy Version 0.0.0.0.8");
    puts("Enter quit to exit\n");
    //Creates environment
    lenv *e = lenv_new();
    print("created environment");
    //Adds functions
    lenv_add_builtins(e);
    print("added builtins");
    while (1)
    {
        char *input = readline("lispy: ");

        add_history(input);
        if (strcmp(input, "quit") == 0)
        {
            printf("quiting program\n");
            break;
        }

        /* Attempt to Parse the user Input */
        mpc_result_t r;
        if (mpc_parse("<stdin>", input, Lispy, &r))
        {
            lval *x = lval_eval(e, lval_read(r.output));
            lval_println(x);
            lval_del(x);
        }
        else
        {
            /* Otherwise Print the Error */
            mpc_err_print(r.error);
            mpc_err_delete(r.error);
        }
        free(input);
    }
    lenv_del(e);
    mpc_cleanup(4, Number, Symbol, Sexpr, Qexpr, Expr, Lispy);
    return 0;
}