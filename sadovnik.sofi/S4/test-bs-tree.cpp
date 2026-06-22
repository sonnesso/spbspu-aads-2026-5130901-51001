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

BOOST_AUTO_TEST_CASE(bs_tree_drop_leaf_returns_value)
{
  IntStrTree tree;
  tree.push(2, "two");
  tree.push(1, "one");
  tree.push(4, "four");

  BOOST_TEST(tree.drop(1) == "one");
  BOOST_TEST(tree.size() == 2);
  BOOST_CHECK(!tree.has(1));
  BOOST_CHECK(tree.has(2));
  BOOST_CHECK(tree.has(4));
}

BOOST_AUTO_TEST_CASE(bs_tree_drop_node_with_one_child)
{
  IntStrTree tree;
  tree.push(2, "two");
  tree.push(1, "one");
  tree.push(4, "four");
  tree.push(3, "three");

  BOOST_TEST(tree.drop(4) == "four");
  BOOST_CHECK(tree.has(3));
  BOOST_CHECK(!tree.has(4));
  BOOST_TEST(tree.get(3) == "three");
}

BOOST_AUTO_TEST_CASE(bs_tree_drop_node_with_two_children)
{
  IntStrTree tree;
  tree.push(2, "two");
  tree.push(1, "one");
  tree.push(4, "four");
  tree.push(3, "three");
  tree.push(5, "five");

  BOOST_TEST(tree.drop(2) == "two");
  BOOST_CHECK(!tree.has(2));
  BOOST_CHECK(tree.has(1));
  BOOST_CHECK(tree.has(3));
  BOOST_CHECK(tree.has(4));
  BOOST_CHECK(tree.has(5));
  BOOST_TEST(tree.get(3) == "three");
}

BOOST_AUTO_TEST_CASE(bs_tree_drop_rejects_missing_key)
{
  IntStrTree tree;
  tree.push(1, "one");

  BOOST_CHECK_THROW(tree.drop(2), std::out_of_range);
  BOOST_TEST(tree.size() == 1);
}

BOOST_AUTO_TEST_CASE(bs_tree_drop_last_element_clears_tree)
{
  IntStrTree tree;
  tree.push(1, "one");

  BOOST_TEST(tree.drop(1) == "one");
  BOOST_CHECK(tree.empty());
  BOOST_TEST(tree.size() == 0);
  BOOST_CHECK(tree.begin() == tree.end());
  BOOST_TEST(tree.height() == 0);
}

BOOST_AUTO_TEST_CASE(bs_tree_drop_allows_reinserting_key)
{
  IntStrTree tree;
  tree.push(1, "one");
  tree.drop(1);

  tree.push(1, "new-one");
  BOOST_TEST(tree.get(1) == "new-one");
}

BOOST_AUTO_TEST_CASE(bs_tree_iterators_visit_keys_in_order)
{
  IntStrTree tree;
  tree.push(2, "two");
  tree.push(4, "four");
  tree.push(1, "one");
  tree.push(5, "five");
  tree.push(3, "three");

  int keys[] = {1, 2, 3, 4, 5};
  std::size_t idx = 0;
  for (auto it = tree.begin(); it != tree.end(); ++it)
  {
    BOOST_REQUIRE(idx < 5);
    BOOST_TEST(it->first == keys[idx]);
    ++idx;
  }
  BOOST_TEST(idx == 5);
}

BOOST_AUTO_TEST_CASE(bs_tree_const_iterators_visit_values_in_order)
{
  IntStrTree tree;
  tree.push(3, "c");
  tree.push(1, "a");
  tree.push(2, "b");

  const IntStrTree & view = tree;
  const std::string vals[] = {"a", "b", "c"};
  std::size_t idx = 0;
  for (auto it = view.cbegin(); it != view.cend(); ++it)
  {
    BOOST_REQUIRE(idx < 3);
    BOOST_TEST(it->second == vals[idx]);
    ++idx;
  }
}

BOOST_AUTO_TEST_CASE(bs_tree_iterator_dereference_reads_entry)
{
  IntStrTree tree;
  tree.push(2, "two");

  auto it = tree.begin();
  BOOST_TEST(it->first == 2);
  BOOST_TEST((*it).second == "two");
}

BOOST_AUTO_TEST_CASE(bs_tree_iterator_decrement_walks_backward)
{
  IntStrTree tree;
  tree.push(1, "one");
  tree.push(2, "two");
  tree.push(3, "three");

  auto it = tree.end();
  --it;
  BOOST_TEST(it->first == 3);

  --it;
  BOOST_TEST(it->first == 2);

  --it;
  BOOST_TEST(it->first == 1);
  BOOST_CHECK(it == tree.begin());
}

BOOST_AUTO_TEST_CASE(bs_tree_iterator_dereference_end_throws)
{
  IntStrTree tree;
  tree.push(1, "one");

  auto it = tree.end();
  BOOST_CHECK_THROW(*it, std::logic_error);
}

BOOST_AUTO_TEST_CASE(bs_tree_height_grows_on_skewed_insert)
{
  IntStrTree tree;
  tree.push(1, "a");
  tree.push(2, "b");
  tree.push(3, "c");
  tree.push(4, "d");
  tree.push(5, "e");

  BOOST_TEST(tree.height() == 5);
}

BOOST_AUTO_TEST_CASE(bs_tree_height_at_root_matches_tree_height)
{
  IntStrTree tree;
  tree.push(2, "two");
  tree.push(1, "one");
  tree.push(4, "four");
  tree.push(3, "three");
  tree.push(5, "five");

  const IntStrTree & view = tree;
  const auto rootIt = view.find(2);
  BOOST_REQUIRE(rootIt != view.cend());
  BOOST_TEST(view.height(rootIt) == view.height());
  BOOST_TEST(view.height() == 3);
}

BOOST_AUTO_TEST_CASE(bs_tree_height_at_internal_node_counts_subtree)
{
  IntStrTree tree;
  tree.push(2, "two");
  tree.push(1, "one");
  tree.push(4, "four");
  tree.push(3, "three");
  tree.push(5, "five");

  const IntStrTree & view = tree;
  const auto nodeIt = view.find(4);
  BOOST_REQUIRE(nodeIt != view.cend());
  BOOST_TEST(view.height(nodeIt) == 2);
}

BOOST_AUTO_TEST_CASE(bs_tree_height_at_leaf_is_one)
{
  IntStrTree tree;
  tree.push(2, "two");
  tree.push(1, "one");
  tree.push(4, "four");

  const IntStrTree & view = tree;
  const auto leafIt = view.find(1);
  BOOST_REQUIRE(leafIt != view.cend());
  BOOST_TEST(view.height(leafIt) == 1);
}

BOOST_AUTO_TEST_CASE(bs_tree_height_shrinks_after_drop)
{
  IntStrTree tree;
  tree.push(1, "a");
  tree.push(2, "b");
  tree.push(3, "c");
  tree.push(4, "d");

  BOOST_TEST(tree.height() == 4);
  tree.drop(1);
  BOOST_TEST(tree.height() == 3);
}

BOOST_AUTO_TEST_CASE(bs_tree_find_returns_end_for_missing_key)
{
  IntStrTree tree;
  tree.push(1, "one");

  const IntStrTree & view = tree;
  BOOST_CHECK(view.find(2) == view.cend());
  BOOST_TEST(view.height(view.find(99)) == 0);
}
