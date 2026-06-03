#include <boost/test/unit_test.hpp>

#include <stdexcept>
#include <string>

#include "list.hpp"

using sadovnik::List;
using sadovnik::LCIter;
using sadovnik::LIter;

BOOST_AUTO_TEST_CASE(empty_list_is_empty)
{
  List< int > list;
  BOOST_TEST(list.empty());
  BOOST_TEST(list.size() == 0);
  BOOST_CHECK(list.begin() == list.end());
  BOOST_CHECK(list.cbegin() == list.cend());
}

BOOST_AUTO_TEST_CASE(push_back_adds_elements_in_order)
{
  List< int > list;
  list.pushBack(1);
  list.pushBack(2);
  list.pushBack(3);

  BOOST_TEST(list.size() == 3);
  BOOST_TEST(list.front() == 1);
  BOOST_TEST(list.back() == 3);

  int expected = 1;
  for (LCIter< int > it = list.cbegin(); it != list.cend(); ++it)
  {
    BOOST_TEST(*it == expected);
    ++expected;
  }
}

BOOST_AUTO_TEST_CASE(push_front_adds_elements_before_first)
{
  List< int > list;
  list.pushBack(2);
  list.pushFront(1);

  BOOST_TEST(list.front() == 1);
  BOOST_TEST(list.back() == 2);
}

BOOST_AUTO_TEST_CASE(insert_after_last_appends_element)
{
  List< int > list;
  list.pushBack(1);
  list.insertAfter(list.last(), 2);

  BOOST_TEST(list.back() == 2);
}

BOOST_AUTO_TEST_CASE(insert_after_middle_keeps_order)
{
  List< int > list;
  list.pushBack(1);
  list.pushBack(3);
  LIter< int > first = list.begin();
  list.insertAfter(first, 2);

  int expected = 1;
  for (LCIter< int > it = list.cbegin(); it != list.cend(); ++it)
  {
    BOOST_TEST(*it == expected);
    ++expected;
  }
}

BOOST_AUTO_TEST_CASE(pop_front_and_pop_back_remove_edges)
{
  List< int > list;
  list.pushBack(1);
  list.pushBack(2);
  list.pushBack(3);

  list.popFront();
  BOOST_TEST(list.front() == 2);

  list.popBack();
  BOOST_TEST(list.back() == 2);
  BOOST_TEST(list.front() == 2);
}

BOOST_AUTO_TEST_CASE(erase_middle_element)
{
  List< int > list;
  list.pushBack(1);
  list.pushBack(2);
  list.pushBack(3);

  LIter< int > it = list.begin();
  ++it;
  LIter< int > next = list.erase(it);

  BOOST_TEST(*list.begin() == 1);
  BOOST_TEST(*next == 3);
}

BOOST_AUTO_TEST_CASE(clear_empties_list)
{
  List< int > list;
  list.pushBack(1);
  list.pushBack(2);
  list.clear();

  BOOST_TEST(list.empty());
  BOOST_CHECK(list.begin() == list.end());
}

BOOST_AUTO_TEST_CASE(copy_constructor_duplicates_content)
{
  List< int > source;
  source.pushBack(10);
  source.pushBack(20);

  List< int > copy(source);
  source.popFront();

  BOOST_TEST(copy.size() == 2);
  BOOST_TEST(copy.front() == 10);
  BOOST_TEST(copy.back() == 20);
}

BOOST_AUTO_TEST_CASE(copy_assignment_duplicates_content)
{
  List< int > source;
  source.pushBack(5);

  List< int > target;
  target.pushBack(1);
  target = source;

  BOOST_TEST(target.front() == 5);
}

BOOST_AUTO_TEST_CASE(move_constructor_transfers_nodes)
{
  List< int > source;
  source.pushBack(7);

  List< int > moved(std::move(source));
  BOOST_TEST(moved.front() == 7);
  BOOST_TEST(source.empty());
}

BOOST_AUTO_TEST_CASE(move_assignment_transfers_nodes)
{
  List< int > source;
  source.pushBack(8);

  List< int > target;
  target = std::move(source);

  BOOST_TEST(target.front() == 8);
  BOOST_TEST(source.empty());
}

BOOST_AUTO_TEST_CASE(bidirectional_iteration)
{
  List< int > list;
  list.pushBack(1);
  list.pushBack(2);
  list.pushBack(3);

  LIter< int > it = list.begin();
  ++it;
  ++it;
  BOOST_TEST(*it == 3);
  --it;
  BOOST_TEST(*it == 2);
}

BOOST_AUTO_TEST_CASE(empty_access_throws)
{
  List< int > list;
  BOOST_CHECK_THROW(list.front(), std::logic_error);
  BOOST_CHECK_THROW(list.back(), std::logic_error);
  BOOST_CHECK_THROW(list.popFront(), std::logic_error);
  BOOST_CHECK_THROW(list.popBack(), std::logic_error);
}

BOOST_AUTO_TEST_CASE(copy_with_nested_lists)
{
  List< List< int > > outer;
  List< int > inner;
  inner.pushBack(42);
  outer.pushBack(inner);

  List< List< int > > copy(outer);
  BOOST_TEST(copy.front().front() == 42);
}
