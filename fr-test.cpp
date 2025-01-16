#include "parser.h"
#include <catch2/catch.hpp>
#include <math.h>

TEST_CASE("Token reader")
{
    SECTION("TOK_OPEN_PARAN")
    {
        Expression expr = initExpression("(");
        CHECK(readToken(&expr).type == TOK_OPEN_PARAN);
    }
    SECTION("TOK_CLOSE_PARAN")
    {
        Expression expr = initExpression(")");
        CHECK(readToken(&expr).type == TOK_CLOSE_PARAN);
    }
    SECTION("TOK_NUMBER")
    {
        Expression expr = initExpression("42.56");
        struct Token_t tok = readToken(&expr);
        CHECK(tok.type == TOK_NUMBER);
        CHECK(tok.value == 42.56);
    }
    SECTION("TOK_PLUS")
    {
        Expression expr = initExpression("+");
        CHECK(readToken(&expr).type == TOK_PLUS);
    }
    SECTION("TOK_MINUS")
    {
        Expression expr = initExpression("-");
        CHECK(readToken(&expr).type == TOK_MINUS);
    }
    SECTION("TOK_MULTIPLY")
    {
        Expression expr = initExpression("*");
        CHECK(readToken(&expr).type == TOK_MULTIPLY);
    }
    SECTION("TOK_DIVIDE")
    {
        Expression expr = initExpression("/");
        CHECK(readToken(&expr).type == TOK_DIVIDE);
    }
    SECTION("TOK_SINE")
    {
        Expression expr = initExpression("sin");
        CHECK(readToken(&expr).type == TOK_SINE);
    }
}

TEST_CASE("Invalid expressions return correct error results", "[parser]")
{
    SECTION("Invalid character")
    {
        Expression expr = initExpression("a");
        evaluateExpression(&expr);
        CHECK(expr.result == RES_INVALID_CHAR);
    }
    SECTION("Valid integer plus an invalid character")
    {
        Expression expr = initExpression("3 + a");
        evaluateExpression(&expr);
        CHECK(expr.result == RES_INVALID_CHAR);
    }
    SECTION("Valid integer plus a valid integer with an invalid character behind")
    {
        Expression expr = initExpression("3 + 3a");
        evaluateExpression(&expr);
        CHECK(expr.result == RES_INVALID_CHAR);
    }
}

TEST_CASE("Addition of two integer operands", "[parser]")
{
    Expression expr = { "553+3", 0, RES_OK, 0, "" };
    SECTION("Check overall consistency")
    {
        CHECK(readNumber(&expr) == 553);
        CHECK(expr.currIdx == 3);
        CHECK(currentCharacter(&expr) == '+');
        CHECK(*currentHead(&expr) == '+');
    }
    SECTION("Check expression evaluation")
    {
        CHECK(evaluateExpression(&expr) == 556);
    }
}

TEST_CASE("Addition of two negative integers", "[parser]")
{
    Expression expr = { "-553 + -3", 0, RES_OK, 0, "" };
    SECTION("Check expression evaluation")
    {
        CHECK(evaluateExpression(&expr) == -556);
    }
}

TEST_CASE("Multiplication of two negative integers", "[parser]")
{
    Expression expr = { "-3 * -2", 0, RES_OK, 0, "" };
    SECTION("Check expression evaluation")
    {
        CHECK(evaluateExpression(&expr) == 6);
    }
}

TEST_CASE("Addition of two floating-point operands", "[parser]")
{
    Expression expr = { "553.2+3.4", 0, RES_OK, 0, "" };
    SECTION("Check overall consistency")
    {
        CHECK(readNumber(&expr) == 553.2);
        CHECK(currentCharacter(&expr) == '+');
        CHECK(*currentHead(&expr) == '+');
    }
    SECTION("Check expression evaluation")
    {
        CHECK(evaluateExpression(&expr) == 556.6);
    }
}

TEST_CASE("Addition of two integer operands with whitespace", "[parser]")
{
    Expression expr = { "    553   +   3     ", 0, RES_OK, 0, "" };
    SECTION("Check expression evaluation")
    {
        CHECK(evaluateExpression(&expr) == 556);
    }
}

TEST_CASE("Addition of two integer operands with whitespace and brackets", "[parser]")
{
    Expression expr = { "  (  553   +   3  )   ", 0, RES_OK, 0, "" };
    SECTION("Check expression evaluation")
    {
        CHECK(evaluateExpression(&expr) == 556);
    }
}

TEST_CASE("Addition of three integer operands", "[parser]")
{
    Expression expr = { "553+3+20", 0, RES_OK, 0, "" };
    SECTION("Check expression evaluation")
    {
        CHECK(evaluateExpression(&expr) == 576);
    }
}

TEST_CASE("Addition of three integer operands with whitespace", "[parser]")
{
    Expression expr = { "    553   +   3     +   20  ", 0, RES_OK, 0, "" };
    SECTION("Check expression evaluation")
    {
        CHECK(evaluateExpression(&expr) == 576);
    }
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
        CHECK(readNumber(&expr) == 5);
        CHECK(expr.currIdx == 1);
        CHECK(currentCharacter(&expr) == '*');
    }
    SECTION("Check expression evaluation")
    {
        CHECK(evaluateExpression(&expr) == 30);
    }
}

TEST_CASE("Multiplication of two floating point operands", "[parser]")
{
    Expression expr = { "2.5*2.5", 0, RES_OK, 0, "" };
    SECTION("Check expression evaluation")
    {
        CHECK(evaluateExpression(&expr) == 6.25);
    }
}

TEST_CASE("Multiplication precedence", "[parser]")
{
    Expression expr = { "2*3+4", 0, RES_OK, 0, "" };
    SECTION("Check expression evaluation")
    {
        CHECK(evaluateExpression(&expr) == 10);
    }
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
