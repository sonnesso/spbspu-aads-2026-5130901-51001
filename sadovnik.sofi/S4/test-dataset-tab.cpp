#include <boost/test/unit_test.hpp>

#include "dataset-tab.hpp"

#include <bs-tree-ops.hpp>

BOOST_AUTO_TEST_CASE(dataset_tab_stores_named_bstrees)
{
  sadovnik::DatasetTab datasets(4);

  sadovnik::IntStrTree first;
  first.push(1, "one");
  first.push(2, "two");

  sadovnik::IntStrTree second;
  second.push(3, "three");

  datasets.add("first", first);
  datasets.add("second", second);

  BOOST_TEST(datasets.size() == 2);
  BOOST_TEST(datasets.get("first").get(1) == "one");
  BOOST_TEST(datasets.get("second").get(3) == "three");
}

BOOST_AUTO_TEST_CASE(dataset_tab_copy_keeps_independent_trees)
{
  sadovnik::DatasetTab datasets(4);

  sadovnik::IntStrTree tree;
  tree.push(1, "one");
  datasets.add("data", tree);

  sadovnik::DatasetTab copy(datasets);
  copy.get("data").push(2, "two");

  BOOST_TEST(datasets.get("data").size() == 1);
  BOOST_TEST(copy.get("data").size() == 2);
}

BOOST_AUTO_TEST_CASE(dataset_tab_iterator_visits_dataset_names)
{
  sadovnik::DatasetTab datasets(8);

  sadovnik::IntStrTree empty;
  datasets.add("alpha", empty);
  datasets.add("beta", empty);

  std::size_t count = 0;
  for (auto it = datasets.begin(); it != datasets.end(); ++it)
  {
    BOOST_CHECK(it->key == "alpha" || it->key == "beta");
    ++count;
  }

  BOOST_TEST(count == 2);
}

BOOST_AUTO_TEST_CASE(dataset_tab_supports_set_ops_on_stored_trees)
{
  sadovnik::DatasetTab datasets(4);

  sadovnik::IntStrTree left;
  left.push(1, "L1");
  left.push(3, "L3");

  sadovnik::IntStrTree right;
  right.push(3, "R3");
  right.push(4, "R4");

  datasets.add("left", left);
  datasets.add("right", right);

  const sadovnik::IntStrTree united =
    sadovnik::unite(datasets.get("left"), datasets.get("right"));

  datasets.add("merged", united);

  BOOST_TEST(datasets.get("merged").get(3) == "L3");
  BOOST_TEST(datasets.get("merged").size() == 3);
}
