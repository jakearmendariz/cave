#include "cave.h"
// cc -std=c99 -Wall mpc.c cval.c builtin.c env.c main.c -ledit -o run
// ./run
#ifdef _WIN32

static char buffer[2048];

char* readline(char* prompt) {
  fputs(prompt, stdout);
  fgets(buffer, 2048, stdin);
  char* cpy = malloc(strlen(buffer)+1);
  strcpy(cpy, buffer);
  cpy[strlen(cpy)-1] = '\0';
  return cpy;
}

void add_history(char* unused) {}

#elif __linux__     //linux/ubuntu
#include <readline/readline.h>
#include <readline/history.h>
#else               //macos
#include <editline/readline.h>
#endif
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
        symbol  : /[a-zA-Z0-9_+\\-*\\/\\\\=<>!,&]+/ ; \
        string  : /\"(\\\\.|[^\"])*\"/ ;             \
        comment : /#[^\\r\\n]*/ ;                    \
        sexpr   : '(' <expr>* ')' ;                  \
        qexpr   : '{' <expr>* '}' ;                  \
        expr    : <number>  | <symbol> | <string>    \
                | <comment> | <sexpr>  | <qexpr>;    \
        lispy   : /^/ <expr>* /$/ ;                  \
    ",
    Number, Symbol, String, Comment, Sexpr, Qexpr, Expr, Lispy);
    puts("Cave: The Programming Language");
    puts("Enter quit to exit\n");
    //Creates environment
    cave_env *e = cave_env_new();
    print("Debug on:");
    //Adds functions
    cave_env_add_builtins(e);
    builtin_load(e, cval_str("\"hello.a\""));

    //Supplied with list of files 
    if (argc >= 2) {

        for (int i = 1; i < argc; i++) {

            cval* args = cval_add(cval_sexpr(), cval_str(argv[i]));

            cval* x = builtin_load(e, args);

            if (x->type == cval_ERR) { cval_println(x); }
                cval_del(x);
            }
    }
    else{
        while (1)
        {
            char *input = readline("code: ");
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
                printf("Goodbye :)\n");
                break;
            }

            /* Attempt to Parse the user Input */
            mpc_result_t r;
            if (mpc_parse("<stdin>", input, Lispy, &r))
            {
                cval *x = cval_eval(e, cval_read(r.output));
                cval_println(x);
                if(x->type == cval_ERR){
                    //cval_println(x);
                }
                cval_del(x);
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
    cave_env_del(e);
    mpc_cleanup(4, Number, Symbol, String, Comment, Sexpr, Qexpr, Expr, Lispy);
    return 0;
}