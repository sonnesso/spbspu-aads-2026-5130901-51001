#include <boost/test/unit_test.hpp>

#include <bs-tree.hpp>

#include <functional>
#include <stdexcept>
#include <string>

namespace
{

  using IntStrTree = sadovnik::BSTree< int, std::string, std::less< int > >;

}

BOOST_AUTO_TEST_CASE(bs_tree_starts_empty)
{
  IntStrTree tree;

  BOOST_CHECK(tree.empty());
  BOOST_TEST(tree.size() == 0);
}

BOOST_AUTO_TEST_CASE(bs_tree_empty_has_zero_height)
{
  IntStrTree tree;

  BOOST_TEST(tree.height() == 0);
  BOOST_TEST(tree.height(tree.cend()) == 0);
}

BOOST_AUTO_TEST_CASE(bs_tree_empty_iterators_are_equal)
{
  IntStrTree tree;

  BOOST_CHECK(tree.begin() == tree.end());
  BOOST_CHECK(tree.cbegin() == tree.cend());
}

BOOST_AUTO_TEST_CASE(bs_tree_clear_keeps_empty_state)
{
  IntStrTree tree;
  tree.clear();

  BOOST_CHECK(tree.empty());
  BOOST_TEST(tree.size() == 0);
  BOOST_TEST(tree.height() == 0);
  BOOST_CHECK(tree.begin() == tree.end());
}

BOOST_AUTO_TEST_CASE(bs_tree_default_constructed_trees_are_independent)
{
  IntStrTree first;
  IntStrTree second;

  first.clear();

  BOOST_CHECK(second.empty());
  BOOST_TEST(second.size() == 0);
}

BOOST_AUTO_TEST_CASE(bs_tree_push_and_get_store_values)
{
  IntStrTree tree;

  tree.push(2, "two");
  tree.push(1, "one");
  tree.push(4, "four");

  BOOST_CHECK(!tree.empty());
  BOOST_TEST(tree.size() == 3);
  BOOST_CHECK(tree.has(1));
  BOOST_CHECK(tree.has(2));
  BOOST_CHECK(tree.has(4));
  BOOST_TEST(tree.get(1) == "one");
  BOOST_TEST(tree.get(2) == "two");
  BOOST_TEST(tree.get(4) == "four");
}

BOOST_AUTO_TEST_CASE(bs_tree_get_rejects_missing_key)
{
  IntStrTree tree;
  tree.push(1, "one");

  BOOST_CHECK(!tree.has(2));
  BOOST_CHECK_THROW(tree.get(2), std::out_of_range);
}

BOOST_AUTO_TEST_CASE(bs_tree_push_rejects_duplicate_key)
{
  IntStrTree tree;
  tree.push(1, "one");

  BOOST_CHECK_THROW(tree.push(1, "another"), std::logic_error);
  BOOST_TEST(tree.size() == 1);
  BOOST_TEST(tree.get(1) == "one");
}

BOOST_AUTO_TEST_CASE(bs_tree_push_updates_height_and_begin)
{
  IntStrTree tree;

  tree.push(2, "two");
  BOOST_TEST(tree.height() == 1);
  BOOST_CHECK(tree.begin() != tree.end());

  tree.push(1, "one");
  tree.push(4, "four");
  BOOST_TEST(tree.height() == 2);
}
