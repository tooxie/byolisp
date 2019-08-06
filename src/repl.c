#include <stdio.h>
#include <stdlib.h>
#include <editline/readline.h>
#include <histedit.h>
#include "mpc.h"

/* Create enum of possible lval types */
enum type{ LVAL_NUM, LVAL_ERR };

/* Create enum of possible error types */
enum error{ LERR_DIV_ZERO, LERR_BAD_OP, LERR_BAD_NUM };

/* Define new lval struct */
typedef struct {
    enum type type;
    long num;
    enum error err;
} lval;

/* Create a new number type lval */
lval lval_num(long x) {
    lval v;
    v.type = LVAL_NUM;
    v.num = x;

    return v;
}

/* Create a new error type lval */
lval lval_err(int x) {
    lval v;
    v.type = LVAL_ERR;
    v.err = x;

    return v;
}

void lval_print(lval v) {
    switch (v.type) {
        /* In the case the type is a number print it */
        /* then `break` out of the switch */
        case LVAL_NUM: printf("%li", v.num); break;

        /* In the case the type is an error */
        case LVAL_ERR:
            /* Check what type of error it is and print it */
            if (v.err == LERR_DIV_ZERO) {
                printf("Error: Division by zero");
            }
            if (v.err == LERR_BAD_OP) {
                printf("Error: Invalid operator");
            }
            if (v.err == LERR_BAD_NUM) {
                printf("Error: Invalid number");
            }
        break;
    }
}

/* Print an lval followed by a newline */
void lval_println(lval v) { lval_print(v); putchar('\n'); }

lval eval_op(lval x, char* op, lval y) {
    /* If either value is an error, return it */
    if (x.type == LVAL_ERR) { return x; }
    if (y.type == LVAL_ERR) { return y; }

    /* Otherwise do maths on the number values */
    if (strcmp(op, "+") == 0) { return lval_num(x.num + y.num); }
    if (strcmp(op, "-") == 0) { return lval_num(x.num - y.num); }
    if (strcmp(op, "*") == 0) { return lval_num(x.num * y.num); }
    if (strcmp(op, "/") == 0) {
        /* If the second operand is zero, return an error */
        return y.num == 0
            ? lval_err(LERR_DIV_ZERO)
            : lval_num(x.num / y.num);
    }

    return lval_err(LERR_BAD_OP);
}

lval eval(mpc_ast_t* t) {
    /* If tagged as number return it directly */
    if (strstr(t->tag, "number")) {
        /* Check if there's some error in conversion */
        errno = 0;
        long x = strtol(t->contents, NULL, 10);
        return errno != ERANGE ? lval_num(x) : lval_err(LERR_BAD_NUM);
    }

    /* The operator is always second child */
    char* op = t->children[1]->contents;

    /* We store the third child in `x` */
    lval x = eval(t->children[2]);

    /* Iterate the remaining children and combining */
    int i = 3;
    while (strstr(t->children[i]->tag, "expr")) {
        x = eval_op(x, op, eval(t->children[i]));
        i++;
    }

    return x;
}

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

            lval result = eval(r.output);
            lval_println(result);
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

int number_of_nodes(mpc_ast_t* t) {
    if (t->children_num == 0) { return 1; }
    if (t->children_num >= 1) {
        int total = 1;
        for (int i = 0; i < t->children_num; i++) {
            total = total + number_of_nodes(t->children[i]);
        }

        return total;
    }

    return 0;
}
