#include <boost/test/unit_test.hpp>

#include <bs-tree-ops.hpp>

#include <functional>
#include <string>

namespace
{

  using IntStrTree = sadovnik::BSTree< int, std::string, std::less< int > >;

  void checkInOrder(const IntStrTree & tree,
                    const int * keys,
                    const char * const * values,
                    std::size_t count)
  {
    std::size_t idx = 0;
    for (auto it = tree.cbegin(); it != tree.cend(); ++it)
    {
      BOOST_REQUIRE(idx < count);
      BOOST_TEST(it->first == keys[idx]);
      BOOST_TEST(it->second == values[idx]);
      ++idx;
    }
    BOOST_TEST(idx == count);
  }

  IntStrTree makeLeft()
  {
    IntStrTree tree;
    tree.push(1, "L1");
    tree.push(3, "L3");
    tree.push(5, "L5");
    return tree;
  }

  IntStrTree makeRight()
  {
    IntStrTree tree;
    tree.push(3, "R3");
    tree.push(4, "R4");
    tree.push(6, "R6");
    return tree;
  }

}

BOOST_AUTO_TEST_CASE(bs_tree_complement_keeps_left_only_keys)
{
  const IntStrTree left = makeLeft();
  const IntStrTree right = makeRight();

  const IntStrTree result = sadovnik::complement(left, right);

  const int keys[] = {1, 5};
  const char * values[] = {"L1", "L5"};
  checkInOrder(result, keys, values, 2);
}

BOOST_AUTO_TEST_CASE(bs_tree_complement_with_empty_operand)
{
  IntStrTree left;
  left.push(2, "two");

  const IntStrTree right;
  const IntStrTree result = sadovnik::complement(left, right);

  BOOST_TEST(result.size() == 1);
  BOOST_TEST(result.get(2) == "two");

  const IntStrTree emptyResult = sadovnik::complement(left, left);
  BOOST_CHECK(emptyResult.empty());
}

BOOST_AUTO_TEST_CASE(bs_tree_intersect_keeps_common_keys_from_left)
{
  const IntStrTree left = makeLeft();
  const IntStrTree right = makeRight();

  const IntStrTree result = sadovnik::intersect(left, right);

  BOOST_TEST(result.size() == 1);
  BOOST_TEST(result.get(3) == "L3");
}

BOOST_AUTO_TEST_CASE(bs_tree_intersect_is_empty_without_common_keys)
{
  IntStrTree left;
  left.push(1, "one");

  IntStrTree right;
  right.push(2, "two");

  const IntStrTree result = sadovnik::intersect(left, right);
  BOOST_CHECK(result.empty());
}

BOOST_AUTO_TEST_CASE(bs_tree_unite_merges_keys_and_prefers_left_on_conflict)
{
  const IntStrTree left = makeLeft();
  const IntStrTree right = makeRight();

  const IntStrTree result = sadovnik::unite(left, right);

  const int keys[] = {1, 3, 4, 5, 6};
  const char * values[] = {"L1", "L3", "R4", "L5", "R6"};
  checkInOrder(result, keys, values, 5);
}

BOOST_AUTO_TEST_CASE(bs_tree_unite_does_not_modify_operands)
{
  const IntStrTree left = makeLeft();
  const IntStrTree right = makeRight();

  (void)sadovnik::unite(left, right);

  BOOST_TEST(left.size() == 3);
  BOOST_TEST(right.size() == 3);
  BOOST_TEST(left.get(3) == "L3");
  BOOST_TEST(right.get(3) == "R3");
}

BOOST_AUTO_TEST_CASE(bs_tree_set_ops_preserve_sorted_order)
{
  IntStrTree left;
  left.push(10, "a");
  left.push(20, "b");

  IntStrTree right;
  right.push(15, "c");
  right.push(25, "d");

  const IntStrTree united = sadovnik::unite(left, right);
  const int keys[] = {10, 15, 20, 25};
  const char * values[] = {"a", "c", "b", "d"};
  checkInOrder(united, keys, values, 4);
}
