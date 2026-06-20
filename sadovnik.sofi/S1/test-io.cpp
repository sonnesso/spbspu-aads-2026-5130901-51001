#include <boost/test/unit_test.hpp>

#include <limits>
#include <sstream>
#include <stdexcept>
#include <string>

#include "io.hpp"

using sadovnik::List;
using sadovnik::Seq;
using sadovnik::calculateSums;
using sadovnik::printNames;
using sadovnik::printSums;
using sadovnik::printTransposed;
using sadovnik::readData;

List< Seq > readSampleData()
{
  std::istringstream input(
    "first 1 1 1\n"
    "second 2 2 2 2\n"
    "third\n"
    "fourth 4 4\n");
  return readData(input);
}

BOOST_AUTO_TEST_CASE(read_empty_input)
{
  std::istringstream input;
  List< Seq > data = readData(input);
  BOOST_TEST(data.empty());
}

BOOST_AUTO_TEST_CASE(read_named_sequences)
{
  std::istringstream input("first 1 1\nsecond 2\n");
  List< Seq > data = readData(input);

  BOOST_TEST(data.size() == 2);
  sadovnik::LCIter< Seq > it = data.cbegin();
  BOOST_TEST(it->first == "first");
  BOOST_TEST(it->second.front() == 1);
}

BOOST_AUTO_TEST_CASE(print_names_for_sample_input)
{
  List< Seq > data = readSampleData();

  std::ostringstream output;
  printNames(output, data);
  BOOST_TEST(output.str() == "first second third fourth\n");
}

BOOST_AUTO_TEST_CASE(print_transposed_for_sample_input)
{
  List< Seq > data = readSampleData();

  std::ostringstream output;
  printTransposed(output, data);
  BOOST_TEST(output.str() ==
    "1 2 4\n"
    "1 2 4\n"
    "1 2\n"
    "2\n");
}

BOOST_AUTO_TEST_CASE(calculate_and_print_sums_for_sample_input)
{
  List< Seq > data = readSampleData();
  List< std::size_t > sums = calculateSums(data);

  std::ostringstream output;
  printSums(output, sums);
  BOOST_TEST(output.str() == "7 7 3 2\n");
}

BOOST_AUTO_TEST_CASE(print_transposed_without_numbers_is_empty)
{
  List< Seq > data;
  data.pushBack(Seq("only_name", sadovnik::List< std::size_t >()));

  std::ostringstream output;
  printTransposed(output, data);
  BOOST_TEST(output.str().empty());
}

BOOST_AUTO_TEST_CASE(calculate_sums_without_numbers_is_empty)
{
  List< Seq > data;
  data.pushBack(Seq("only_name", sadovnik::List< std::size_t >()));

  List< std::size_t > sums = calculateSums(data);
  BOOST_TEST(sums.empty());
}

BOOST_AUTO_TEST_CASE(calculate_sums_overflow_throws)
{
  sadovnik::List< std::size_t > numbersA;
  numbersA.pushBack(std::numeric_limits< std::size_t >::max());
  sadovnik::List< std::size_t > numbersB;
  numbersB.pushBack(1);

  List< Seq > data;
  data.pushBack(Seq("a", numbersA));
  data.pushBack(Seq("b", numbersB));

  BOOST_CHECK_THROW(calculateSums(data), std::overflow_error);
}
