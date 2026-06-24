#include "commands.hpp"

#include "session-types.hpp"

#include <string-utils.hpp>

#include <iomanip>
#include <istream>
#include <string>

namespace
{

  using sadovnik::CommandContext;
  using sadovnik::List;
  using sadovnik::TyreKind;
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
    if (tokens.size() < 3 || tokens.size() > 4)
    {
      return false;
    }

    double length_km = 0.0;
    unsigned laps = 0;
    double base_lap_s = 90.0;

    if (!sadovnik::parseDouble(tokenAt(tokens, 1), length_km) || length_km <= 0.0)
    {
      return false;
    }

    if (!parsePositiveUnsigned(tokenAt(tokens, 2), laps))
    {
      return false;
    }

    if (tokens.size() == 4)
    {
      if (!sadovnik::parseDouble(tokenAt(tokens, 3), base_lap_s) || base_lap_s <= 0.0)
      {
        return false;
      }
    }

    context.session().setTrack(length_km, laps, base_lap_s);

    out << std::fixed << std::setprecision(3);
    out << "Track set: " << length_km << " km, " << laps << " laps, base lap "
        << base_lap_s << " s\n";
    return true;
  }

  bool addTyreCmd(CommandContext & context, const List< std::string > & tokens,
                  std::ostream &)
  {
    if (tokens.size() != 6 && tokens.size() != 7)
    {
      return false;
    }

    const std::string name = tokenAt(tokens, 1);
    if (!sadovnik::isValidName(name))
    {
      return false;
    }

    TyreKind kind = TyreKind::Slick;
    if (!sadovnik::parseTyreKind(tokenAt(tokens, 2), kind))
    {
      return false;
    }

    double degr = 0.0;
    if (!sadovnik::parseDouble(tokenAt(tokens, 3), degr) || degr < 0.0)
    {
      return false;
    }

    unsigned max_laps = 0;
    unsigned pit_time = 0;
    if (!parsePositiveUnsigned(tokenAt(tokens, 4), max_laps))
    {
      return false;
    }
    if (!parsePositiveUnsigned(tokenAt(tokens, 5), pit_time))
    {
      return false;
    }

    std::string compound;
    if (tokens.size() == 7)
    {
      if (kind != TyreKind::Slick)
      {
        return false;
      }
      if (!sadovnik::parseCompound(tokenAt(tokens, 6), compound))
      {
        return false;
      }
    }

    sadovnik::TyreTab & tyres = context.session().tyres();
    if (tyres.has(name))
    {
      return false;
    }

    TyreSpec spec;
    spec.kind = kind;
    spec.compound = compound;
    spec.degr = degr;
    spec.max_laps = max_laps;
    spec.pit_time = pit_time;
    spec.base_offset = sadovnik::baseOffsetForTyre(kind, compound);

    tyres.add(name, spec);
    context.session().markDirty();
    return true;
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
    commands.add("create-strategy", stubCmd);
    commands.add("simulate", stubCmd);
    commands.add("compare", stubCmd);
    commands.add("optimal-pit-window", stubCmd);
    commands.add("show-tyres", stubCmd);
    commands.add("list-strategies", stubCmd);
    commands.add("del-strategy", stubCmd);
    commands.add("save-session", stubCmd);
    commands.add("load-session", stubCmd);
    commands.add("load-session-force", stubCmd);
    commands.add("load-preset", stubCmd);
    commands.add("validate", stubCmd);
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
