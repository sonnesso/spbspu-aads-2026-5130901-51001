#include "commands.hpp"

#include "session-types.hpp"
#include "strategy-ops.hpp"
#include "tyre-ops.hpp"

#include <string-utils.hpp>

#include <istream>
#include <string>

namespace
{

  using sadovnik::CommandContext;
  using sadovnik::List;
  using sadovnik::Stint;
  using sadovnik::StrategyTab;
  using sadovnik::TyreSpec;

  const char INVALID_COMMAND[] = "<INVALID COMMAND>";

  std::string tokenAt(const List< std::string > & tokens, std::size_t index)
  {
    std::size_t cur = 0;
    for (auto it = tokens.begin(); it != tokens.end(); ++it)
    {
      if (cur == index)
      {
        return *it;
      }
      ++cur;
    }

    throw std::logic_error("missing token");
  }

  bool parsePositiveUnsigned(const std::string & value, unsigned & result)
  {
    unsigned long long parsed = 0;
    if (!sadovnik::parseUnsigned(value, parsed) || parsed == 0)
    {
      return false;
    }

    result = static_cast< unsigned >(parsed);
    return true;
  }

  bool stubCmd(CommandContext &, const List< std::string > &, std::ostream &)
  {
    return false;
  }

  bool setTrackCmd(CommandContext & context, const List< std::string > & tokens,
                   std::ostream & out)
  {
    if (tokens.size() < 2 || tokens.size() > 3)
    {
      return false;
    }

    unsigned laps = 0;
    double base_lap_s = 90.0;

    if (!parsePositiveUnsigned(tokenAt(tokens, 1), laps))
    {
      return false;
    }

    if (tokens.size() == 3)
    {
      if (!sadovnik::parseDouble(tokenAt(tokens, 2), base_lap_s) ||
          base_lap_s <= 0.0)
      {
        return false;
      }
    }

    context.session().setTrack(0.0, laps, base_lap_s);
    sadovnik::printTrackSetLine(context.session().track(), out);
    return true;
  }

  bool addTyreCmd(CommandContext & context, const List< std::string > & tokens,
                  std::ostream &)
  {
    std::string name;
    TyreSpec spec;
    if (!sadovnik::parseAddTyreTokens(tokens, name, spec))
    {
      return false;
    }

    sadovnik::TyreTab & tyres = context.session().tyres();
    if (tyres.has(name))
    {
      return false;
    }

    try
    {
      tyres.add(name, spec);
    }
    catch (const std::exception &)
    {
      return false;
    }

    context.session().addTyreName(name);
    context.session().markDirty();
    return true;
  }

  bool showTyresCmd(CommandContext & context, const List< std::string > & tokens,
                    std::ostream & out)
  {
    if (tokens.size() != 1)
    {
      return false;
    }

    sadovnik::printTyres(context.session(), out);
    return true;
  }

  bool createStrategyCmd(CommandContext & context,
                         const List< std::string > & tokens, std::ostream & out)
  {
    std::string name;
    List< Stint > stints;
    if (!sadovnik::parseCreateStrategyTokens(tokens, name, stints))
    {
      return false;
    }

    if (!sadovnik::isCreateStrategyStintsValid(context.session(), stints))
    {
      return false;
    }

    StrategyTab & strategies = context.session().strategies();
    if (strategies.has(name))
    {
      return false;
    }

    try
    {
      strategies.add(name, stints);
    }
    catch (const std::exception &)
    {
      return false;
    }

    context.session().markDirty();
    context.session().addStrategyName(name);
    sadovnik::printStrategyCreatedLine(name, stints, out);
    return true;
  }

  bool listStrategiesCmd(CommandContext & context,
                         const List< std::string > & tokens, std::ostream & out)
  {
    if (tokens.size() != 1)
    {
      return false;
    }

    sadovnik::printStrategies(context.session(), out);
    return true;
  }

  bool delStrategyCmd(CommandContext & context, const List< std::string > & tokens,
                      std::ostream & out)
  {
    if (tokens.size() != 2)
    {
      return false;
    }

    const std::string name = tokenAt(tokens, 1);
    if (!sadovnik::isValidName(name))
    {
      return false;
    }

    return sadovnik::deleteStrategy(context.session(), name, out);
  }

  bool validateCmd(CommandContext & context, const List< std::string > & tokens,
                   std::ostream & out)
  {
    if (tokens.size() != 2)
    {
      return false;
    }

    const std::string name = tokenAt(tokens, 1);
    if (!sadovnik::isValidName(name))
    {
      return false;
    }

    return sadovnik::validateStrategy(context.session(), name, out);
  }

  bool simulateCmd(CommandContext & context, const List< std::string > & tokens,
                   std::ostream & out)
  {
    if (tokens.size() != 2)
    {
      return false;
    }

    const std::string name = tokenAt(tokens, 1);
    if (!sadovnik::isValidName(name))
    {
      return false;
    }

    return sadovnik::simulateStrategy(context.session(), name, out);
  }

  bool compareCmd(CommandContext & context, const List< std::string > & tokens,
                  std::ostream & out)
  {
    if (tokens.size() < 3)
    {
      return false;
    }

    List< std::string > names;
    for (std::size_t pos = 1; pos < tokens.size(); ++pos)
    {
      const std::string name = tokenAt(tokens, pos);
      if (!sadovnik::isValidName(name))
      {
        return false;
      }
      names.pushBack(name);
    }

    return sadovnik::compareStrategies(context.session(), names, out);
  }

}

namespace sadovnik
{

  CommandContext::CommandContext(Session & session)
    : session_(session)
  {
  }

  Session & CommandContext::session()
  {
    return session_;
  }

  const Session & CommandContext::session() const
  {
    return session_;
  }

  CommandTab createCommandTable()
  {
    CommandTab commands(32, 4);
    commands.add("set-track", setTrackCmd);
    commands.add("add-tyre", addTyreCmd);
    commands.add("create-strategy", createStrategyCmd);
    commands.add("simulate", simulateCmd);
    commands.add("compare", compareCmd);
    commands.add("optimal-pit-window", stubCmd);
    commands.add("show-tyres", showTyresCmd);
    commands.add("list-strategies", listStrategiesCmd);
    commands.add("del-strategy", delStrategyCmd);
    commands.add("save-session", stubCmd);
    commands.add("load-session", stubCmd);
    commands.add("load-session-force", stubCmd);
    commands.add("load-preset", stubCmd);
    commands.add("validate", validateCmd);
    commands.add("suggest-strategies", stubCmd);
    commands.add("set-weather", stubCmd);
    commands.add("set-hum", stubCmd);
    commands.add("crossover-check", stubCmd);
    return commands;
  }

  bool executeCommandLine(CommandContext & context, const CommandTab & commands,
                          const std::string & line, std::ostream & out)
  {
    const List< std::string > tokens = splitTokens(line);
    if (tokens.empty())
    {
      return true;
    }

    const CommandHandler * handler = commands.find(tokenAt(tokens, 0));
    if (handler == nullptr)
    {
      out << INVALID_COMMAND << '\n';
      return false;
    }

    bool ok = false;
    try
    {
      ok = (*handler)(context, tokens, out);
    }
    catch (const std::exception &)
    {
      ok = false;
    }

    if (!ok)
    {
      out << INVALID_COMMAND << '\n';
    }

    return ok;
  }

  void processCommands(CommandContext & context, const CommandTab & commands,
                       std::istream & in, std::ostream & out)
  {
    std::string line;
    while (true)
    {
      std::getline(in, line);
      if (line.empty() && in.eof())
      {
        break;
      }
      if (in.fail() && line.empty())
      {
        break;
      }

      executeCommandLine(context, commands, line, out);

      if (in.eof())
      {
        break;
      }
    }
  }

}
