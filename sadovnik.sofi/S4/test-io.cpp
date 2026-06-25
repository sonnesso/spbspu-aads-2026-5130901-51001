#include <boost/test/unit_test.hpp>

#include "io.hpp"

#include <sstream>
#include <stdexcept>

BOOST_AUTO_TEST_CASE(io_reads_example_from_assignment)
{
  std::istringstream input(
    "first 1 name 2 surname\n"
    "second 4 mouse 1 name 2 keyboard\n");

  const sadovnik::DatasetTab datasets = sadovnik::readDatasets(input);

  BOOST_CHECK(datasets.has("first"));
  BOOST_CHECK(datasets.has("second"));
  BOOST_TEST(datasets.get("first").get(1) == "name");
  BOOST_TEST(datasets.get("first").get(2) == "surname");
  BOOST_TEST(datasets.get("second").get(4) == "mouse");
  BOOST_TEST(datasets.get("second").get(1) == "name");
}

BOOST_AUTO_TEST_CASE(io_ignores_empty_lines)
{
  std::istringstream input("\n\nalpha 1 one\n\n beta 2 two \n");

  const sadovnik::DatasetTab datasets = sadovnik::readDatasets(input);

  BOOST_TEST(datasets.size() == 2);
  BOOST_TEST(datasets.get("alpha").get(1) == "one");
  BOOST_TEST(datasets.get("beta").get(2) == "two");
}

BOOST_AUTO_TEST_CASE(io_reads_empty_dataset_line)
{
  std::istringstream input("empty\n");

  const sadovnik::DatasetTab datasets = sadovnik::readDatasets(input);

  BOOST_CHECK(datasets.has("empty"));
  BOOST_CHECK(datasets.get("empty").empty());
}

BOOST_AUTO_TEST_CASE(io_reads_empty_file_as_empty_table)
{
  std::istringstream input;

  const sadovnik::DatasetTab datasets = sadovnik::readDatasets(input);

  BOOST_CHECK(datasets.empty());
}

BOOST_AUTO_TEST_CASE(io_rejects_invalid_dataset_file)
{
  std::istringstream badName("1bad 1 one\n");
  std::istringstream duplicateName("a 1 one\na 2 two\n");
  std::istringstream oddTokens("a 1\n");
  std::istringstream badKey("a x one\n");
  std::istringstream duplicateKey("a 1 one 1 two\n");

  BOOST_CHECK_THROW(sadovnik::readDatasets(badName), std::logic_error);
  BOOST_CHECK_THROW(sadovnik::readDatasets(duplicateName), std::logic_error);
  BOOST_CHECK_THROW(sadovnik::readDatasets(oddTokens), std::logic_error);
  BOOST_CHECK_THROW(sadovnik::readDatasets(badKey), std::logic_error);
  BOOST_CHECK_THROW(sadovnik::readDatasets(duplicateKey), std::logic_error);
}
