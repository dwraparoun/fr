#include "newton.h"
#include "parser.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

void newton(const char* expr, double x0, double tol, unsigned itermax)
{
    double deltax = 1e-12;
    int i = 0;
    for (;;) {
        // FIXME use central derivative!
        struct Expression e = initExpressionWithVariable(expr, x0);
        double f0 = evaluateExpression(&e);
        if (e.result != RES_OK) {
            printParsingError(&e);
            exit(EXIT_FAILURE);
        }
        double absErr = fabs(f0);
        if (absErr <= tol) {
            fprintf(stdout, "%s = %f\n", e.var.name, x0);
            exit(EXIT_SUCCESS);
        }
        if (!hasVariable(&e)) {
            // calculator mode
            fprintf(stdout, "%f\n", f0);
            exit(EXIT_SUCCESS);
        }
        // root finding mode
        struct Expression e2 = initExpressionWithVariable(expr, x0 + deltax); //< forward derivative
        double f1 = evaluateExpression(&e2);
        if (e2.result != RES_OK) {
            printParsingError(&e2);
            exit(EXIT_FAILURE);
        }
        double fprime = (f1 - f0) / deltax;
        x0 -= f0 / fprime; // FIXME check div by zero
        ++i;
        if (i == itermax) {
            fprintf(stderr, "Failed to converge after %d iterations.\n",
                itermax); // FIXME write better: |f({x_sym}={guess})| = {curr_tol} <= {tol}");
            exit(EXIT_FAILURE);
        }
    }
}