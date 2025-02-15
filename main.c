#include "newton.h"
#include "parser.h"
#include <getopt.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

const char* usage() { return "Usage: fr [--tol] [--x0] <expr>"; }

void parseArgs(int argc, char* const* argv, double* argTol, double* argX0)
{
    if (argc < 2) {
        fprintf(stderr, "%s\n", usage());
        exit(EXIT_FAILURE);
    }
    const struct option long_options[] = { { "x0", required_argument, 0, 'a' },
        { "tol", required_argument, 0, 'b' }, { 0, 0, 0, 0 } };
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
            if (argTol <= 0) {
                fprintf(stderr, "--tol must be >0. Got %f\n", *argTol);
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

int main(int argc, char* const* argv)
{
    double tol = 1e-6;
    double x0 = 0.;
    parseArgs(argc, argv, &tol, &x0);
    double result = newton(argv[optind], x0, tol);
    fprintf(stdout, "%f\n", result);
    return EXIT_SUCCESS;
}
