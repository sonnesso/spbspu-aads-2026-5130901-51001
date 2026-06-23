#include <boost/test/unit_test.hpp>

#include "commands.hpp"
#include "io.hpp"

#include <sstream>

BOOST_AUTO_TEST_CASE(s4_smoke_runs_loader_and_commands)
{
  std::istringstream file("alpha 1 one\nbeta 2 two\n");
  sadovnik::DatasetTab datasets = sadovnik::readDatasets(file);

  sadovnik::CommandContext context(datasets);
  const sadovnik::CommandTab commands = sadovnik::createCommandTable();

  std::istringstream input("print alpha\nunion merged alpha beta\nprint merged\n");
  std::ostringstream output;

  sadovnik::processCommands(context, commands, input, output);

  BOOST_TEST(output.str() == "alpha 1 one\nmerged 1 one 2 two\n");
}
