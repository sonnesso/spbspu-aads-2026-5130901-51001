#ifndef F0_COMMANDS_HPP
#define F0_COMMANDS_HPP

#include "session.hpp"

#include <functional>
#include <iosfwd>
#include <string>

#include <list.hpp>

namespace sadovnik
{

  class CommandContext
  {
  public:
    explicit CommandContext(Session & session);

    Session & session();
    const Session & session() const;

  private:
    Session & session_;
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
