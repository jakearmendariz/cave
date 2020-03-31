#include "lisp.h"
// cc -std=c99 -Wall mpc.c lval.c builtin.c env.c main.c -ledit -o run
// ./run
int main(int argc, char **argv)
{
    Number  = mpc_new("number");
    Symbol  = mpc_new("symbol");
    String  = mpc_new("string");
    Comment = mpc_new("comment");
    Sexpr   = mpc_new("sexpr");
    Qexpr   = mpc_new("qexpr");
    Expr    = mpc_new("expr");
    Lispy   = mpc_new("lispy");
    mpca_lang(MPCA_LANG_DEFAULT,
    "                                              \
        number  : /-?[0-9]+/ ;                       \
        symbol  : /[a-zA-Z0-9_+\\-*\\/\\\\=<>!&]+/ ; \
        string  : /\"(\\\\.|[^\"])*\"/ ;             \
        comment : /#[^\\r\\n]*/ ;                    \
        sexpr   : '(' <expr>* ')' ;                  \
        qexpr   : '{' <expr>* '}' ;                  \
        expr    : <number>  | <symbol> | <string>    \
                | <comment> | <sexpr>  | <qexpr>;    \
        lispy   : /^/ <expr>* /$/ ;                  \
    ",
    Number, Symbol, String, Comment, Sexpr, Qexpr, Expr, Lispy);
    puts("Lispy Version 0.0.0.1.4");
    puts("Enter quit to exit\n");
    //Creates environment
    lenv *e = lenv_new();
    print("Debug on:");
    //Adds functions
    lenv_add_builtins(e);

    //Supplied with list of files 
    if (argc >= 2) {

        for (int i = 1; i < argc; i++) {

            lval* args = lval_add(lval_sexpr(), lval_str(argv[i]));

            lval* x = builtin_load(e, args);

            if (x->type == LVAL_ERR) { lval_println(x); }
                lval_del(x);
            }
    }
    else{
        while (1)
        {
            char *input = readline("lispy: ");
            if(strstr(input, ";")){
                int i = strcspn (input,";");
                input[i] = '\0';
            }
            if(strlen(input) == 0){
                continue;
            }

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
    }
    lenv_del(e);
    mpc_cleanup(4, Number, Symbol, String, Comment, Sexpr, Qexpr, Expr, Lispy);
    return 0;
}