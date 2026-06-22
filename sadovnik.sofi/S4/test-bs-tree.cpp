#include <boost/test/unit_test.hpp>

#include <bs-tree.hpp>

#include <functional>
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
