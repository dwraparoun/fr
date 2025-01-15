#include "parser.h"
#include <stdio.h>
#include <stdlib.h>

int main(int argc, const char* const* const argv)
{
    if (argc != 2 || argv[1][0] == '\0') {
        fprintf(stderr, "Usage: %s <expr>\n", argv[0]);
        return EXIT_FAILURE;
    }
    struct Expression expr = initExpression(argv[1]);
    double result = evaluateExpression(&expr);
    if (expr.result == RES_OK)
        fprintf(stdout, "%f\n", result);
    else {
        fprintf(stderr, "Error: %s\n", expr.errMsg);
        fprintf(stderr, "%s\n", expr.expr);
        unsigned i = 0;
        while (expr.expr[i]) {
            if (expr.errIdx == i)
                fprintf(stderr, "%c", '^');
            else
                fprintf(stderr, "%c", '-');
            i++;
        }
        if (expr.errIdx == i)
            fprintf(stderr, "%c", '^');
        fprintf(stderr, "\n");
    }

    return EXIT_SUCCESS;
}
