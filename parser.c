#include "parser.h"
#include <ctype.h>
#include <stdlib.h>

struct Expression initExpression(const char* expr)
{
    struct Expression e = { expr, 0, RES_OK, 0, "" };
    return e;
}

void consumeCharacter(struct Expression* expr) { expr->currIdx++; }

const char* currentHead(struct Expression* expr)
{
    return expr->expr + expr->currIdx;
}

char currentCharacter(struct Expression* expr)
{
    return expr->expr[expr->currIdx];
}

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

#define RETURN_ON_ERROR(e, r)    \
    do {                         \
        if (e->result != RES_OK) \
            return r;            \
    } while (0)

struct Token_t readToken(struct Expression* expr)
{
    consumeWhitespace(expr);
    struct Token_t ret;
    ret.type = TOK_NONE;
    ret.value = 0.;
    ret.idx = expr->currIdx;
    switch (currentCharacter(expr)) {
    case '(':
        ret.type = TOK_OPEN_PARAN;
        consumeCharacter(expr);
        break;
    case ')':
        ret.type = TOK_CLOSE_PARAN;
        consumeCharacter(expr);
        break;
    case '0':
    case '1':
    case '2':
    case '3':
    case '4':
    case '5':
    case '6':
    case '7':
    case '8':
    case '9':
        ret.type = TOK_NUMBER;
        ret.value = readNumber(expr); //< consumes this and following digit characters
        RETURN_ON_ERROR(expr, ret); //< should never happen
        break;
    case '+':
        ret.type = TOK_PLUS;
        consumeCharacter(expr);
        break;
    case '-':
        ret.type = TOK_MINUS;
        consumeCharacter(expr);
        break;
    case '*':
        ret.type = TOK_MULTIPLY;
        consumeCharacter(expr);
        break;
    case '/':
        ret.type = TOK_DIVIDE;
        consumeCharacter(expr);
        break;
    case '\0':
        break;
    default:
        expr->result = RES_INVALID_CHAR;
        expr->errIdx = expr->currIdx;
        expr->errMsg = "Invalid character";
        consumeCharacter(expr);
        break;
    }
    return ret;
}

void unreadToken(struct Expression* expr, struct Token_t* token)
{
    expr->currIdx = token->idx; // roll back read pointer
}

double evaluatePrimary(struct Expression* expr)
{
    struct Token_t token = readToken(expr);
    RETURN_ON_ERROR(expr, 0);

    if (token.type == TOK_NUMBER)
        return token.value;

    if (token.type == TOK_PLUS)
        return evaluatePrimary(expr); // e.g. +42.43 or +++42.43

    if (token.type == TOK_MINUS)
        return -evaluatePrimary(expr); // e.g. -42.43 or ---42.43

    if (token.type == TOK_OPEN_PARAN) {
        double result = evaluateExpression(expr);
        RETURN_ON_ERROR(expr, result);
        token = readToken(expr);
        if (token.type != TOK_CLOSE_PARAN) {
            expr->result = RES_CLOSING_PARAN_MISSING;
            expr->errIdx = expr->currIdx;
            expr->errMsg = "Missing closing brace";
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
            left /= evaluatePrimary(expr);
            RETURN_ON_ERROR(expr, left);
            break;
        default:
            unreadToken(expr, &token);
            return left;
        }
        token = readToken(expr);
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
    }
    return left;
}

#undef RETURN_ON_ERROR
