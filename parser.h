#ifndef PARSER_H
#define PARSER_H

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

enum ParsingResult {
    RES_OK,
    RES_ERR_INVALID_CHAR,
    RES_ERR_INVALID_INPUT,
    RES_ERR_DIV_BY_ZERO,
    RES_ERR_OPEN_PARAN_MISSING,
    RES_ERR_CLOSE_PARAN_MISSING,
    RES_ERR_VAR_TOO_LONG,
    RES_ERR_MULTIPLE_VARIABLES,
    RES_ERR_INTERNAL, //< euphemism for 'bug' =)
};

struct Variable {
    char name[32];
    unsigned len; //< excluding '\0', e.g. variable 'a\0' has len=1
    double value; //< if parser finds a variable, this value is assigned
};

struct Expression {
    const char* expr;
    unsigned currIdx;
    enum ParsingResult result;
    unsigned errIdx;
    const char* errMsg;
    struct Variable var;
};

enum TokenType {
    TOK_NONE,
    TOK_OPEN_PARAN,
    TOK_CLOSE_PARAN,
    TOK_NUMBER,
    TOK_PLUS,
    TOK_MINUS,
    TOK_MULTIPLY,
    TOK_DIVIDE,
    TOK_SINE,
    TOK_COSINE,
    TOK_TAN,
    TOK_ATAN,
    TOK_EXP,
    TOK_SQRT,
    TOK_POWER,
    TOK_VARIABLE,
};

struct Token_t {
    enum TokenType type;
    unsigned idx;
    double value;
};

struct Expression createExpression(const char* expr); //< varValue is zero
struct Expression createExpressionWithVariable(const char* expr, double varValue);
void consumeCharacter(struct Expression* expr);
void consumeWhitespace(struct Expression* expr);
const char* currentHead(struct Expression* expr);
char currentCharacter(struct Expression* expr);
double readNonNegativeNumber(struct Expression* expr);
void readVariable(struct Expression* expr);
bool hasVariable(struct Expression* expr);
struct Token_t readToken(struct Expression* expr);
void unreadToken(struct Expression* expr, struct Token_t* token);

// Grammar is a tiny subset of C Programming Language (see K&R 2nd Edition sec. A13 p.238)
// See also Bjarne Stroustrup C++ Programming Language Second Edition sec 6.4 p.189
double evaluatePrimary(struct Expression* expr);
double evaluateTerm(struct Expression* expr);
double evaluateExpression(struct Expression* expr);
void printParsingError(struct Expression* expr);

#ifdef __cplusplus
}
#endif

#endif
