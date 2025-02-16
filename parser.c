#include "parser.h"

#include <ctype.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct Expression createExpression(const char* expr)
{
    struct Expression e;
    e.expr = expr;
    e.currIdx = 0;
    e.result = RES_OK;
    e.errIdx = 0;
    e.errMsg = NULL;
    struct Variable v;
    v.value = 0;
    v.len = 0;
    e.var = v;
    return e;
}

struct Expression createExpressionWithVariable(const char* expr, double varValue)
{
    struct Expression e = createExpression(expr);
    e.var.value = varValue;
    return e;
}

void consumeCharacters(struct Expression* expr, unsigned n) { expr->currIdx += n; }

void consumeCharacter(struct Expression* expr) { consumeCharacters(expr, 1); }

const char* currentHead(struct Expression* expr) { return expr->expr + expr->currIdx; }

char currentCharacter(struct Expression* expr) { return expr->expr[expr->currIdx]; }

void consumeWhitespace(struct Expression* expr)
{
    while (isspace(currentCharacter(expr)))
        consumeCharacter(expr);
}

double readNonNegativeNumber(struct Expression* expr)
{
    // note: number is non-negative!
    consumeWhitespace(expr);
    if (!isdigit(currentCharacter(expr))) {
        expr->result = RES_ERR_INTERNAL;
        expr->errIdx = expr->currIdx;
        expr->errMsg = "Not a number";
        return 0;
    }
    double num = atof(currentHead(expr));
    while (isdigit(currentCharacter(expr)))
        consumeCharacter(expr);
    // consume all digits after '.'
    if (currentCharacter(expr) == '.') {
        consumeCharacter(expr);
        while (isdigit(currentCharacter(expr)))
            consumeCharacter(expr);
    }
    return num;
}

bool hasVariable(struct Expression* expr) { return expr->var.len > 0; }

void readVariable(struct Expression* expr)
{
    if (hasVariable(expr)) {
        if (strncmp(currentHead(expr), expr->var.name, expr->var.len) != 0) {
            expr->result = RES_ERR_MULTIPLE_VARIABLES;
            expr->errIdx = expr->currIdx;
            expr->errMsg = "Multiple variables not allowed";
            return;
        }
        consumeCharacters(expr, expr->var.len);
        return;
    }
    size_t varLen = 0;
    while (isalnum(currentCharacter(expr))) {
        expr->var.name[varLen++] = currentCharacter(expr);
        consumeCharacter(expr);
    }
    if (varLen >= sizeof(expr->var.name)) {
        expr->result = RES_ERR_VAR_TOO_LONG;
        expr->errIdx = expr->currIdx;
        expr->errMsg = "Variable name too long";
        return;
    }
    expr->var.name[varLen] = '\0';
    expr->var.len = varLen;
}

#define RETURN_ON_ERROR(e, r)                                                                      \
    do {                                                                                           \
        if (e->result != RES_OK)                                                                   \
            return r;                                                                              \
    } while (0)

struct Token_t readToken(struct Expression* expr)
{
    consumeWhitespace(expr);
    struct Token_t ret;
    ret.type = TOK_NONE;
    ret.value = 0.;
    ret.idx = expr->currIdx;
    char c = currentCharacter(expr);
    if (strncmp(currentHead(expr), "sin", 3) == 0) {
        ret.type = TOK_SINE;
        consumeCharacters(expr, 3);
    } else if (strncmp(currentHead(expr), "cos", 3) == 0) {
        ret.type = TOK_COSINE;
        consumeCharacters(expr, 3);
    } else if (strncmp(currentHead(expr), "tan", 3) == 0) {
        ret.type = TOK_TAN;
        consumeCharacters(expr, 3);
    } else if (strncmp(currentHead(expr), "atan", 4) == 0) {
        ret.type = TOK_ATAN;
        consumeCharacters(expr, 4);
    } else if (strncmp(currentHead(expr), "exp", 3) == 0) {
        ret.type = TOK_EXP;
        consumeCharacters(expr, 3);
    } else if (strncmp(currentHead(expr), "sqrt", 4) == 0) {
        ret.type = TOK_SQRT;
        consumeCharacters(expr, 4);
        // } else if (strncmp(currentHead(expr), "**", 2) == 0) {
        // ret.type = TOK_POWER;
        // consumeCharacters(expr, 2);
        // ret.value = readNonNegativeNumber(expr);
        // RETURN_ON_ERROR(expr, ret);
    } else if (isalpha(c)) { //< for now only one variable is allowed
        ret.type = TOK_VARIABLE;
        ret.value = expr->var.value;
        // expr->nvars++;
        readVariable(expr);
        RETURN_ON_ERROR(expr, ret);
    } else if (isdigit(c)) {
        ret.type = TOK_NUMBER;
        ret.value = readNonNegativeNumber(expr); //< consumes this and following digit characters
        RETURN_ON_ERROR(expr, ret); //< should never happen
    } else if (c == '(') {
        ret.type = TOK_OPEN_PARAN;
        consumeCharacter(expr);
    } else if (c == ')') {
        ret.type = TOK_CLOSE_PARAN;
        consumeCharacter(expr);
    } else if (c == '+') {
        ret.type = TOK_PLUS;
        consumeCharacter(expr);
    } else if (c == '-') {
        ret.type = TOK_MINUS;
        consumeCharacter(expr);
    } else if (c == '*') {
        ret.type = TOK_MULTIPLY;
        consumeCharacter(expr);
    } else if (c == '/') {
        ret.type = TOK_DIVIDE;
        consumeCharacter(expr);
    } else if (c == '\0') {
    } else {
        expr->result = RES_ERR_INVALID_CHAR;
        expr->errIdx = expr->currIdx;
        expr->errMsg = "Invalid character";
        consumeCharacter(expr);
    }
    return ret;
}

void unreadToken(struct Expression* expr, struct Token_t* token)
{
    expr->currIdx = token->idx; // roll back read pointer
}

double (*getFunction(enum TokenType type))(double)
{
    switch (type) {
    case TOK_SINE:
        return &sin;
    case TOK_COSINE:
        return &cos;
    case TOK_TAN:
        return &tan;
    case TOK_ATAN:
        return &tan;
    case TOK_EXP:
        return &exp;
    case TOK_SQRT:
        return &sqrt;
    default:
        return 0;
    }
}

double evaluatePrimary(struct Expression* expr)
{
    struct Token_t token = readToken(expr);
    RETURN_ON_ERROR(expr, 0);

    if (token.type == TOK_NUMBER)
        return token.value;

    if (token.type == TOK_VARIABLE)
        return token.value;

    if (token.type == TOK_PLUS)
        return evaluatePrimary(expr); // e.g. +42.43 or +++42.43

    if (token.type == TOK_MINUS)
        return -evaluatePrimary(expr); // e.g. -42.43 or ---42.43

    if (token.type == TOK_SINE || token.type == TOK_COSINE || token.type == TOK_TAN
        || token.type == TOK_ATAN || token.type == TOK_EXP || token.type == TOK_SQRT) {
        double (*f)(double) = getFunction(token.type);
        token = readToken(expr);
        RETURN_ON_ERROR(expr, 0);
        if (token.type != TOK_OPEN_PARAN) {
            expr->result = RES_ERR_OPEN_PARAN_MISSING;
            expr->errIdx = expr->currIdx;
            expr->errMsg = "Open parenthesis missing after function";
            return 0;
        }
        double fArg = evaluateExpression(expr);
        RETURN_ON_ERROR(expr, fArg);
        double result = f(fArg);
        token = readToken(expr);
        RETURN_ON_ERROR(expr, result);
        if (token.type != TOK_CLOSE_PARAN) {
            expr->result = RES_ERR_CLOSE_PARAN_MISSING;
            expr->errIdx = expr->currIdx;
            expr->errMsg = "Close parenthesis missing after function";
        }
        return result;
    }

    if (token.type == TOK_OPEN_PARAN) {
        double result = evaluateExpression(expr);
        RETURN_ON_ERROR(expr, result);
        token = readToken(expr);
        RETURN_ON_ERROR(expr, result);
        if (token.type != TOK_CLOSE_PARAN) {
            expr->result = RES_ERR_CLOSE_PARAN_MISSING;
            expr->errIdx = expr->currIdx;
            expr->errMsg = "Close parenthesis missing";
        }
        return result;
    }
    expr->result = RES_ERR_INVALID_INPUT;
    expr->errIdx = token.idx;
    expr->errMsg = "Invalid input";
    return 0;
}

double evaluateTerm(struct Expression* expr)
{
    double left = evaluatePrimary(expr);
    RETURN_ON_ERROR(expr, left);

    struct Token_t token = readToken(expr);
    RETURN_ON_ERROR(expr, left);

    while (token.type != TOK_NONE) {
        switch (token.type) {
        case TOK_MULTIPLY:
            left *= evaluatePrimary(expr);
            RETURN_ON_ERROR(expr, left);
            break;
        case TOK_DIVIDE: {
            double right = evaluatePrimary(expr);
            RETURN_ON_ERROR(expr, left);
            if (right == 0) {
                expr->result = RES_ERR_DIV_BY_ZERO;
                expr->errMsg = "Division by zero";
                expr->errIdx = token.idx;
                return left;
            }
            left /= right;
            break;
        }
        // case TOK_POWER:
        // left = pow(left, token.value);
        // break;
        default:
            unreadToken(expr, &token);
            return left;
        }
        token = readToken(expr);
        RETURN_ON_ERROR(expr, left);
    }
    return left;
}

double evaluateExpression(struct Expression* expr)
{
    double left = evaluateTerm(expr);
    RETURN_ON_ERROR(expr, left);

    struct Token_t token = readToken(expr);
    RETURN_ON_ERROR(expr, left);

    while (token.type != TOK_NONE) {
        switch (token.type) {
        case TOK_PLUS:
            left += evaluateTerm(expr);
            RETURN_ON_ERROR(expr, left);
            break;
        case TOK_MINUS:
            left -= evaluateTerm(expr);
            RETURN_ON_ERROR(expr, left);
            break;
        default:
            unreadToken(expr, &token);
            return left;
        }
        token = readToken(expr);
        RETURN_ON_ERROR(expr, left);
    }
    return left;
}

#undef RETURN_ON_ERROR

void printParsingError(struct Expression* expr)
{
    if (expr->result == RES_OK)
        return;
    fprintf(stderr, "Error: %s\n", expr->errMsg);
    fprintf(stderr, "%s\n", expr->expr);
    unsigned i = 0;
    while (expr->expr[i]) {
        if (expr->errIdx == i)
            fprintf(stderr, "%c", '^');
        else
            fprintf(stderr, "%c", '-');
        i++;
    }
    if (expr->errIdx == i)
        fprintf(stderr, "%c", '^');
    fprintf(stderr, "\n");
}
