#ifdef __cplusplus
extern "C" {
#endif

enum ParsingResult {
    RES_OK,
    RES_INVALID_CHAR,
    RES_INVALID_INPUT,
    RES_OPEN_PARAN_MISSING,
    RES_CLOSE_PARAN_MISSING,
    RES_INTERNAL_ERROR, //< also known as 'bug'
};

struct Expression {
    const char* const expr;
    unsigned currIdx;
    enum ParsingResult result;
    unsigned errIdx;
    const char* errMsg;
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
};

struct Token_t {
    enum TokenType type;
    unsigned idx;
    double value;
};

struct Expression initExpression(const char* expr);
void consumeCharacter(struct Expression* expr);
void consumeWhitespace(struct Expression* expr);
const char* currentHead(struct Expression* expr);
char currentCharacter(struct Expression* expr);
double readNumber(struct Expression* expr);
struct Token_t readToken(struct Expression* expr);
void unreadToken(struct Expression* expr, struct Token_t* token);

// Grammar is a tiny subset of C Programming Language (see K&R 2nd Edition sec. A13 p.238)
// See also Bjarne Stroustrup C++ Programming Language Second Edition sec 6.4 p.189
double evaluatePrimary(struct Expression* expr);
double evaluateTerm(struct Expression* expr);
double evaluateExpression(struct Expression* expr);

#ifdef __cplusplus
}
#endif
