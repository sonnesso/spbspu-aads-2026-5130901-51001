#include <boost/test/unit_test.hpp>

#include <hash-table.hpp>

#include <stdexcept>
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

BOOST_AUTO_TEST_CASE(hash_table_add_and_get_store_values)
{
  StrTable tab(4);

  BOOST_CHECK(tab.add("one", 1));
  BOOST_CHECK(tab.has("one"));
  BOOST_TEST(tab.get("one") == 1);
  BOOST_TEST(tab.size() == 1);

  BOOST_REQUIRE(tab.find("one") != nullptr);
  BOOST_TEST(*tab.find("one") == 1);
}

BOOST_AUTO_TEST_CASE(hash_table_add_rejects_duplicate_key)
{
  StrTable tab(2);

  tab.add("same", 1);

  BOOST_CHECK_THROW(tab.add("same", 2), std::logic_error);
  BOOST_TEST(tab.get("same") == 1);
}

BOOST_AUTO_TEST_CASE(hash_table_get_on_missing_key_throws)
{
  StrTable tab(2);

  BOOST_CHECK(!tab.has("missing"));
  BOOST_CHECK(tab.find("missing") == nullptr);
  BOOST_CHECK_THROW(tab.get("missing"), std::out_of_range);
}
