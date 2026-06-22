#include <boost/test/unit_test.hpp>

#include <hash-functions.hpp>
#include <hash-table.hpp>

namespace
{

  using StrTab = sadovnik::HashTable< std::string, int, sadovnik::StrHash, sadovnik::StrEq >;
  using EdgeTab = sadovnik::HashTable< sadovnik::EdgeKey, int, sadovnik::EdgeKeyHash, sadovnik::EdgeKeyEq >;

}

BOOST_AUTO_TEST_CASE(str_hash_is_stable)
{
  const sadovnik::StrHash hash;
  const std::string key = "vertex";

  BOOST_TEST(hash(key) == hash(key));
}

BOOST_AUTO_TEST_CASE(str_hash_differs_for_different_keys)
{
  const sadovnik::StrHash hash;

  BOOST_TEST(hash("alpha") != hash("beta"));
}

BOOST_AUTO_TEST_CASE(str_eq_compares_strings)
{
  const sadovnik::StrEq eq;

  BOOST_TEST(eq("same", "same"));
  BOOST_CHECK(!eq("left", "right"));
}

BOOST_AUTO_TEST_CASE(edge_key_eq_compares_both_fields)
{
  const sadovnik::EdgeKeyEq eq;
  const sadovnik::EdgeKey lhs = {"a", "b"};
  const sadovnik::EdgeKey rhs = {"a", "c"};
  const sadovnik::EdgeKey same = {"a", "b"};

  BOOST_TEST(eq(lhs, same));
  BOOST_CHECK(!eq(lhs, rhs));
}

BOOST_AUTO_TEST_CASE(edge_key_hash_depends_on_direction)
{
  const sadovnik::EdgeKeyHash hash;
  const sadovnik::EdgeKey ab = {"a", "b"};
  const sadovnik::EdgeKey ba = {"b", "a"};

  BOOST_TEST(hash(ab) != hash(ba));
}

BOOST_AUTO_TEST_CASE(edge_key_hash_is_stable)
{
  const sadovnik::EdgeKeyHash hash;
  const sadovnik::EdgeKey key = {"from", "to"};

  BOOST_TEST(hash(key) == hash(key));
}

BOOST_AUTO_TEST_CASE(hash_table_accepts_blake2_string_hash)
{
  StrTab tab(7);

  tab.add("graphs", 1);
  tab.add("vertexes", 2);

  BOOST_TEST(tab.has("graphs"));
  BOOST_TEST(tab.get("vertexes") == 2);
  BOOST_CHECK(!tab.has("missing"));
}

BOOST_AUTO_TEST_CASE(hash_table_accepts_blake2_edge_key_hash)
{
  EdgeTab tab(5);
  const sadovnik::EdgeKey edge = {"x", "y"};

  tab.add(edge, 10);

  BOOST_TEST(tab.has(edge));
  BOOST_TEST(tab.get(edge) == 10);
}
