#include <boost/test/unit_test.hpp>

#include "graph.hpp"

#include <string>
#include <vector>

namespace
{

  std::vector< std::string > toVec(const sadovnik::List< std::string > & list)
  {
    std::vector< std::string > result;
    for (auto it = list.begin(); it != list.end(); ++it)
    {
      result.push_back(*it);
    }
    return result;
  }

  std::vector< unsigned long long > weightsOf(
    const sadovnik::Graph::WeightedVertex & item)
  {
    std::vector< unsigned long long > result;
    for (auto it = item.weights.begin(); it != item.weights.end(); ++it)
    {
      result.push_back(*it);
    }
    return result;
  }

}

BOOST_AUTO_TEST_CASE(graph_bind_adds_vertices_and_edges)
{
  sadovnik::Graph graph;

  graph.bind("b", "a", 5);

  BOOST_CHECK(graph.hasVertex("a"));
  BOOST_CHECK(graph.hasVertex("b"));
  BOOST_TEST(graph.vertexCount() == 2);
  BOOST_TEST(graph.edgeKeyCount() == 1);
}

BOOST_AUTO_TEST_CASE(graph_cut_removes_one_matching_weight)
{
  sadovnik::Graph graph;
  graph.bind("a", "b", 5);
  graph.bind("a", "b", 5);

  BOOST_CHECK(graph.cut("a", "b", 5));
  BOOST_TEST(graph.edgeKeyCount() == 1);
  BOOST_CHECK(graph.cut("a", "b", 5));
  BOOST_TEST(graph.edgeKeyCount() == 0);
  BOOST_CHECK(!graph.cut("a", "b", 5));
}

BOOST_AUTO_TEST_CASE(graph_returns_sorted_vertices)
{
  sadovnik::Graph graph;
  graph.addVertex("c");
  graph.addVertex("a");
  graph.addVertex("b");

  const std::vector< std::string > expected{"a", "b", "c"};

  BOOST_TEST(toVec(graph.getVertexes()) == expected, boost::test_tools::per_element());
}

BOOST_AUTO_TEST_CASE(graph_outbound_and_inbound_are_sorted)
{
  sadovnik::Graph graph;
  graph.bind("a", "c", 7);
  graph.bind("a", "b", 4);
  graph.bind("a", "b", 2);
  graph.bind("c", "b", 9);

  auto outbound = graph.getOutbound("a");
  auto outIt = outbound.begin();
  BOOST_REQUIRE(outIt != outbound.end());
  BOOST_TEST(outIt->vertex == "b");
  const std::vector< unsigned long long > expectedB{2, 4};
  BOOST_TEST(weightsOf(*outIt) == expectedB, boost::test_tools::per_element());
  ++outIt;
  BOOST_REQUIRE(outIt != outbound.end());
  BOOST_TEST(outIt->vertex == "c");

  auto inbound = graph.getInbound("b");
  auto inIt = inbound.begin();
  BOOST_REQUIRE(inIt != inbound.end());
  BOOST_TEST(inIt->vertex == "a");
  ++inIt;
  BOOST_REQUIRE(inIt != inbound.end());
  BOOST_TEST(inIt->vertex == "c");
}

BOOST_AUTO_TEST_CASE(graph_merge_preserves_vertices_and_repeated_edges)
{
  sadovnik::Graph left;
  sadovnik::Graph right;
  left.bind("a", "b", 1);
  right.bind("a", "b", 1);
  right.bind("b", "c", 2);

  const sadovnik::Graph merged = left.merge(right);

  BOOST_TEST(merged.vertexCount() == 3);
  auto outbound = merged.getOutbound("a");
  BOOST_REQUIRE(outbound.begin() != outbound.end());
  const std::vector< unsigned long long > expected{1, 1};
  BOOST_TEST(weightsOf(*outbound.begin()) == expected, boost::test_tools::per_element());
}

BOOST_AUTO_TEST_CASE(graph_extract_keeps_only_selected_vertices_and_edges)
{
  sadovnik::Graph graph;
  graph.bind("a", "b", 1);
  graph.bind("b", "c", 2);

  sadovnik::List< std::string > vertices;
  vertices.pushBack("a");
  vertices.pushBack("b");

  const sadovnik::Graph extracted = graph.extract(vertices);

  BOOST_CHECK(extracted.hasVertex("a"));
  BOOST_CHECK(extracted.hasVertex("b"));
  BOOST_CHECK(!extracted.hasVertex("c"));
  BOOST_TEST(extracted.edgeKeyCount() == 1);
}
