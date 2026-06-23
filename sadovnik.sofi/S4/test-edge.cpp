#include <boost/test/unit_test.hpp>

#include "commands.hpp"
#include "io.hpp"

#include <sstream>
#include <string>

namespace
{

  sadovnik::DatasetTab makeDatasets()
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
    return datasets;
  }

  std::string run(sadovnik::DatasetTab & datasets, const std::string & line)
  {
    sadovnik::CommandContext context(datasets);
    const sadovnik::CommandTab commands = sadovnik::createCommandTable();
    std::ostringstream out;
    sadovnik::executeCommandLine(context, commands, line, out);
    return out.str();
  }

  std::string runScript(sadovnik::DatasetTab & datasets, const std::string & script)
  {
    sadovnik::CommandContext context(datasets);
    const sadovnik::CommandTab commands = sadovnik::createCommandTable();
    std::istringstream input(script);
    std::ostringstream output;
    sadovnik::processCommands(context, commands, input, output);
    return output.str();
  }

}

BOOST_AUTO_TEST_CASE(edge_print_empty_after_intersect_with_no_overlap)
{
  sadovnik::DatasetTab datasets(4);

  sadovnik::IntStrTree left;
  left.push(1, "L1");

  sadovnik::IntStrTree right;
  right.push(2, "R2");

  datasets.add("left", left);
  datasets.add("right", right);

  BOOST_TEST(run(datasets, "intersect nodata left right") == "");
  BOOST_TEST(run(datasets, "print nodata") == "<EMPTY>\n");
}

BOOST_AUTO_TEST_CASE(edge_print_empty_after_complement_with_full_overlap)
{
  std::istringstream file("first 1 name 2 surname\nsecond 1 name 2 keyboard 4 mouse\n");
  sadovnik::DatasetTab datasets = sadovnik::readDatasets(file);

  BOOST_TEST(run(datasets, "complement diff first second") == "");
  BOOST_TEST(run(datasets, "print diff") == "<EMPTY>\n");
}

BOOST_AUTO_TEST_CASE(edge_assignment_file_and_commands_scenario)
{
  std::istringstream file(
    "first 1 name 2 surname\n"
    "second 4 mouse 1 name 2 keyboard\n");

  sadovnik::DatasetTab datasets = sadovnik::readDatasets(file);
  const std::string output = runScript(
    datasets,
    "print first\n"
    "print second\n"
    "intersect common first second\n"
    "print common\n"
    "union all first second\n"
    "print all\n");

  BOOST_TEST(output ==
    "first 1 name 2 surname\n"
    "second 1 name 2 keyboard 4 mouse\n"
    "common 1 name 2 surname\n"
    "all 1 name 2 surname 4 mouse\n");
}

BOOST_AUTO_TEST_CASE(edge_rejects_wrong_token_count_for_each_command)
{
  sadovnik::DatasetTab datasets = makeDatasets();

  BOOST_TEST(run(datasets, "print") == "<INVALID COMMAND>\n");
  BOOST_TEST(run(datasets, "print left extra") == "<INVALID COMMAND>\n");
  BOOST_TEST(run(datasets, "complement new left") == "<INVALID COMMAND>\n");
  BOOST_TEST(run(datasets, "intersect new left") == "<INVALID COMMAND>\n");
  BOOST_TEST(run(datasets, "union new left") == "<INVALID COMMAND>\n");
}

BOOST_AUTO_TEST_CASE(edge_rejects_invalid_dataset_names_in_commands)
{
  sadovnik::DatasetTab datasets = makeDatasets();

  BOOST_TEST(run(datasets, "print 1bad") == "<INVALID COMMAND>\n");
  BOOST_TEST(run(datasets, "union 1bad left right") == "<INVALID COMMAND>\n");
  BOOST_TEST(run(datasets, "complement new left 2bad") == "<INVALID COMMAND>\n");
}

BOOST_AUTO_TEST_CASE(edge_ignores_whitespace_only_command_lines)
{
  sadovnik::DatasetTab datasets = makeDatasets();

  BOOST_TEST(runScript(datasets, "  \t  \nprint left\n") == "left 1 L1 3 L3\n");
}

BOOST_AUTO_TEST_CASE(edge_invalid_command_is_case_sensitive)
{
  sadovnik::DatasetTab datasets = makeDatasets();

  BOOST_TEST(run(datasets, "Print left") == "<INVALID COMMAND>\n");
  BOOST_TEST(run(datasets, "UNION new left right") == "<INVALID COMMAND>\n");
}

BOOST_AUTO_TEST_CASE(edge_union_with_empty_dataset)
{
  sadovnik::DatasetTab datasets(4);
  sadovnik::IntStrTree empty;
  sadovnik::IntStrTree tree;
  tree.push(2, "two");

  datasets.add("empty", empty);
  datasets.add("data", tree);

  BOOST_TEST(run(datasets, "union merged empty data") == "");
  BOOST_TEST(run(datasets, "print merged") == "merged 2 two\n");
  BOOST_TEST(run(datasets, "print empty") == "<EMPTY>\n");
}

BOOST_AUTO_TEST_CASE(edge_values_may_contain_digits)
{
  std::istringstream file("data 10 v42 20 v99\n");
  sadovnik::DatasetTab datasets = sadovnik::readDatasets(file);

  BOOST_TEST(run(datasets, "print data") == "data 10 v42 20 v99\n");
}
