#include <boost/test/unit_test.hpp>

#include <stdexcept>
#include <string>

#include "queue.hpp"

using sadovnik::Queue;

BOOST_AUTO_TEST_CASE(empty_queue_is_empty)
{
  Queue< int > queue;
  BOOST_TEST(queue.empty());
  BOOST_TEST(queue.size() == 0);
}

BOOST_AUTO_TEST_CASE(queue_push_increases_size)
{
  Queue< int > queue;
  queue.push(10);
  queue.push(20);

  BOOST_TEST(!queue.empty());
  BOOST_TEST(queue.size() == 2);
  BOOST_TEST(queue.front() == 10);
}

BOOST_AUTO_TEST_CASE(drop_returns_elements_in_order)
{
  Queue< int > queue;
  queue.push(1);
  queue.push(2);
  queue.push(3);

  int value = 0;
  queue.drop(value);
  BOOST_TEST(value == 1);
  queue.drop(value);
  BOOST_TEST(value == 2);
  queue.drop(value);
  BOOST_TEST(value == 3);
  BOOST_TEST(queue.empty());
}

BOOST_AUTO_TEST_CASE(drop_on_empty_queue_throws)
{
  Queue< int > queue;
  int value = 0;
  BOOST_CHECK_THROW(queue.drop(value), std::logic_error);
}

BOOST_AUTO_TEST_CASE(front_on_empty_queue_throws)
{
  Queue< int > queue;
  BOOST_CHECK_THROW(queue.front(), std::logic_error);
}

BOOST_AUTO_TEST_CASE(queue_copy_constructor_duplicates_content)
{
  Queue< int > source;
  source.push(5);
  source.push(7);

  Queue< int > copy(source);
  int value = 0;
  copy.drop(value);
  BOOST_TEST(value == 5);
  copy.drop(value);
  BOOST_TEST(value == 7);
}

BOOST_AUTO_TEST_CASE(queue_move_constructor_transfers_content)
{
  Queue< int > source;
  source.push(42);

  Queue< int > moved(std::move(source));
  BOOST_TEST(moved.front() == 42);
  BOOST_TEST(source.empty());
}

BOOST_AUTO_TEST_CASE(queue_swap_exchanges_contents)
{
  Queue< int > first;
  Queue< int > second;
  first.push(1);
  second.push(2);

  swap(first, second);
  BOOST_TEST(first.front() == 2);
  BOOST_TEST(second.front() == 1);
}
