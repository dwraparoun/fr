#include "parser.h"
#include <getopt.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

static const char* usage()
{
    return "Usage: fr [--tol <tol>] [--x0 <x0>] [--maxiter <maxiter>] [--deltax <deltax>] <expr>";
}

static void parseArgs(int argc, char* const* argv, double* argTol, double* argX0, double* argDeltax,
    unsigned* argNiter)
{
    if (argc < 2) {
        fprintf(stderr, "%s\n", usage());
        exit(EXIT_FAILURE);
    }
    const struct option long_options[] = { { "x0", required_argument, 0, 'a' },
        { "tol", required_argument, 0, 'b' }, { "maxiter", required_argument, 0, 'c' },
        { "deltax", required_argument, 0, 'd' }, { 0, 0, 0, 0 } };
    for (;;) {
        int option_index = 0;
        int c = getopt_long(argc, argv, "a:b:", long_options, &option_index);
        if (c == -1)
            break; //< all options have been parsed
        switch (c) {
        case 'a':
            *argX0 = atof(optarg);
            break;
        case 'b':
            *argTol = atof(optarg);
            if (*argTol <= 0) {
                fprintf(stderr, "--tol must be >0. Got %f\n", *argTol);
                exit(EXIT_FAILURE);
            }
            break;
        case 'c':
            *argNiter = atol(optarg);
            if (*argNiter <= 0) {
                fprintf(stderr, "--maxiter must be >0. Got %d\n", *argNiter);
                exit(EXIT_FAILURE);
            }
            break;
        case 'd':
            *argDeltax = atof(optarg);
            if (*argDeltax <= 0) {
                fprintf(stderr, "--deltax must be >0. Got %f\n", *argDeltax);
                exit(EXIT_FAILURE);
            }
            break;
        default:
            exit(EXIT_FAILURE); // getopt printed error already
        }
    }
    if ((argc - optind) == 0) {
        fprintf(stderr, "Missing expression. %s\n", usage());
        exit(EXIT_FAILURE);
    }
    if ((argc - optind) > 1) {
        fprintf(stderr, "Too many arguments: ");
        while (optind < argc)
            fprintf(stderr, "'%s' ", argv[optind++]);
        fprintf(stderr, "%s\n", usage());
        exit(EXIT_FAILURE);
    }
}

static void newton(const char* expr, double x0, double tol, double deltax, unsigned maxiter)
{
    for (int i = 0;; i++) {
        struct Expression e = createExpressionWithVariable(expr, x0);
        double f = evaluateExpression(&e);
        if (e.result != RES_OK) {
            printParsingError(&e);
            exit(EXIT_FAILURE);
        }
        if (!hasVariable(&e)) {
            // program used as a calculator
            fprintf(stdout, "%f\n", f);
            exit(EXIT_SUCCESS);
        }
        // program used as a root finder
        double absErr = fabs(f);
        if (absErr <= tol) {
            fprintf(stdout, "%s = %f\n", e.var.name, x0);
            exit(EXIT_SUCCESS);
        }
        if (i == maxiter) {
            fprintf(stderr, "Failed to converge after %d iterations. |f(%s=%f)| = %f > %f\n",
                maxiter, e.var.name, x0, absErr, tol);
            exit(EXIT_FAILURE);
        }
        // compute central derivative
        struct Expression e2 = createExpressionWithVariable(expr, x0 + deltax);
        struct Expression e1 = createExpressionWithVariable(expr, x0 - deltax);
        double f2 = evaluateExpression(&e2);
        if (e2.result != RES_OK) {
            printParsingError(&e2);
            exit(EXIT_FAILURE);
        }
        double f1 = evaluateExpression(&e1);
        if (e1.result != RES_OK) {
            printParsingError(&e1);
            exit(EXIT_FAILURE);
        }
        double fprime = (f2 - f1) / (2. * deltax);
        if (fprime == 0) {
            fprintf(stderr,
                "Newton algorithm resulted in division by zero: f(%s=%f)=%f - f(%s=%f)=%f = 0\n",
                e.var.name, x0 + deltax, f2, e.var.name, x0 - deltax, f1);
            exit(EXIT_FAILURE);
        }
        x0 -= f / fprime;
    }
}

int main(int argc, char* const* argv)
{
    double tol = 1e-5;
    double deltax = 1e-5; // for derivative calculation
    double x0 = 0.;
    unsigned maxiter = 50;
    parseArgs(argc, argv, &tol, &x0, &deltax, &maxiter);
    newton(argv[optind], x0, tol, deltax, maxiter);
    return EXIT_SUCCESS;
}
