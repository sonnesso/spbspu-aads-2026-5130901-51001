#include "commands.hpp"

#include <iostream>

int main()
{
  try
  {
    sadovnik::Session session;
    sadovnik::CommandContext context(session);
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
