#include <boost/test/unit_test.hpp>

#include <hash-table.hpp>

#include <string>

namespace
{

  struct ZeroHash
  {
    std::size_t operator()(const std::string &) const
    {
      return 0;
    }
  };

  struct StrEq
  {
    bool operator()(const std::string & lhs, const std::string & rhs) const
    {
      return lhs == rhs;
    }
  };

  using StrTable = sadovnik::HashTable< std::string, int, ZeroHash, StrEq >;

}

BOOST_AUTO_TEST_CASE(hash_table_starts_empty)
{
  StrTable tab(3);

  BOOST_CHECK(tab.empty());
  BOOST_TEST(tab.size() == 0);
  BOOST_TEST(tab.slots() == 3);
}

BOOST_AUTO_TEST_CASE(hash_table_capacity_counts_spare_cell)
{
  StrTable tab(5, 2);

  BOOST_TEST(tab.bucketSize() == 2);
  BOOST_TEST(tab.capacity() == 15);
}

BOOST_AUTO_TEST_CASE(hash_table_clear_keeps_capacity)
{
  StrTable tab(2, 3);
  tab.clear();

  BOOST_CHECK(tab.empty());
  BOOST_TEST(tab.capacity() == 8);
}

BOOST_AUTO_TEST_CASE(hash_table_rejects_bad_capacity)
{
  BOOST_CHECK_THROW(StrTable(0), std::invalid_argument);
  BOOST_CHECK_THROW(StrTable(1, 0), std::invalid_argument);
}
