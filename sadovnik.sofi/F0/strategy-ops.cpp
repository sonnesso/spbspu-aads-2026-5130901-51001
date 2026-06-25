#include "strategy-ops.hpp"

#include "tyre-math.hpp"

#include <string-utils.hpp>

#include <iomanip>
#include <ostream>
#include <string>

namespace
{

  using sadovnik::List;
  using sadovnik::Stint;
  using sadovnik::TyreKind;
  using sadovnik::TyreSpec;

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

  std::string slickCompoundKey(const TyreSpec & spec,
                               const std::string & tyre_name)
  {
    if (!spec.compound.empty())
    {
      return spec.compound;
    }

    return tyre_name;
  }

  bool hasSlickCompoundKey(const List< std::string > & keys,
                           const std::string & key)
  {
    for (auto it = keys.begin(); it != keys.end(); ++it)
    {
      if (*it == key)
      {
        return true;
      }
    }

    return false;
  }

  bool hasEnoughSlickCompoundsForDry(const sadovnik::Session & session,
                                     const List< Stint > & stints)
  {
    List< std::string > keys;
    for (auto it = stints.begin(); it != stints.end(); ++it)
    {
      const TyreSpec & spec = session.tyres().get(it->tyre_name);
      if (spec.kind != TyreKind::Slick)
      {
        continue;
      }

      const std::string key = slickCompoundKey(spec, it->tyre_name);
      if (!hasSlickCompoundKey(keys, key))
      {
        keys.pushBack(key);
      }
    }

    return keys.size() >= 2;
  }

  std::string strategyValidationError(const sadovnik::Session & session,
                                      const List< Stint > & stints)
  {
    if (!session.track().is_set)
    {
      return "track not set";
    }

    unsigned total_laps = 0;
    for (auto it = stints.begin(); it != stints.end(); ++it)
    {
      if (!session.tyres().has(it->tyre_name))
      {
        return "tyre not found";
      }

      const TyreSpec & spec = session.tyres().get(it->tyre_name);
      if (it->laps > spec.max_laps)
      {
        return "tyre limits exceeded";
      }

      total_laps += it->laps;
    }

    if (total_laps != session.track().laps)
    {
      return "laps mismatch";
    }

    if (session.weather() == sadovnik::Weather::Dry &&
        !hasEnoughSlickCompoundsForDry(session, stints))
    {
      return "must use at least 2 slick compounds in dry race";
    }

    return std::string();
  }

}

namespace sadovnik
{

  bool parseCreateStrategyTokens(const List< std::string > & tokens,
                                 std::string & name, List< Stint > & stints)
  {
    if (tokens.size() < 4)
    {
      return false;
    }

    const std::size_t stint_tokens = tokens.size() - 2;
    if (stint_tokens % 2 != 0)
    {
      return false;
    }

    name = tokenAt(tokens, 1);
    if (!isValidName(name))
    {
      return false;
    }

    stints = List< Stint >();
    for (std::size_t pos = 2; pos < tokens.size(); pos += 2)
    {
      const std::string tyre_name = tokenAt(tokens, pos);
      if (!isValidName(tyre_name))
      {
        return false;
      }

      unsigned laps = 0;
      if (!parsePositiveUnsigned(tokenAt(tokens, pos + 1), laps))
      {
        return false;
      }

      stints.pushBack(Stint(tyre_name, laps));
    }

    return true;
  }

  bool isCreateStrategyStintsValid(const Session & session,
                                   const List< Stint > & stints)
  {
    return strategyValidationError(session, stints).empty();
  }

  void printStrategyValidLine(const std::string & name, Weather weather,
                              std::ostream & out)
  {
    out << "Strategy \"" << name << "\" is valid: laps OK";
    if (weather == Weather::Dry)
    {
      out << ", 2 slick compounds";
    }
    out << ", tyre limits OK\n";
  }

  bool validateStrategy(const Session & session, const std::string & name,
                        std::ostream & out)
  {
    if (!session.strategies().has(name))
    {
      return false;
    }

    const List< Stint > & stints = session.strategies().get(name);
    const std::string error = strategyValidationError(session, stints);
    if (!error.empty())
    {
      out << "Strategy \"" << name << "\" is invalid: " << error << '\n';
      return true;
    }

    printStrategyValidLine(name, session.weather(), out);
    return true;
  }

  void writeApproxDuration(double seconds, std::ostream & out)
  {
    const unsigned long long whole_secs =
      static_cast< unsigned long long >(seconds);
    const unsigned long long hours = whole_secs / 3600;
    const unsigned long long rem = whole_secs % 3600;
    const unsigned mins = static_cast< unsigned >(rem / 60);
    const unsigned secs = static_cast< unsigned >(rem % 60);

    out << "approx " << hours << 'h' << ' ' << mins << 'm' << ' ' << secs
        << 's';
  }

  void printSimulateTotalLine(const std::string & name, double seconds,
                              std::ostream & out)
  {
    out << std::fixed << std::setprecision(1);
    out << "Strategy \"" << name << "\" total time: " << seconds
        << " seconds (";
    writeApproxDuration(seconds, out);
    out << ")\n";
  }

  void printSimulateStintLines(const Session & session,
                               const List< Stint > & stints, std::ostream & out)
  {
    out << std::fixed << std::setprecision(1);

    unsigned stint_num = 1;
    unsigned race_lap = 1;
    bool first_stint = true;
    for (auto it = stints.begin(); it != stints.end(); ++it)
    {
      if (!first_stint)
      {
        const TyreSpec & tyre = session.tyres().get(it->tyre_name);
        out << "Pit stop: " << tyre.pit_time << " s\n";
      }
      first_stint = false;

      const unsigned start_lap = race_lap;
      const unsigned end_lap = race_lap + it->laps - 1;
      const double stint_time = stintTotalTime(session, *it);

      out << "Stint " << stint_num << ' ' << it->tyre_name << " (laps "
          << start_lap << '-' << end_lap << "): " << stint_time << " s\n";

      race_lap = end_lap + 1;
      ++stint_num;
    }
  }

  bool simulateStrategy(const Session & session, const std::string & name,
                        std::ostream & out)
  {
    if (!session.strategies().has(name))
    {
      return false;
    }

    const List< Stint > & stints = session.strategies().get(name);
    if (!strategyValidationError(session, stints).empty())
    {
      return false;
    }

    printSimulateStintLines(session, stints, out);
    const double total = strategyRaceTime(session, stints);
    printSimulateTotalLine(name, total, out);
    return true;
  }

  void printStrategyCreatedLine(const std::string & name,
                                const List< Stint > & stints, std::ostream & out)
  {
    out << "Strategy \"" << name << "\" created (";
    bool first = true;
    for (auto it = stints.begin(); it != stints.end(); ++it)
    {
      if (!first)
      {
        out << ", ";
      }
      first = false;
      out << it->laps << " laps on " << it->tyre_name;
    }
    out << ")\n";
  }

  void writeStintSequence(std::ostream & out, const List< Stint > & stints)
  {
    bool first = true;
    for (auto it = stints.begin(); it != stints.end(); ++it)
    {
      if (!first)
      {
        out << " -> ";
      }
      first = false;
      out << it->tyre_name << '(' << it->laps << ')';
    }
  }

  void printStrategies(const Session & session, std::ostream & out)
  {
    out << "Strategies:\n";
    for (auto it = session.strategyNames().begin();
         it != session.strategyNames().end(); ++it)
    {
      const List< Stint > & stints = session.strategies().get(*it);
      out << *it << ": ";
      writeStintSequence(out, stints);
      out << '\n';
    }
  }

  bool deleteStrategy(Session & session, const std::string & name,
                      std::ostream & out)
  {
    if (!session.strategies().has(name))
    {
      return false;
    }

    session.strategies().drop(name);
    session.removeStrategyName(name);
    session.markDirty();
    out << "Strategy \"" << name << "\" deleted\n";
    return true;
  }

}
