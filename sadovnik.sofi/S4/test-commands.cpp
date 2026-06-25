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
    left.push(5, "L5");

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

}

BOOST_AUTO_TEST_CASE(commands_print_dataset_in_key_order)
{
  sadovnik::DatasetTab datasets = makeDatasets();

  BOOST_TEST(run(datasets, "print left") == "left 1 L1 3 L3 5 L5\n");
}

BOOST_AUTO_TEST_CASE(commands_print_empty_dataset)
{
  sadovnik::DatasetTab datasets(4);
  sadovnik::IntStrTree empty;
  datasets.add("empty", empty);

  BOOST_TEST(run(datasets, "print empty") == "<EMPTY>\n");
}

BOOST_AUTO_TEST_CASE(commands_complement_creates_new_dataset)
{
  sadovnik::DatasetTab datasets = makeDatasets();

  BOOST_TEST(run(datasets, "complement diff left right") == "");
  BOOST_CHECK(datasets.has("diff"));
  BOOST_TEST(datasets.get("diff").get(1) == "L1");
  BOOST_TEST(datasets.get("diff").get(5) == "L5");
  BOOST_CHECK(!datasets.get("diff").has(3));
}

BOOST_AUTO_TEST_CASE(commands_intersect_uses_left_values)
{
  sadovnik::DatasetTab datasets = makeDatasets();

  BOOST_TEST(run(datasets, "intersect common left right") == "");
  BOOST_TEST(datasets.get("common").get(3) == "L3");
}

BOOST_AUTO_TEST_CASE(commands_union_prefers_left_on_conflict)
{
  sadovnik::DatasetTab datasets = makeDatasets();

  BOOST_TEST(run(datasets, "union merged left right") == "");
  BOOST_TEST(datasets.get("merged").get(3) == "L3");
  BOOST_TEST(datasets.get("merged").get(4) == "R4");
  BOOST_TEST(datasets.get("merged").size() == 4);
}

BOOST_AUTO_TEST_CASE(commands_report_invalid_command)
{
  sadovnik::DatasetTab datasets = makeDatasets();

  BOOST_TEST(run(datasets, "unknown") == "<INVALID COMMAND>\n");
  BOOST_TEST(run(datasets, "print missing") == "<INVALID COMMAND>\n");
  BOOST_TEST(run(datasets, "union left left right") == "<INVALID COMMAND>\n");
  BOOST_TEST(run(datasets, "complement left left right") == "<INVALID COMMAND>\n");
}

BOOST_AUTO_TEST_CASE(commands_ignore_empty_lines)
{
  sadovnik::DatasetTab datasets = makeDatasets();

  BOOST_TEST(run(datasets, "") == "");
  BOOST_TEST(run(datasets, "print left") == "left 1 L1 3 L3 5 L5\n");
}

BOOST_AUTO_TEST_CASE(commands_process_integration_scenario)
{
  std::istringstream file("first 1 one 2 two\nsecond 3 three\n");
  sadovnik::DatasetTab datasets = sadovnik::readDatasets(file);
  sadovnik::CommandContext context(datasets);
  const sadovnik::CommandTab commands = sadovnik::createCommandTable();
  std::istringstream input("print first\nprint second\nbad\n");
  std::ostringstream output;

  sadovnik::processCommands(context, commands, input, output);

  BOOST_TEST(output.str() == "first 1 one 2 two\nsecond 3 three\n<INVALID COMMAND>\n");
}
