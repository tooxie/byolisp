#include <stdio.h>
#include <stdlib.h>
#include <editline/readline.h>
#include <histedit.h>
#include "mpc.h"

int main(int argc, char** argv) {
    /* Create some parsers */
    mpc_parser_t* Number   = mpc_new("number");
    mpc_parser_t* Operator = mpc_new("operator");
    mpc_parser_t* Expr     = mpc_new("expr");
    mpc_parser_t* Lispy    = mpc_new("lispy");

    /* Define them in the following language */
    mpca_lang(MPCA_LANG_DEFAULT,
        "                                                       \
            number   : /-?[0-9]+/ ;                             \
            operator : '+' | '-' | '*' | '/' ;                  \
            expr     : <number> | '(' <operator> <expr>+ ')' ;  \
            lispy    : /^/ <operator> <expr>+ /$/ ;             \
        ", Number, Operator, Expr, Lispy);

    /* Print version and exit information */
    puts("Lispy version 0.0.0.0.1");
    puts("Press Ctrl+c to exit\n");

    /* In a never ending loop */
    while(1) {
        /* Print prompt and get input */
        char* input = readline("lispy> ");

        /* Read a line of user input of max size 2048 */
        add_history(input);

        /* Attempt to parse user input */
        mpc_result_t r;
        if(mpc_parse("<stdin>", input, Lispy, &r)) {
            /* On success print the AST */
            mpc_ast_print(r.output);
            mpc_ast_delete(r.output);
        } else {
            mpc_err_print(r.error);
            mpc_err_delete(r.error);
        }

        /* Free retrieved input */
        free(input);
    }

    mpc_cleanup(4, Number, Operator, Expr, Lispy);

    return 0;
}
