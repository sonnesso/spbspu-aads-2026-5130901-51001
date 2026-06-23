#include "commands.hpp"
#include "io.hpp"

#include <fstream>
#include <iostream>

int main(int argc, char * argv[])
{
  if (argc != 2)
  {
    std::cerr << "filename required\n";
    return 1;
  }

  std::ifstream file(argv[1]);
  if (!file)
  {
    std::cerr << "cannot open file\n";
    return 1;
  }

  try
  {
    sadovnik::DatasetTab datasets = sadovnik::readDatasets(file);
    sadovnik::CommandContext context(datasets);
    const sadovnik::CommandTab commands = sadovnik::createCommandTable();
    sadovnik::processCommands(context, commands, std::cin, std::cout);
  }
  catch (const std::exception & error)
  {
    std::cerr << error.what() << '\n';
    return 2;
  }

  return 0;
}
