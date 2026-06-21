#include <boost/test/unit_test.hpp>

#include <stdexcept>
#include <string>

#include "stack.hpp"

using sadovnik::Stack;

BOOST_AUTO_TEST_CASE(empty_stack_is_empty)
{
  Stack< int > stack;
  BOOST_TEST(stack.empty());
  BOOST_TEST(stack.size() == 0);
}

BOOST_AUTO_TEST_CASE(stack_push_increases_size)
{
  Stack< int > stack;
  stack.push(10);
  stack.push(20);

  BOOST_TEST(!stack.empty());
  BOOST_TEST(stack.size() == 2);
  BOOST_TEST(stack.top() == 20);
}

BOOST_AUTO_TEST_CASE(drop_returns_elements_in_reverse_order)
{
  Stack< int > stack;
  stack.push(1);
  stack.push(2);
  stack.push(3);

  int value = 0;
  stack.drop(value);
  BOOST_TEST(value == 3);
  stack.drop(value);
  BOOST_TEST(value == 2);
  stack.drop(value);
  BOOST_TEST(value == 1);
  BOOST_TEST(stack.empty());
}

BOOST_AUTO_TEST_CASE(drop_on_empty_stack_throws)
{
  Stack< int > stack;
  int value = 0;
  BOOST_CHECK_THROW(stack.drop(value), std::logic_error);
}

BOOST_AUTO_TEST_CASE(top_on_empty_stack_throws)
{
  Stack< int > stack;
  BOOST_CHECK_THROW(stack.top(), std::logic_error);
}

BOOST_AUTO_TEST_CASE(stack_copy_constructor_duplicates_content)
{
  Stack< int > source;
  source.push(5);
  source.push(7);

  Stack< int > copy(source);
  int value = 0;
  copy.drop(value);
  BOOST_TEST(value == 7);
  copy.drop(value);
  BOOST_TEST(value == 5);
}

BOOST_AUTO_TEST_CASE(stack_move_constructor_transfers_content)
{
  Stack< int > source;
  source.push(42);

  Stack< int > moved(std::move(source));
  BOOST_TEST(moved.top() == 42);
  BOOST_TEST(source.empty());
}

BOOST_AUTO_TEST_CASE(stack_swap_exchanges_contents)
{
  Stack< int > first;
  Stack< int > second;
  first.push(1);
  second.push(2);

  swap(first, second);
  BOOST_TEST(first.top() == 2);
  BOOST_TEST(second.top() == 1);
}
