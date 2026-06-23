#include <boost/test/unit_test.hpp>

#include <bs-tree.hpp>

#include <functional>
#include <string>
#include <utility>

namespace
{

  using IntStrTree = sadovnik::BSTree< int, std::string, std::less< int > >;

  void fillSampleTree(IntStrTree & tree)
  {
    tree.push(4, "four");
    tree.push(2, "two");
    tree.push(6, "six");
    tree.push(1, "one");
    tree.push(3, "three");
    tree.push(5, "five");
  }

  void checkInOrderKeys(const IntStrTree & tree, const int * keys, std::size_t count)
  {
    std::size_t idx = 0;
    for (auto it = tree.cbegin(); it != tree.cend(); ++it)
    {
      BOOST_REQUIRE(idx < count);
      BOOST_TEST(it->first == keys[idx]);
      ++idx;
    }
    BOOST_TEST(idx == count);
  }

}

BOOST_AUTO_TEST_CASE(bs_tree_copy_constructor_duplicates_content)
{
  IntStrTree source;
  fillSampleTree(source);

  IntStrTree copy(source);

  BOOST_TEST(copy.size() == source.size());
  BOOST_TEST(copy.height() == source.height());
  BOOST_TEST(copy.get(3) == "three");
  BOOST_TEST(copy.get(5) == "five");
}

BOOST_AUTO_TEST_CASE(bs_tree_copy_is_independent)
{
  IntStrTree source;
  fillSampleTree(source);

  IntStrTree copy(source);
  source.push(7, "seven");

  BOOST_TEST(copy.size() == 6);
  BOOST_CHECK(!copy.has(7));
  BOOST_TEST(source.size() == 7);
}

BOOST_AUTO_TEST_CASE(bs_tree_copy_preserves_structure)
{
  IntStrTree source;
  fillSampleTree(source);

  const auto sourceIt = source.find(3);
  IntStrTree copy(source);
  const auto copyIt = copy.find(3);

  BOOST_TEST(sourceIt->second == copyIt->second);
  BOOST_TEST(source.height(sourceIt) == copy.height(copyIt));
}

BOOST_AUTO_TEST_CASE(bs_tree_copy_assignment_duplicates_content)
{
  IntStrTree source;
  fillSampleTree(source);

  IntStrTree target;
  target.push(9, "nine");
  target = source;

  BOOST_TEST(target.size() == 6);
  BOOST_TEST(target.get(1) == "one");
  BOOST_CHECK(!target.has(9));
}

BOOST_AUTO_TEST_CASE(bs_tree_copy_assignment_is_independent)
{
  IntStrTree source;
  fillSampleTree(source);

  IntStrTree target;
  target = source;
  source.drop(2);

  BOOST_CHECK(target.has(2));
  BOOST_CHECK(!source.has(2));
}

BOOST_AUTO_TEST_CASE(bs_tree_move_constructor_transfers_content)
{
  IntStrTree source;
  fillSampleTree(source);

  const int keys[] = {1, 2, 3, 4, 5, 6};
  IntStrTree moved(std::move(source));

  BOOST_TEST(moved.size() == 6);
  BOOST_CHECK(source.empty());
  BOOST_TEST(source.size() == 0);
  checkInOrderKeys(moved, keys, 6);
}

BOOST_AUTO_TEST_CASE(bs_tree_move_assignment_transfers_content)
{
  IntStrTree source;
  fillSampleTree(source);

  IntStrTree target;
  target.push(9, "nine");
  target = std::move(source);

  BOOST_TEST(target.size() == 6);
  BOOST_TEST(target.get(4) == "four");
  BOOST_CHECK(source.empty());
  BOOST_CHECK(!target.has(9));
}

BOOST_AUTO_TEST_CASE(bs_tree_swap_exchanges_contents)
{
  IntStrTree first;
  first.push(1, "one");
  first.push(3, "three");

  IntStrTree second;
  second.push(2, "two");
  second.push(4, "four");
  second.push(5, "five");

  swap(first, second);

  BOOST_TEST(first.size() == 3);
  BOOST_TEST(second.size() == 2);
  BOOST_TEST(first.get(4) == "four");
  BOOST_TEST(second.get(1) == "one");
}

BOOST_AUTO_TEST_CASE(bs_tree_self_copy_assignment_is_safe)
{
  IntStrTree tree;
  fillSampleTree(tree);

  IntStrTree & ref = tree;
  tree = ref;

  BOOST_TEST(tree.size() == 6);
  BOOST_TEST(tree.get(6) == "six");
}
