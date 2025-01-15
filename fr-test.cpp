#include "parser.h"
#include <catch2/catch.hpp>

TEST_CASE("Invalid expressions return correct error codes", "[parser]")
{
    struct ExpressionParsingResult res = { RES_OK, 0, "" };
    SECTION("Invalid character")
    {
        Expression expr = { "a", 0 };
        evaluateExpression(&expr, &res);
        CHECK(res.code == RES_INVALID_CHAR);
    }
    SECTION("Valid integer plus an invalid character")
    {
        Expression expr = { "3 + a", 0 };
        evaluateExpression(&expr, &res);
        CHECK(res.code == RES_INVALID_CHAR);
    }
    SECTION("Valid integer plus a valid integer with an invalid character behind")
    {
        Expression expr = { "3 + 3a", 0 };
        evaluateExpression(&expr, &res);
        CHECK(res.code == RES_INVALID_CHAR);
    }
}

TEST_CASE("Addition of two integer operands", "[parser]")
{
    Expression expr = { "553+3", 0 };
    SECTION("Check overall consistency")
    {
        CHECK(readNumber(&expr) == 553);
        CHECK(expr.currIdx == 3);
        CHECK(currentCharacter(&expr) == '+');
        CHECK(*currentHead(&expr) == '+');
        CHECK(readNumber(&expr) == 3);
        CHECK(*(currentHead(&expr) - 1) == '3');
    }
    SECTION("Check expression evaluation")
    {
        CHECK(evaluateExpression(&expr, 0) == 556);
    }
}

TEST_CASE("Addition of two negative integers", "[parser]")
{
    Expression expr = { "-553 + -3", 0 };
    SECTION("Check expression evaluation")
    {
        CHECK(evaluateExpression(&expr, 0) == -556);
    }
}

TEST_CASE("Multiplication of two negative integers", "[parser]")
{
    Expression expr = { "-3 * -2", 0 };
    SECTION("Check expression evaluation")
    {
        CHECK(evaluateExpression(&expr, 0) == 6);
    }
}

TEST_CASE("Addition of two floating-point operands", "[parser]")
{
    Expression expr = { "553.2+3.4", 0 };
    SECTION("Check overall consistency")
    {
        CHECK(readNumber(&expr) == 553.2);
        CHECK(currentCharacter(&expr) == '+');
        CHECK(*currentHead(&expr) == '+');
        CHECK(readNumber(&expr) == 3.4);
    }
    SECTION("Check expression evaluation")
    {
        CHECK(evaluateExpression(&expr, 0) == 556.6);
    }
}

TEST_CASE("Addition of two integer operands with whitespace", "[parser]")
{
    Expression expr = { "    553   +   3     ", 0 };
    SECTION("Check expression evaluation")
    {
        CHECK(evaluateExpression(&expr, 0) == 556);
    }
}

TEST_CASE("Addition of two integer operands with whitespace and brackets", "[parser]")
{
    Expression expr = { "  (  553   +   3  )   ", 0 };
    SECTION("Check expression evaluation")
    {
        CHECK(evaluateExpression(&expr, 0) == 556);
    }
}

TEST_CASE("Addition of three integer operands", "[parser]")
{
    Expression expr = { "553+3+20", 0 };
    SECTION("Check expression evaluation")
    {
        CHECK(evaluateExpression(&expr, 0) == 576);
    }
}

TEST_CASE("Addition of three integer operands with whitespace", "[parser]")
{
    Expression expr = { "    553   +   3     +   20  ", 0 };
    SECTION("Check expression evaluation")
    {
        CHECK(evaluateExpression(&expr, 0) == 576);
    }
}

TEST_CASE("Addition of three integer operands with whitespace and brackets", "[parser]")
{
    Expression expr = { "    553   +  ( 3     +   20 )    ", 0 };
    CHECK(evaluateExpression(&expr, 0) == 576);
}

TEST_CASE("Addition of multiple integer operands with whitespace and brackets", "[parser]")
{
    Expression expr = { "    553   +  ( 3     +   20 )    +(7+    3)", 0 };
    CHECK(evaluateExpression(&expr, 0) == 586);
}

TEST_CASE("Multiplication of two integer operands", "[parser]")
{
    Expression expr = { "5*6", 0 };
    SECTION("Check overall consistency")
    {
        CHECK(readNumber(&expr) == 5);
        CHECK(expr.currIdx == 1);
        CHECK(currentCharacter(&expr) == '*');
    }
    SECTION("Check expression evaluation")
    {
        CHECK(evaluateExpression(&expr, 0) == 30);
    }
}

TEST_CASE("Multiplication of two floating point operands", "[parser]")
{
    Expression expr = { "2.5*2.5", 0 };
    SECTION("Check expression evaluation")
    {
        CHECK(evaluateExpression(&expr, 0) == 6.25);
    }
}

TEST_CASE("Multiplication precedence", "[parser]")
{
    Expression expr = { "2*3+4", 0 };
    SECTION("Check expression evaluation")
    {
        CHECK(evaluateExpression(&expr, 0) == 10);
    }
}
