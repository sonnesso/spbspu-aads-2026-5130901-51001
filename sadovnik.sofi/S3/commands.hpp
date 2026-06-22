#ifndef COMMANDS_HPP
#define COMMANDS_HPP

#include "io.hpp"

#include <functional>
#include <iosfwd>
#include <string>

namespace sadovnik
{

  class CommandContext
  {
  public:
    explicit CommandContext(GraphTab & graphs);

    GraphTab & graphs();
    const GraphTab & graphs() const;

  private:
    GraphTab & graphs_;
  };

  using CommandHandler =
    std::function< bool(CommandContext &, const List< std::string > &, std::ostream &) >;
  using CommandTab = HashTable< std::string, CommandHandler, StrHash, StrEq >;

  CommandTab createCommandTable();
  bool executeCommandLine(CommandContext & context, const CommandTab & commands,
                          const std::string & line, std::ostream & out);
  void processCommands(CommandContext & context, const CommandTab & commands,
                       std::istream & in, std::ostream & out);

}

#endif
