#include <boost/test/unit_test.hpp>

#include "commands.hpp"
#include "io.hpp"

#include <sstream>
#include <string>

namespace
{

  sadovnik::GraphTab makeGraphs()
  {
    sadovnik::GraphTab graphs(4);
    sadovnik::Graph graph;
    graph.bind("a", "b", 3);
    graph.bind("a", "b", 1);
    graph.bind("c", "b", 2);
    graphs.add("g1", graph);
    return graphs;
  }

  std::string run(sadovnik::GraphTab & graphs, const std::string & line)
  {
    sadovnik::CommandContext context(graphs);
    const sadovnik::CommandTab commands = sadovnik::createCommandTable();
    std::ostringstream out;
    sadovnik::executeCommandLine(context, commands, line, out);
    return out.str();
  }

}

BOOST_AUTO_TEST_CASE(commands_print_graphs_and_vertexes_sorted)
{
  sadovnik::GraphTab graphs = makeGraphs();
  graphs.add("a2", sadovnik::Graph());

  BOOST_TEST(run(graphs, "graphs") == "a2\ng1\n");
  BOOST_TEST(run(graphs, "vertexes g1") == "a\nb\nc\n");
}

BOOST_AUTO_TEST_CASE(commands_print_outbound_and_inbound_sorted)
{
  sadovnik::GraphTab graphs = makeGraphs();

  BOOST_TEST(run(graphs, "outbound g1 a") == "b 1 3\n");
  BOOST_TEST(run(graphs, "inbound g1 b") == "a 1 3\nc 2\n");
}

BOOST_AUTO_TEST_CASE(commands_bind_and_cut_change_graph)
{
  sadovnik::GraphTab graphs = makeGraphs();

  BOOST_TEST(run(graphs, "bind g1 b d 0") == "");
  BOOST_TEST(run(graphs, "outbound g1 b") == "d 0\n");
  BOOST_TEST(run(graphs, "cut g1 a b 1") == "");
  BOOST_TEST(run(graphs, "outbound g1 a") == "b 3\n");
}

BOOST_AUTO_TEST_CASE(commands_create_accepts_empty_or_counted_graph)
{
  sadovnik::GraphTab graphs = makeGraphs();

  BOOST_TEST(run(graphs, "create empty") == "");
  BOOST_TEST(run(graphs, "create g2 2 x y") == "");
  BOOST_TEST(run(graphs, "vertexes g2") == "x\ny\n");
}

BOOST_AUTO_TEST_CASE(commands_merge_and_extract_create_new_graphs)
{
  sadovnik::GraphTab graphs = makeGraphs();

  BOOST_TEST(run(graphs, "create g2 1 d") == "");
  BOOST_TEST(run(graphs, "bind g2 d a 4") == "");
  BOOST_TEST(run(graphs, "merge merged g1 g2") == "");
  BOOST_TEST(run(graphs, "outbound merged d") == "a 4\n");
  BOOST_TEST(run(graphs, "extract part merged 2 a b") == "");
  BOOST_TEST(run(graphs, "vertexes part") == "a\nb\n");
}

BOOST_AUTO_TEST_CASE(commands_report_invalid_command)
{
  sadovnik::GraphTab graphs = makeGraphs();

  BOOST_TEST(run(graphs, "unknown") == "invalid\n");
  BOOST_TEST(run(graphs, "vertexes missing") == "invalid\n");
  BOOST_TEST(run(graphs, "create bad 2 a") == "invalid\n");
  BOOST_TEST(run(graphs, "cut g1 a b 99") == "invalid\n");
}

BOOST_AUTO_TEST_CASE(commands_ignore_empty_lines)
{
  sadovnik::GraphTab graphs = makeGraphs();

  BOOST_TEST(run(graphs, "") == "");
  BOOST_TEST(run(graphs, "outbound g1 a") == "b 1 3\n");
}

BOOST_AUTO_TEST_CASE(commands_process_integration_scenario)
{
  std::istringstream file("g1 2\na b 3\na c 1\n");
  sadovnik::GraphTab graphs = sadovnik::readGraphs(file);
  sadovnik::CommandContext context(graphs);
  const sadovnik::CommandTab commands = sadovnik::createCommandTable();
  std::istringstream input("graphs\noutbound g1 a\ncreate empty\nbad command\n");
  std::ostringstream output;

  sadovnik::processCommands(context, commands, input, output);

  BOOST_TEST(output.str() == "g1\nb 3\nc 1\ninvalid\n");
  BOOST_CHECK(graphs.has("empty"));
}

BOOST_AUTO_TEST_CASE(commands_reject_invalid_graph_and_vertex_names)
{
  sadovnik::GraphTab graphs = makeGraphs();

  BOOST_TEST(run(graphs, "create 1bad") == "invalid\n");
  BOOST_TEST(run(graphs, "create good 1 bad-name") == "invalid\n");
  BOOST_TEST(run(graphs, "bind g1 a bad-name 1") == "invalid\n");
}

BOOST_AUTO_TEST_CASE(commands_reject_duplicate_create_name)
{
  sadovnik::GraphTab graphs = makeGraphs();

  BOOST_TEST(run(graphs, "create g1") == "invalid\n");
}

BOOST_AUTO_TEST_CASE(commands_merge_same_graph_into_new_graph)
{
  sadovnik::GraphTab graphs = makeGraphs();

  BOOST_TEST(run(graphs, "merge doubled g1 g1") == "");
  BOOST_TEST(run(graphs, "outbound doubled a") == "b 1 1 3 3\n");
}

BOOST_AUTO_TEST_CASE(commands_extract_subset_drops_external_edges)
{
  sadovnik::GraphTab graphs = makeGraphs();

  BOOST_TEST(run(graphs, "extract onlyab g1 2 a b") == "");
  BOOST_TEST(run(graphs, "vertexes onlyab") == "a\nb\n");
  BOOST_TEST(run(graphs, "inbound onlyab b") == "a 1 3\n");
  BOOST_TEST(run(graphs, "inbound onlyab c") == "invalid\n");
}

BOOST_AUTO_TEST_CASE(commands_keep_multiple_equal_weights_sorted)
{
  sadovnik::GraphTab graphs = makeGraphs();

  BOOST_TEST(run(graphs, "bind g1 a b 1") == "");
  BOOST_TEST(run(graphs, "bind g1 a b 2") == "");
  BOOST_TEST(run(graphs, "outbound g1 a") == "b 1 1 2 3\n");
}

BOOST_AUTO_TEST_CASE(commands_support_empty_created_graph)
{
  sadovnik::GraphTab graphs = makeGraphs();

  BOOST_TEST(run(graphs, "create gr3") == "");
  BOOST_TEST(run(graphs, "vertexes gr3") == "\n");
}

BOOST_AUTO_TEST_CASE(commands_print_empty_line_for_empty_results)
{
  sadovnik::GraphTab graphs(4);

  BOOST_TEST(run(graphs, "graphs") == "\n");
  BOOST_TEST(run(graphs, "create gr2") == "");
  BOOST_TEST(run(graphs, "vertexes gr2") == "\n");
  BOOST_TEST(run(graphs, "bind gr2 d a 0") == "");
  BOOST_TEST(run(graphs, "outbound gr2 d") == "a 0\n");
  BOOST_TEST(run(graphs, "cut gr2 d a 0") == "");
  BOOST_TEST(run(graphs, "outbound gr2 d") == "\n");
}
