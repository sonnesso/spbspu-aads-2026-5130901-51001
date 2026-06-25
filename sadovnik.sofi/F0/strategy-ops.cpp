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

  bool hasStrategyName(const List< std::string > & names,
                       const std::string & name)
  {
    for (auto it = names.begin(); it != names.end(); ++it)
    {
      if (*it == name)
      {
        return true;
      }
    }

    return false;
  }

  struct CompareEntry
  {
    std::string name;
    double time;
    const List< Stint > * stints;

    CompareEntry()
      : name(),
        time(0.0),
        stints(nullptr)
    {
    }
  };

  bool stintsHaveSameTyrePlan(const List< Stint > & left,
                              const List< Stint > & right)
  {
    if (left.size() != right.size())
    {
      return false;
    }

    auto left_it = left.begin();
    auto right_it = right.begin();
    for (; left_it != left.end(); ++left_it, ++right_it)
    {
      if (left_it->tyre_name != right_it->tyre_name)
      {
        return false;
      }
    }

    return true;
  }

  bool differOnlyByFirstPitLap(const List< Stint > & left,
                               const List< Stint > & right)
  {
    if (left.size() != 2 || right.size() != 2)
    {
      return false;
    }

    if (!stintsHaveSameTyrePlan(left, right))
    {
      return false;
    }

    return left.begin()->laps != right.begin()->laps;
  }

  List< Stint > buildTwoStintStrategy(const std::string & tyre1,
                                      const std::string & tyre2,
                                      unsigned first_laps, unsigned total_laps)
  {
    List< Stint > stints;
    stints.pushBack(Stint(tyre1, first_laps));
    stints.pushBack(Stint(tyre2, total_laps - first_laps));
    return stints;
  }

  bool isPitLapFaster(const sadovnik::Session & session,
                      const std::string & tyre1, const std::string & tyre2,
                      unsigned pit_lap, unsigned total_laps, double ref_time)
  {
    if (pit_lap == 0 || pit_lap >= total_laps)
    {
      return false;
    }

    const List< Stint > stints =
      buildTwoStintStrategy(tyre1, tyre2, pit_lap, total_laps);
    if (!strategyValidationError(session, stints).empty())
    {
      return false;
    }

    return strategyRaceTime(session, stints) < ref_time;
  }

  bool tryWriteUndercutWindow(const sadovnik::Session & session,
                              const List< Stint > & left,
                              const List< Stint > & right, std::ostream & out)
  {
    if (!differOnlyByFirstPitLap(left, right))
    {
      return false;
    }

    const unsigned pit_left = left.begin()->laps;
    const unsigned pit_right = right.begin()->laps;
    const unsigned ref_lap =
      (pit_left > pit_right ? pit_left : pit_right) + 1;
    const unsigned total_laps = session.track().laps;
    if (ref_lap >= total_laps)
    {
      return false;
    }

    const std::string tyre1 = left.begin()->tyre_name;
    auto second_it = left.begin();
    ++second_it;
    const std::string tyre2 = second_it->tyre_name;

    const List< Stint > ref_stints =
      buildTwoStintStrategy(tyre1, tyre2, ref_lap, total_laps);
    if (!strategyValidationError(session, ref_stints).empty())
    {
      return false;
    }

    const double ref_time = strategyRaceTime(session, ref_stints);

    unsigned win_lo = 0;
    unsigned win_hi = 0;
    bool any = false;
    for (unsigned pit_lap = 1; pit_lap < total_laps; ++pit_lap)
    {
      if (!isPitLapFaster(session, tyre1, tyre2, pit_lap, total_laps,
                          ref_time))
      {
        continue;
      }

      if (!any)
      {
        win_lo = pit_lap;
        win_hi = pit_lap;
        any = true;
        continue;
      }

      if (pit_lap < win_lo)
      {
        win_lo = pit_lap;
      }
      if (pit_lap > win_hi)
      {
        win_hi = pit_lap;
      }
    }

    if (!any)
    {
      return false;
    }

    for (unsigned pit_lap = win_lo; pit_lap <= win_hi; ++pit_lap)
    {
      if (!isPitLapFaster(session, tyre1, tyre2, pit_lap, total_laps,
                          ref_time))
      {
        return false;
      }
    }

    out << "Undercut window: pit on laps " << win_lo << '-' << win_hi
        << " faster than lap " << ref_lap << '\n';
    return true;
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

  bool isStrategyComparable(const Session & session, const std::string & name,
                            const List< Stint > * & stints)
  {
    if (!session.strategies().has(name))
    {
      return false;
    }

    stints = &session.strategies().get(name);
    return strategyValidationError(session, *stints).empty();
  }

  void writeCompareStrategyLine(const std::string & name, double seconds,
                                const List< Stint > & stints, std::ostream & out)
  {
    out << std::fixed << std::setprecision(1);
    out << name << ": " << seconds << " s";
    if (stints.size() >= 2)
    {
      const unsigned pit_lap = stints.begin()->laps;
      out << " (pit after lap " << pit_lap << ')';
    }
    out << '\n';
  }

  void writeCompareBestLine(const std::string & best_name, double delta_s,
                            std::ostream & out)
  {
    out << std::fixed << std::setprecision(1);
    out << "Best: " << best_name << " (faster by " << delta_s << " s)\n";
  }

  bool compareStrategies(const Session & session,
                         const List< std::string > & names, std::ostream & out)
  {
    if (names.size() < 2)
    {
      return false;
    }

    List< std::string > seen;
    for (auto it = names.begin(); it != names.end(); ++it)
    {
      if (hasStrategyName(seen, *it))
      {
        return false;
      }
      seen.pushBack(*it);
    }

    List< CompareEntry > entries;
    for (auto it = names.begin(); it != names.end(); ++it)
    {
      const List< Stint > * stints = nullptr;
      if (!isStrategyComparable(session, *it, stints))
      {
        return false;
      }

      CompareEntry entry;
      entry.name = *it;
      entry.stints = stints;
      entry.time = strategyRaceTime(session, *stints);
      entries.pushBack(entry);
    }

    for (auto it = entries.begin(); it != entries.end(); ++it)
    {
      writeCompareStrategyLine(it->name, it->time, *it->stints, out);
    }

    const CompareEntry * best = nullptr;
    const CompareEntry * runner_up = nullptr;
    for (auto it = entries.begin(); it != entries.end(); ++it)
    {
      if (best == nullptr || it->time < best->time)
      {
        runner_up = best;
        best = &(*it);
        continue;
      }

      if (runner_up == nullptr || it->time < runner_up->time)
      {
        runner_up = &(*it);
      }
    }

    double delta_s = 0.0;
    if (runner_up != nullptr)
    {
      delta_s = runner_up->time - best->time;
    }

    writeCompareBestLine(best->name, delta_s, out);

    if (names.size() == 2)
    {
      auto first = entries.begin();
      auto second = entries.begin();
      ++second;
      tryWriteUndercutWindow(session, *first->stints, *second->stints, out);
    }

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
