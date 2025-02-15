#include "newton.h"
#include "parser.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

double newton(const char* expr, double x0, double tol)
{
    double deltax = 1e-12;
    unsigned itermax = 50;
    int i = 0;
    for (;;) {
        // FIXME use central derivative!
        struct Expression e = initExpression(expr);
        e.x0 = x0;
        double f0 = evaluateExpression(&e);
        if (e.result != RES_OK) {
            printParsingError(&e);
            exit(EXIT_FAILURE);
        }
        struct Expression e2 = initExpression(expr);
        e2.x0 = x0 + deltax; //< forward derivative
        double f1 = evaluateExpression(&e2);
        if (e2.result != RES_OK) {
            printParsingError(&e2);
            exit(EXIT_FAILURE);
        }
        double fprime = (f1 - f0) / deltax;
        x0 -= f0 / fprime; // FIXME check div by zero
        double err = fabs(f0);
        if (err <= tol)
            break;
        ++i;
        if (i == itermax) {
            fprintf(stderr, "Failed to converge after %d iterations.\n",
                itermax); // FIXME write better: |f({x_sym}={guess})| = {curr_tol} <= {tol}");
            exit(EXIT_FAILURE);
        }
    }
    return x0;
}