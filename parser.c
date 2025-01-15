#include "parser.h"
#include <ctype.h>
#include <stdlib.h>

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
    if (!isdigit(currentCharacter(expr)))
        __builtin_unreachable();
    consumeWhitespace(expr);
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

struct Token_t readToken(struct Expression* expr,
    struct ExpressionParsingResult* exprParsingResult)
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
        ret.value = readNumber(expr);
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
        if (exprParsingResult) {
            exprParsingResult->code = RES_INVALID_CHAR;
            exprParsingResult->errIdx = expr->currIdx;
            exprParsingResult->errMsg = "Invalid character";
        }
        consumeCharacter(expr);
        break;
    }
    return ret;
}

void unreadToken(struct Expression* expr, struct Token_t* token)
{
    expr->currIdx = token->idx;
}

double evaluatePrimary(struct Expression* expr,
    struct ExpressionParsingResult* exprParsingResult)
{
    struct Token_t token = readToken(expr, exprParsingResult);
    if (exprParsingResult && exprParsingResult->code != RES_OK)
        return 0;
    if (token.type == TOK_NUMBER)
        return token.value;
    if (token.type == TOK_PLUS)
        return evaluatePrimary(expr, exprParsingResult);
    if (token.type == TOK_MINUS)
        return -evaluatePrimary(expr, exprParsingResult);
    if (token.type == TOK_OPEN_PARAN) {
        double result = evaluateExpression(expr, exprParsingResult);
        if (exprParsingResult && exprParsingResult->code != RES_OK)
            return result;
        token = readToken(expr, exprParsingResult);
        if (token.type != TOK_CLOSE_PARAN) {
            if (exprParsingResult) {
                exprParsingResult->code = RES_CLOSING_PARAN_MISSING;
                exprParsingResult->errIdx = expr->currIdx;
                exprParsingResult->errMsg = "Missing closing brace";
            }
        }
        return result;
    }
    if (exprParsingResult) {
        exprParsingResult->code = RES_INVALID_INPUT;
        exprParsingResult->errIdx = token.idx;
        exprParsingResult->errMsg = "Invalid input";
    }
    return 0;
}

double evaluateTerm(struct Expression* expr,
    struct ExpressionParsingResult* exprParsingResult)
{
    double left = evaluatePrimary(expr, exprParsingResult);
    if (exprParsingResult && exprParsingResult->code != RES_OK)
        return left;

    struct Token_t token = readToken(expr, exprParsingResult);
    if (exprParsingResult && exprParsingResult->code != RES_OK)
        return left;

    while (token.type != TOK_NONE) {
        switch (token.type) {
        case TOK_MULTIPLY:
            left *= evaluatePrimary(expr, exprParsingResult);
            if (exprParsingResult && exprParsingResult->code != RES_OK)
                return left;
            break;
        case TOK_DIVIDE:
            left /= evaluatePrimary(expr, exprParsingResult);
            if (exprParsingResult && exprParsingResult->code != RES_OK)
                return left;
            break;
        default:
            unreadToken(expr, &token);
            return left;
        }
        token = readToken(expr, exprParsingResult);
    }
    return left;
}

double evaluateExpression(struct Expression* expr,
    struct ExpressionParsingResult* exprParsingResult)
{
    double left = evaluateTerm(expr, exprParsingResult);
    if (exprParsingResult && exprParsingResult->code != RES_OK)
        return left;

    struct Token_t token = readToken(expr, exprParsingResult);
    if (exprParsingResult && exprParsingResult->code != RES_OK)
        return left;

    while (token.type != TOK_NONE) {
        switch (token.type) {
        case TOK_PLUS:
            left += evaluateTerm(expr, exprParsingResult);
            if (exprParsingResult && exprParsingResult->code != RES_OK)
                return left;
            break;
        case TOK_MINUS:
            left -= evaluateTerm(expr, exprParsingResult);
            if (exprParsingResult && exprParsingResult->code != RES_OK)
                return left;
            break;
        default:
            unreadToken(expr, &token);
            return left;
        }
        token = readToken(expr, exprParsingResult);
    }
    return left;
}
