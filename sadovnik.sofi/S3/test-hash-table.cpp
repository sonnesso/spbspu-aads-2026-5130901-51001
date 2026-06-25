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
  using CollTab = sadovnik::HashTable< std::string, int, ZeroHash, StrEq >;

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

BOOST_AUTO_TEST_CASE(hash_table_set_replaces_or_inserts)
{
  StrTable tab(2);

  BOOST_CHECK(tab.set("value", 1));
  BOOST_CHECK(!tab.set("value", 2));

  BOOST_TEST(tab.get("value") == 2);
  BOOST_TEST(tab.size() == 1);
}

BOOST_AUTO_TEST_CASE(hash_table_drop_existing_and_missing_keys)
{
  StrTable tab(2);

  tab.add("one", 1);

  BOOST_CHECK(tab.drop("one"));
  BOOST_CHECK(!tab.drop("one"));
  BOOST_CHECK(!tab.has("one"));
  BOOST_CHECK(tab.find("one") == nullptr);
  BOOST_CHECK_THROW(tab.get("one"), std::out_of_range);
}

BOOST_AUTO_TEST_CASE(hash_table_handles_bucket_collisions)
{
  CollTab tab(2, 2);

  tab.add("a", 1);
  tab.add("b", 2);
  tab.add("c", 3);

  BOOST_TEST(tab.get("a") == 1);
  BOOST_TEST(tab.get("b") == 2);
  BOOST_TEST(tab.get("c") == 3);
  BOOST_TEST(tab.size() == 3);
}

BOOST_AUTO_TEST_CASE(hash_table_reports_overflow_when_full)
{
  CollTab tab(1, 2);

  tab.add("a", 1);
  tab.add("b", 2);
  tab.add("c", 3);

  BOOST_CHECK_THROW(tab.add("d", 4), std::overflow_error);
  BOOST_CHECK_THROW(tab.set("new", 5), std::overflow_error);
}

BOOST_AUTO_TEST_CASE(hash_table_reuses_cell_after_drop)
{
  CollTab tab(1, 2);

  tab.add("a", 1);
  tab.add("b", 2);
  BOOST_CHECK(tab.drop("a"));

  BOOST_CHECK(tab.add("c", 3));
  BOOST_TEST(tab.get("c") == 3);
  BOOST_TEST(tab.get("b") == 2);
  BOOST_TEST(tab.size() == 2);
}

BOOST_AUTO_TEST_CASE(hash_table_iterator_visits_all_entries)
{
  StrTable tab(4);
  tab.add("one", 1);
  tab.add("two", 2);
  tab.add("three", 3);

  int sum = 0;
  std::size_t count = 0;
  for (StrTable::iterator it = tab.begin(); it != tab.end(); ++it)
  {
    sum += it->val;
    ++count;
  }

  BOOST_TEST(count == 3);
  BOOST_TEST(sum == 6);
}

BOOST_AUTO_TEST_CASE(hash_table_const_iterator_visits_all_entries)
{
  StrTable tab(4);
  tab.add("one", 1);
  tab.add("two", 2);
  const StrTable & ctab = tab;

  int sum = 0;
  for (StrTable::const_iterator it = ctab.begin(); it != ctab.end(); ++it)
  {
    sum += it->val;
  }

  BOOST_TEST(sum == 3);
}

BOOST_AUTO_TEST_CASE(hash_table_iterator_skips_dropped_entries)
{
  StrTable tab(4);
  tab.add("one", 1);
  tab.add("two", 2);
  tab.drop("one");

  std::size_t count = 0;
  for (StrTable::iterator it = tab.begin(); it != tab.end(); ++it)
  {
    BOOST_TEST(it->key != "one");
    ++count;
  }

  BOOST_TEST(count == 1);
}

BOOST_AUTO_TEST_CASE(hash_table_rehash_preserves_elements)
{
  CollTab tab(1, 4);
  tab.add("a", 1);
  tab.add("b", 2);
  tab.add("c", 3);

  tab.rehash(4);

  BOOST_TEST(tab.slots() == 4);
  BOOST_TEST(tab.get("a") == 1);
  BOOST_TEST(tab.get("b") == 2);
  BOOST_TEST(tab.get("c") == 3);
  BOOST_TEST(tab.size() == 3);
}

BOOST_AUTO_TEST_CASE(hash_table_rehash_rejects_bad_slot_count)
{
  StrTable tab(2);
  tab.add("one", 1);

  BOOST_CHECK_THROW(tab.rehash(0), std::invalid_argument);
}

BOOST_AUTO_TEST_CASE(hash_table_copy_is_independent)
{
  StrTable src(4);
  src.add("one", 1);
  src.add("two", 2);

  StrTable copy(src);
  copy.set("one", 10);
  copy.add("three", 3);

  BOOST_TEST(src.get("one") == 1);
  BOOST_TEST(src.size() == 2);
  BOOST_CHECK(!src.has("three"));
  BOOST_TEST(copy.get("one") == 10);
  BOOST_TEST(copy.get("three") == 3);
}

BOOST_AUTO_TEST_CASE(hash_table_copy_assignment_is_independent)
{
  StrTable src(4);
  src.add("one", 1);

  StrTable dst(2);
  dst.add("old", 99);
  dst = src;
  dst.set("one", 10);

  BOOST_TEST(src.get("one") == 1);
  BOOST_CHECK(!src.has("old"));
  BOOST_TEST(dst.get("one") == 10);
}

BOOST_AUTO_TEST_CASE(hash_table_move_transfers_entries)
{
  StrTable src(4);
  src.add("one", 1);
  src.add("two", 2);

  StrTable moved(std::move(src));

  BOOST_TEST(moved.get("one") == 1);
  BOOST_TEST(moved.get("two") == 2);
  BOOST_TEST(moved.size() == 2);
  BOOST_CHECK(src.empty());
  BOOST_TEST(src.size() == 0);
}

BOOST_AUTO_TEST_CASE(hash_table_move_assignment_transfers_entries)
{
  StrTable src(4);
  src.add("one", 1);

  StrTable dst(2);
  dst.add("old", 99);
  dst = std::move(src);

  BOOST_TEST(dst.get("one") == 1);
  BOOST_CHECK(!dst.has("old"));
  BOOST_CHECK(src.empty());
}
