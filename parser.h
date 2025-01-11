#ifdef __cplusplus
extern "C" {
#endif

struct Expression {
    const char* const expr;
    unsigned currIdx;
};

enum ToketType {
    TOKEN_NONE,
    TOKEN_OPEN_PARAN,
    TOKEN_CLOSE_PARAN,
    TOKEN_NUMBER,
    TOKEN_PLUS,
    TOKEN_MINUS,
    TOKEN_MULTIPLY,
    TOKEN_DIVIDE,
};

struct Token_t {
    enum ToketType type;
    unsigned idx;
    double value;
};

enum ResultCode {
    RES_OK,
    RES_INVALID_CHAR,
    RES_INVALID_INPUT,
    RES_CLOSING_PARAN_MISSING,
};

struct ExpressionParsingResult {
    enum ResultCode code;
    unsigned errIdx;
    const char* errMsg;
};

void consumeCharacter(struct Expression* expr);
void consumeWhitespace(struct Expression* expr);
const char* currentHead(struct Expression* expr);
char currentCharacter(struct Expression* expr);
double readNumber(struct Expression* expr);

struct Token_t readToken(struct Expression* expr, struct ExpressionParsingResult* exprParsingResult);
void unreadToken(struct Expression* expr, struct Token_t* token);

// Grammar is a tiny subset of C Programming Language (see K&R 2nd Edition sec. A13 p.238)
// See also Bjarne Stroustrup C++ Programming Language Second Edition sec 6.4 p.189
double evaluatePrimary(struct Expression* expr, struct ExpressionParsingResult* exprParsingResult);
double evaluateTerm(struct Expression* expr, struct ExpressionParsingResult* exprParsingResult);
double evaluateExpression(struct Expression* expr, struct ExpressionParsingResult* result);

#ifdef __cplusplus
}
#endif
