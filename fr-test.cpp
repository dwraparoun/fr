#include "parser.h"
#include <catch2/catch.hpp>
#include <math.h>

#define CHECK_TOK(expr, tok)                                                                       \
    do {                                                                                           \
        SECTION(#expr)                                                                             \
        {                                                                                          \
            Expression e = createExpression(expr);                                                 \
            CHECK(readToken(&e).type == tok);                                                      \
        }                                                                                          \
    } while (0)

TEST_CASE("Token reader", "[parser]")
{
    CHECK_TOK("", TOK_NONE);
    CHECK_TOK("(", TOK_OPEN_PARAN);
    CHECK_TOK(")", TOK_CLOSE_PARAN);
    CHECK_TOK("33.34", TOK_NUMBER);
    CHECK_TOK("+", TOK_PLUS);
    CHECK_TOK("-", TOK_MINUS);
    CHECK_TOK("*", TOK_MULTIPLY);
    CHECK_TOK("/", TOK_DIVIDE);
    CHECK_TOK("sin", TOK_SINE);
    CHECK_TOK("cos", TOK_COSINE);
    CHECK_TOK("tan", TOK_TAN);
    CHECK_TOK("atan", TOK_ATAN);
    CHECK_TOK("exp", TOK_EXP);
    CHECK_TOK("sqrt", TOK_SQRT);
    // CHECK_TOK("", TOK_POWER);
    CHECK_TOK("myVariable", TOK_VARIABLE);
}
#undef CHECK_TOK

#define CHECK_ERR(expr, errcode)                                                                   \
    do {                                                                                           \
        SECTION(#expr)                                                                             \
        {                                                                                          \
            Expression e = createExpression(expr);                                                 \
            evaluateExpression(&e);                                                                \
            CHECK(e.result == errcode);                                                            \
        }                                                                                          \
    } while (0)
TEST_CASE("Invalid expressions return correct error results", "[parser]")
{
    CHECK_ERR(":", RES_ERR_INVALID_CHAR);
    CHECK_ERR("", RES_ERR_INVALID_INPUT);
    CHECK_ERR("sin5)", RES_ERR_OPEN_PARAN_MISSING);
    CHECK_ERR("sin(5", RES_ERR_CLOSE_PARAN_MISSING);
    CHECK_ERR("aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa", RES_ERR_VAR_TOO_LONG); //< sizeof(Variable.name)
    CHECK_ERR("a+b", RES_ERR_MULTIPLE_VARIABLES);
}
#undef CHECK_ERR

TEST_CASE("Addition of two integer operands", "[parser]")
{
    Expression expr = { "553+3", 0, RES_OK, 0, "" };
    SECTION("Check overall consistency")
    {
        CHECK(readNonNegativeNumber(&expr) == 553);
        CHECK(expr.currIdx == 3);
        CHECK(currentCharacter(&expr) == '+');
        CHECK(*currentHead(&expr) == '+');
    }
    SECTION("Check expression evaluation") { CHECK(evaluateExpression(&expr) == 556); }
}

TEST_CASE("Addition of two negative integers", "[parser]")
{
    Expression expr = { "-553 + -3", 0, RES_OK, 0, "" };
    SECTION("Check expression evaluation") { CHECK(evaluateExpression(&expr) == -556); }
}

TEST_CASE("Multiplication of two negative integers", "[parser]")
{
    Expression expr = { "-3 * -2", 0, RES_OK, 0, "" };
    SECTION("Check expression evaluation") { CHECK(evaluateExpression(&expr) == 6); }
}

TEST_CASE("Addition of two floating-point operands", "[parser]")
{
    Expression expr = { "553.2+3.4", 0, RES_OK, 0, "" };
    SECTION("Check overall consistency")
    {
        CHECK(readNonNegativeNumber(&expr) == 553.2);
        CHECK(currentCharacter(&expr) == '+');
        CHECK(*currentHead(&expr) == '+');
    }
    SECTION("Check expression evaluation") { CHECK(evaluateExpression(&expr) == 556.6); }
}

TEST_CASE("Addition of two integer operands with whitespace", "[parser]")
{
    Expression expr = { "    553   +   3     ", 0, RES_OK, 0, "" };
    SECTION("Check expression evaluation") { CHECK(evaluateExpression(&expr) == 556); }
}

TEST_CASE("Addition of two integer operands with whitespace and brackets", "[parser]")
{
    Expression expr = { "  (  553   +   3  )   ", 0, RES_OK, 0, "" };
    SECTION("Check expression evaluation") { CHECK(evaluateExpression(&expr) == 556); }
}

TEST_CASE("Addition of three integer operands", "[parser]")
{
    Expression expr = { "553+3+20", 0, RES_OK, 0, "" };
    SECTION("Check expression evaluation") { CHECK(evaluateExpression(&expr) == 576); }
}

TEST_CASE("Addition of three integer operands with whitespace", "[parser]")
{
    Expression expr = { "    553   +   3     +   20  ", 0, RES_OK, 0, "" };
    SECTION("Check expression evaluation") { CHECK(evaluateExpression(&expr) == 576); }
}

TEST_CASE("Addition of three integer operands with whitespace and brackets", "[parser]")
{
    Expression expr = { "    553   +  ( 3     +   20 )    ", 0, RES_OK, 0, "" };
    CHECK(evaluateExpression(&expr) == 576);
}

TEST_CASE("Addition of multiple integer operands with whitespace and brackets", "[parser]")
{
    Expression expr = { "    553   +  ( 3     +   20 )    +(7+    3)", 0, RES_OK, 0, "" };
    CHECK(evaluateExpression(&expr) == 586);
}

TEST_CASE("Multiplication of two integer operands", "[parser]")
{
    Expression expr = { "5*6", 0, RES_OK, 0, "" };
    SECTION("Check overall consistency")
    {
        CHECK(readNonNegativeNumber(&expr) == 5);
        CHECK(expr.currIdx == 1);
        CHECK(currentCharacter(&expr) == '*');
    }
    SECTION("Check expression evaluation") { CHECK(evaluateExpression(&expr) == 30); }
}

TEST_CASE("Multiplication of two floating point operands", "[parser]")
{
    Expression expr = { "2.5*2.5", 0, RES_OK, 0, "" };
    SECTION("Check expression evaluation") { CHECK(evaluateExpression(&expr) == 6.25); }
}

TEST_CASE("Multiplication precedence", "[parser]")
{
    Expression expr = { "2*3+4", 0, RES_OK, 0, "" };
    SECTION("Check expression evaluation") { CHECK(evaluateExpression(&expr) == 10); }
}

TEST_CASE("Math functions", "[parser]")
{
    SECTION("sin")
    {
        Expression expr = { "sin(3.14159/2)", 0, RES_OK, 0, "" };
        CHECK(round(evaluateExpression(&expr)) == 1.0);
    }
    SECTION("cos")
    {
        Expression expr = { "cos(0)", 0, RES_OK, 0, "" };
        CHECK(round(evaluateExpression(&expr)) == 1.0);
    }
    SECTION("tan")
    {
        Expression expr = { "tan(0)", 0, RES_OK, 0, "" };
        CHECK(round(evaluateExpression(&expr)) == 0.0);
    }
    SECTION("atan")
    {
        Expression expr = { "atan(0)", 0, RES_OK, 0, "" };
        CHECK(round(evaluateExpression(&expr)) == 0.0);
    }
    SECTION("exp")
    {
        Expression expr = { "exp(0)", 0, RES_OK, 0, "" };
        CHECK(round(evaluateExpression(&expr)) == 1.0);
    }
}
