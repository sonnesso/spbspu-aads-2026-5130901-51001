#include <boost/test/unit_test.hpp>

#include <bs-tree.hpp>

#include <functional>
#include <stdexcept>
#include <string>

namespace
{

  using IntStrTree = sadovnik::BSTree< int, std::string, std::less< int > >;

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

BOOST_AUTO_TEST_CASE(bs_tree_rotate_left_promotes_right_child)
{
  IntStrTree tree;
  tree.push(2, "two");
  tree.push(1, "one");
  tree.push(4, "four");

  const auto rising = tree.find(4);
  const auto after = tree.rotateLeft(rising);

  BOOST_TEST(after->first == 4);
  BOOST_TEST(tree.height() == 3);
  BOOST_CHECK(tree.has(1));
  BOOST_CHECK(tree.has(2));
  BOOST_CHECK(tree.has(4));

  const int keys[] = {1, 2, 4};
  checkInOrderKeys(tree, keys, 3);
}

BOOST_AUTO_TEST_CASE(bs_tree_rotate_right_promotes_left_child)
{
  IntStrTree tree;
  tree.push(4, "four");
  tree.push(2, "two");
  tree.push(5, "five");

  const auto rising = tree.find(2);
  const auto after = tree.rotateRight(rising);

  BOOST_TEST(after->first == 2);
  BOOST_TEST(tree.height() == 3);
  BOOST_CHECK(tree.has(2));
  BOOST_CHECK(tree.has(4));
  BOOST_CHECK(tree.has(5));

  const int keys[] = {2, 4, 5};
  checkInOrderKeys(tree, keys, 3);
}

BOOST_AUTO_TEST_CASE(bs_tree_rotate_keeps_size_and_lookup)
{
  IntStrTree tree;
  tree.push(2, "two");
  tree.push(1, "one");
  tree.push(4, "four");
  tree.push(3, "three");

  tree.rotateLeft(tree.find(4));

  BOOST_TEST(tree.size() == 4);
  BOOST_TEST(tree.get(3) == "three");
}

BOOST_AUTO_TEST_CASE(bs_tree_rotate_preserves_iterator_nodes)
{
  IntStrTree tree;
  tree.push(2, "two");
  tree.push(1, "one");
  tree.push(4, "four");

  const auto saved = tree.find(1);
  tree.rotateLeft(tree.find(4));

  BOOST_TEST(saved->first == 1);
  BOOST_TEST(saved->second == "one");
  BOOST_TEST(tree.get(1) == "one");
}

BOOST_AUTO_TEST_CASE(bs_tree_rotate_left_rejects_invalid_position)
{
  IntStrTree tree;
  tree.push(2, "two");
  tree.push(1, "one");

  BOOST_CHECK_THROW(tree.rotateLeft(tree.find(1)), std::logic_error);
  BOOST_CHECK_THROW(tree.rotateLeft(tree.cend()), std::logic_error);
}

BOOST_AUTO_TEST_CASE(bs_tree_rotate_right_rejects_invalid_position)
{
  IntStrTree tree;
  tree.push(2, "two");
  tree.push(4, "four");

  BOOST_CHECK_THROW(tree.rotateRight(tree.find(4)), std::logic_error);
}

BOOST_AUTO_TEST_CASE(bs_tree_rotate_large_left_promotes_middle_node)
{
  IntStrTree tree;
  tree.push(5, "five");
  tree.push(2, "two");
  tree.push(4, "four");

  const auto rising = tree.find(4);
  const auto after = tree.rotateLargeLeft(rising);

  BOOST_TEST(after->first == 4);
  BOOST_TEST(tree.height() == 2);
  BOOST_CHECK(tree.has(2));
  BOOST_CHECK(tree.has(4));
  BOOST_CHECK(tree.has(5));

  const int keys[] = {2, 4, 5};
  checkInOrderKeys(tree, keys, 3);
}

BOOST_AUTO_TEST_CASE(bs_tree_rotate_large_right_promotes_middle_node)
{
  IntStrTree tree;
  tree.push(2, "two");
  tree.push(5, "five");
  tree.push(8, "eight");
  tree.push(4, "four");

  const auto rising = tree.find(4);
  const auto after = tree.rotateLargeRight(rising);

  BOOST_TEST(after->first == 4);
  BOOST_TEST(tree.height() == 3);
  BOOST_CHECK(tree.has(2));
  BOOST_CHECK(tree.has(4));
  BOOST_CHECK(tree.has(5));
  BOOST_CHECK(tree.has(8));

  const int keys[] = {2, 4, 5, 8};
  checkInOrderKeys(tree, keys, 4);
}

BOOST_AUTO_TEST_CASE(bs_tree_rotate_large_keeps_size_and_lookup)
{
  IntStrTree tree;
  tree.push(5, "five");
  tree.push(2, "two");
  tree.push(6, "six");
  tree.push(4, "four");
  tree.push(3, "three");

  tree.rotateLargeLeft(tree.find(4));

  BOOST_TEST(tree.size() == 5);
  BOOST_TEST(tree.get(3) == "three");
  BOOST_TEST(tree.get(6) == "six");
}

BOOST_AUTO_TEST_CASE(bs_tree_rotate_large_preserves_iterator_nodes)
{
  IntStrTree tree;
  tree.push(2, "two");
  tree.push(5, "five");
  tree.push(8, "eight");
  tree.push(4, "four");

  const auto saved = tree.find(8);
  tree.rotateLargeRight(tree.find(4));

  BOOST_TEST(saved->first == 8);
  BOOST_TEST(saved->second == "eight");
  BOOST_TEST(tree.get(8) == "eight");
}

BOOST_AUTO_TEST_CASE(bs_tree_rotate_large_left_rejects_invalid_position)
{
  IntStrTree tree;
  tree.push(5, "five");
  tree.push(2, "two");
  tree.push(4, "four");

  BOOST_CHECK_THROW(tree.rotateLargeLeft(tree.find(2)), std::logic_error);
  BOOST_CHECK_THROW(tree.rotateLargeLeft(tree.cend()), std::logic_error);
}

BOOST_AUTO_TEST_CASE(bs_tree_rotate_large_right_rejects_invalid_position)
{
  IntStrTree tree;
  tree.push(2, "two");
  tree.push(5, "five");
  tree.push(4, "four");

  BOOST_CHECK_THROW(tree.rotateLargeRight(tree.find(5)), std::logic_error);
}
