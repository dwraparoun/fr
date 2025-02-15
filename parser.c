#include "parser.h"

#include <ctype.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct Expression initExpression(const char* expr)
{
    struct Expression e = { expr, 0, RES_OK, 0, "", 0, 0 };
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

double readNumber(struct Expression* expr)
{
    consumeWhitespace(expr);
    if (!isdigit(currentCharacter(expr))) {
        // should be unreachable
        expr->result = RES_INTERNAL_ERROR;
        expr->errIdx = expr->currIdx;
        expr->errMsg = "Internal error";
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

void readVariable(struct Expression* expr)
{
    consumeWhitespace(expr);
    while (isalnum(currentCharacter(expr)))
        consumeCharacter(expr);
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
    } else if (strncmp(currentHead(expr), "atan", 3) == 0) {
        ret.type = TOK_ATAN;
        consumeCharacters(expr, 4);
    } else if (strncmp(currentHead(expr), "exp", 3) == 0) {
        ret.type = TOK_EXP;
        consumeCharacters(expr, 3);
    } else if (isalpha(c) && expr->nvars == 0) { //< for now only one variable is allowed
        // FIXME variable names can't start with sin/cos/tan/atan/exp :(
        ret.type = TOK_VARIABLE;
        expr->nvars++;
        readVariable(expr);
    } else if (isdigit(c)) {
        ret.type = TOK_NUMBER;
        ret.value = readNumber(expr); //< consumes this and following digit characters
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
        expr->result = RES_INVALID_CHAR;
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
        return expr->x0;

    if (token.type == TOK_PLUS)
        return evaluatePrimary(expr); // e.g. +42.43 or +++42.43

    if (token.type == TOK_MINUS)
        return -evaluatePrimary(expr); // e.g. -42.43 or ---42.43

    if (token.type == TOK_SINE || token.type == TOK_COSINE || token.type == TOK_TAN
        || token.type == TOK_ATAN || token.type == TOK_EXP) {
        double (*f)(double) = getFunction(token.type);
        token = readToken(expr);
        RETURN_ON_ERROR(expr, 0);
        if (token.type != TOK_OPEN_PARAN) {
            expr->result = RES_OPEN_PARAN_MISSING;
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
            expr->result = RES_CLOSE_PARAN_MISSING;
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
            expr->result = RES_CLOSE_PARAN_MISSING;
            expr->errIdx = expr->currIdx;
            expr->errMsg = "Close parenthesis missing";
        }
        return result;
    }
    expr->result = RES_INVALID_INPUT;
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
        case TOK_DIVIDE:
            // FIXME check zero!
            left /= evaluatePrimary(expr);
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
