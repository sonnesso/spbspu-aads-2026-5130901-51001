#include <boost/test/unit_test.hpp>

#include <sstream>
#include <stdexcept>
#include <string>

#include "calculator.hpp"

using sadovnik::Calculator;
using sadovnik::Value;

BOOST_AUTO_TEST_CASE(addition_and_subtraction)
{
  Calculator calc;
  BOOST_TEST(calc.evaluate("2 + 3") == 5);
  BOOST_TEST(calc.evaluate("100 - 85") == 15);
}

BOOST_AUTO_TEST_CASE(multiplication_and_division)
{
  Calculator calc;
  BOOST_TEST(calc.evaluate("3 * 4") == 12);
  BOOST_TEST(calc.evaluate("10 / 2") == 5);
}

BOOST_AUTO_TEST_CASE(modulo_operation)
{
  Calculator calc;
  BOOST_TEST(calc.evaluate("10 % 3") == 1);
  BOOST_TEST(calc.evaluate("7 % 3") == 1);
}

BOOST_AUTO_TEST_CASE(bitwise_or_operation)
{
  Calculator calc;
  BOOST_TEST(calc.evaluate("1 | 2") == 3);
  BOOST_TEST(calc.evaluate("5 | 3") == 7);
  BOOST_TEST(calc.evaluate("0 | 5") == 5);
  BOOST_TEST(calc.evaluate("7 | 0") == 7);
}

BOOST_AUTO_TEST_CASE(bitwise_or_from_string_stream)
{
  Calculator calc;
  std::ostringstream expression;
  expression << 12 << " | " << 3;
  BOOST_TEST(calc.evaluate(expression.str()) == 15);
}

BOOST_AUTO_TEST_CASE(parentheses_change_order)
{
  Calculator calc;
  BOOST_TEST(calc.evaluate("( 1 + 2 ) * 3") == 9);
  BOOST_TEST(calc.evaluate("( 2 + 3 ) * ( 4 + 5 )") == 45);
  BOOST_TEST(calc.evaluate("( 10 / 2 ) + 3") == 8);
}

BOOST_AUTO_TEST_CASE(operator_precedence)
{
  Calculator calc;
  BOOST_TEST(calc.evaluate("1 + 2 * 3") == 7);
  BOOST_TEST(calc.evaluate("10 - 4 / 2") == 8);
  BOOST_TEST(calc.evaluate("2 + 2 * 2") == 6);
}

BOOST_AUTO_TEST_CASE(bitwise_or_precedence)
{
  Calculator calc;
  BOOST_TEST(calc.evaluate("1 + 2 | 3") == 3);
  BOOST_TEST(calc.evaluate("2 | 3 * 4") == 14);
  BOOST_TEST(calc.evaluate("( 1 | 2 ) + 3") == 6);
}

BOOST_AUTO_TEST_CASE(division_by_zero_throws)
{
  Calculator calc;
  BOOST_CHECK_THROW(calc.evaluate("5 / 0"), std::logic_error);
}

BOOST_AUTO_TEST_CASE(modulo_by_zero_throws)
{
  Calculator calc;
  BOOST_CHECK_THROW(calc.evaluate("5 % 0"), std::logic_error);
}

BOOST_AUTO_TEST_CASE(mismatched_parentheses_throw)
{
  Calculator calc;
  BOOST_CHECK_THROW(calc.evaluate("( 1 + 2"), std::logic_error);
  BOOST_CHECK_THROW(calc.evaluate("1 + 2 )"), std::logic_error);
}

BOOST_AUTO_TEST_CASE(invalid_expression_throws)
{
  Calculator calc;
  BOOST_CHECK_THROW(calc.evaluate("1 2 +"), std::logic_error);
  BOOST_CHECK_THROW(calc.evaluate("+ 1 2"), std::logic_error);
  BOOST_CHECK_THROW(calc.evaluate("1 +"), std::logic_error);
}

BOOST_AUTO_TEST_CASE(unknown_token_throws)
{
  Calculator calc;
  BOOST_CHECK_THROW(calc.evaluate("2 & 3"), std::logic_error);
}

BOOST_AUTO_TEST_CASE(sample_input_from_assignment)
{
  Calculator calc;
  BOOST_TEST(calc.evaluate("( 1 + 2 ) * ( 3 - 4 )") == -3);
  BOOST_TEST(calc.evaluate("1 + 3") == 4);
  BOOST_TEST(calc.evaluate("( 10 / ( 2 + 3 ) % 4 )") == 2);
  BOOST_TEST(calc.evaluate("4 * 7 - 3") == 25);
}
