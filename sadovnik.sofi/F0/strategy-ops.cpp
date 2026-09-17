#include "strategy-ops.hpp"

#include "tyre-math.hpp"

#include "queue.hpp"
#include <bs-tree.hpp>
#include <string-utils.hpp>

#include <functional>
#include <iomanip>
#include <ostream>
#include <string>
#include <utility>

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

  bool usesWetWeatherTyre(const sadovnik::Session & session,
                          const List< Stint > & stints)
  {
    for (auto it = stints.begin(); it != stints.end(); ++it)
    {
      if (!session.tyres().has(it->tyre_name))
      {
        continue;
      }

      const TyreKind kind = session.tyres().get(it->tyre_name).kind;
      if (kind == TyreKind::Inter || kind == TyreKind::Wet)
      {
        return true;
      }
    }

    return false;
  }

  bool requiresTwoSlickCompounds(const sadovnik::Session & session,
                                 const List< Stint > & stints)
  {
    if (session.weather() != sadovnik::Weather::Dry)
    {
      return false;
    }

    if (usesWetWeatherTyre(session, stints))
    {
      return false;
    }

    return true;
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

    if (requiresTwoSlickCompounds(session, stints) &&
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

  bool fillStrategyFromTokens(const List< std::string > & tokens,
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

  bool parseCreateStrategyTokens(const List< std::string > & tokens,
                                 std::string & name, List< Stint > & stints)
  {
    return fillStrategyFromTokens(tokens, name, stints);
  }

  bool parseStrategyLineTokens(const List< std::string > & tokens,
                               std::string & name, List< Stint > & stints)
  {
    if (tokens.empty() || tokenAt(tokens, 0) != "strategy")
    {
      return false;
    }

    return fillStrategyFromTokens(tokens, name, stints);
  }

  bool isCreateStrategyStintsValid(const Session & session,
                                   const List< Stint > & stints)
  {
    return strategyValidationError(session, stints).empty();
  }

  void printStrategyValidLine(const std::string & name, bool two_slick_rule,
                              std::ostream & out)
  {
    out << "Strategy \"" << name << "\" is valid: laps OK";
    if (two_slick_rule)
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

    printStrategyValidLine(name, requiresTwoSlickCompounds(session, stints), out);
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
    using RankKey = std::pair< double, std::string >;
    sadovnik::BSTree< RankKey, CompareEntry, std::less< RankKey > > ranked;
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
      ranked.push(RankKey(entry.time, entry.name), entry);
    }

    for (auto it = ranked.begin(); it != ranked.end(); ++it)
    {
      writeCompareStrategyLine(it->second.name, it->second.time, *it->second.stints,
                               out);
    }

    auto best_it = ranked.begin();
    const CompareEntry & best = best_it->second;
    double delta_s = 0.0;
    auto runner_it = best_it;
    ++runner_it;
    if (runner_it != ranked.end())
    {
      delta_s = runner_it->second.time - best.time;
    }

    writeCompareBestLine(best.name, delta_s, out);

    if (names.size() == 2)
    {
      auto first = entries.begin();
      auto second = entries.begin();
      ++second;
      tryWriteUndercutWindow(session, *first->stints, *second->stints, out);
    }

    return true;
  }

  bool isOneTyrePitOptionValid(const sadovnik::Session & session,
                               const std::string & tyre_name, unsigned pit_lap,
                               unsigned total_laps)
  {
    if (!session.track().is_set || !session.tyres().has(tyre_name))
    {
      return false;
    }

    if (pit_lap == 0 || pit_lap >= total_laps)
    {
      return false;
    }

    const TyreSpec & spec = session.tyres().get(tyre_name);
    const unsigned second_laps = total_laps - pit_lap;
    if (pit_lap > spec.max_laps || second_laps > spec.max_laps)
    {
      return false;
    }

    return true;
  }

  bool optimalPitWindow(const Session & session, const std::string & tyre_name,
                        unsigned total_laps, std::ostream & out)
  {
    if (total_laps < 2 || !session.track().is_set ||
        !session.tyres().has(tyre_name))
    {
      return false;
    }

    Queue< unsigned > candidates;
    for (unsigned pit_lap = 1; pit_lap < total_laps; ++pit_lap)
    {
      if (isOneTyrePitOptionValid(session, tyre_name, pit_lap, total_laps))
      {
        candidates.push(pit_lap);
      }
    }

    if (candidates.empty())
    {
      return false;
    }

    unsigned best_lap = 0;
    double best_time = 0.0;
    bool any = false;
    while (!candidates.empty())
    {
      unsigned pit_lap = 0;
      candidates.drop(pit_lap);

      List< Stint > stints;
      stints.pushBack(Stint(tyre_name, pit_lap));
      stints.pushBack(Stint(tyre_name, total_laps - pit_lap));
      const double time = strategyRaceTime(session, stints);
      if (!any || time < best_time ||
          (time == best_time && pit_lap < best_lap))
      {
        best_lap = pit_lap;
        best_time = time;
        any = true;
      }
    }

    out << std::fixed << std::setprecision(1);
    out << "Optimal pit window: lap " << best_lap << " (total time "
        << best_time << "s, best of " << (total_laps - 1) << " options)\n";
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

  bool clampStintLaps(unsigned & first, unsigned second_max, unsigned total)
  {
    if (total < 2)
    {
      return false;
    }
    if (first >= total)
    {
      first = total - 1;
    }
    if (first == 0)
    {
      first = 1;
    }
    const unsigned second = total - first;
    if (second == 0 || second > second_max || first > second_max)
    {
      return false;
    }
    return true;
  }

  bool buildTwoStintPlan(const Session & session, const std::string & tyre1,
                         const std::string & tyre2, unsigned first_laps,
                         List< Stint > & stints)
  {
    if (!session.tyres().has(tyre1) || !session.tyres().has(tyre2))
    {
      return false;
    }

    const unsigned total = session.track().laps;
    const unsigned max1 = session.tyres().get(tyre1).max_laps;
    const unsigned max2 = session.tyres().get(tyre2).max_laps;
    unsigned first = first_laps;
    if (first > max1)
    {
      first = max1;
    }
    if (!clampStintLaps(first, max2, total))
    {
      return false;
    }
    if (total - first > max2)
    {
      return false;
    }

    stints = List< Stint >();
    stints.pushBack(Stint(tyre1, first));
    stints.pushBack(Stint(tyre2, total - first));
    return isCreateStrategyStintsValid(session, stints);
  }

  bool buildThreeStintPlan(const Session & session, const std::string & tyre1,
                           const std::string & tyre2, const std::string & tyre3,
                           unsigned first_laps, unsigned second_laps,
                           List< Stint > & stints)
  {
    if (!session.tyres().has(tyre1) || !session.tyres().has(tyre2) ||
        !session.tyres().has(tyre3))
    {
      return false;
    }

    const unsigned total = session.track().laps;
    if (first_laps + second_laps >= total)
    {
      return false;
    }

    const unsigned third = total - first_laps - second_laps;
    const TyreSpec & s1 = session.tyres().get(tyre1);
    const TyreSpec & s2 = session.tyres().get(tyre2);
    const TyreSpec & s3 = session.tyres().get(tyre3);
    if (first_laps == 0 || second_laps == 0 || third == 0)
    {
      return false;
    }
    if (first_laps > s1.max_laps || second_laps > s2.max_laps ||
        third > s3.max_laps)
    {
      return false;
    }

    stints = List< Stint >();
    stints.pushBack(Stint(tyre1, first_laps));
    stints.pushBack(Stint(tyre2, second_laps));
    stints.pushBack(Stint(tyre3, third));
    return isCreateStrategyStintsValid(session, stints);
  }

  std::string uniqueStrategyName(Session & session, const std::string & base,
                                 std::ostream & out)
  {
    if (!session.strategies().has(base))
    {
      return base;
    }

    const std::string renamed = base + "_2";
    out << "Warning: strategy \"" << base
        << "\" already exists; saved as \"" << renamed << "\".\n";
    if (!session.strategies().has(renamed))
    {
      return renamed;
    }

    const std::string alt = base + "_3";
    out << "Warning: strategy \"" << renamed
        << "\" already exists; saved as \"" << alt << "\".\n";
    return alt;
  }

  void writeSuggestStintBrief(const List< Stint > & stints, std::ostream & out)
  {
    bool first = true;
    for (auto it = stints.begin(); it != stints.end(); ++it)
    {
      if (!first)
      {
        out << ", ";
      }
      first = false;
      out << it->tyre_name << ' ' << it->laps;
    }
  }

  struct SuggestCandidate
  {
    std::string base_name;
    List< Stint > stints;
    double time;
  };

  using SuggestRankKey = std::pair< double, std::string >;
  using SuggestRankTree =
    sadovnik::BSTree< SuggestRankKey, SuggestCandidate, std::less< SuggestRankKey > >;

  void pushCandidate(const Session & session, const std::string & base_name,
                     const List< Stint > & stints, SuggestRankTree & ranked)
  {
    if (!isCreateStrategyStintsValid(session, stints))
    {
      return;
    }

    SuggestCandidate candidate;
    candidate.base_name = base_name;
    candidate.stints = stints;
    candidate.time = strategyRaceTime(session, stints);
    ranked.push(SuggestRankKey(candidate.time, base_name), candidate);
  }

  bool suggestStrategies(Session & session, std::ostream & out)
  {
    if (!session.track().is_set)
    {
      return false;
    }

    if (!session.tyres().has("Soft") || !session.tyres().has("Medium") ||
        !session.tyres().has("Hard"))
    {
      return false;
    }

    const unsigned laps = session.track().laps;
    const std::string prefix =
      session.circuitName().empty() ? std::string("Track") : session.circuitName();

    SuggestRankTree ranked;
    List< Stint > plan;

    if (buildTwoStintPlan(session, "Soft", "Hard", laps * 34 / 100, plan))
    {
      pushCandidate(session, prefix + "_1Stop_SH", plan, ranked);
    }
    if (buildTwoStintPlan(session, "Medium", "Hard", laps * 34 / 100, plan))
    {
      pushCandidate(session, prefix + "_1Stop_MH", plan, ranked);
    }
    if (buildTwoStintPlan(session, "Soft", "Medium", laps * 40 / 100, plan))
    {
      pushCandidate(session, prefix + "_1Stop_SM", plan, ranked);
    }
    if (buildThreeStintPlan(session, "Soft", "Medium", "Hard", laps * 22 / 100,
                            laps * 32 / 100, plan))
    {
      pushCandidate(session, prefix + "_2Stop_SMH", plan, ranked);
    }
    if (buildThreeStintPlan(session, "Soft", "Medium", "Hard", laps * 18 / 100,
                            laps * 28 / 100, plan))
    {
      pushCandidate(session, prefix + "_2Stop_SMH_B", plan, ranked);
    }

    if (ranked.empty())
    {
      return false;
    }

    const std::string label =
      session.circuitName().empty() ? std::string("Track") : session.circuitName();
    out << "Suggested strategies (" << label << ", " << laps << " laps):\n";

    std::string best_name;
    std::size_t rank = 1;
    for (auto it = ranked.begin(); it != ranked.end() && rank <= 5; ++it)
    {
      const SuggestCandidate & candidate = it->second;
      const std::string name =
        uniqueStrategyName(session, candidate.base_name, out);
      try
      {
        session.strategies().add(name, candidate.stints);
      }
      catch (const std::exception &)
      {
        return false;
      }
      session.addStrategyName(name);

      out << std::fixed << std::setprecision(1);
      out << "  " << rank << ". " << name << " — " << candidate.time << " s (";
      writeSuggestStintBrief(candidate.stints, out);
      out << ")\n";

      if (rank == 1)
      {
        best_name = name;
      }
      ++rank;
    }

    out << "Best suggested: " << best_name << '\n';
    session.markDirty();
    return true;
  }

  double humidityCrossoverBias(TyreKind kind, unsigned humidity)
  {
    const double ref = 40.0;
    const double scale = 0.15;
    const double hum = static_cast< double >(humidity);

    if (kind == TyreKind::Wet)
    {
      return (ref - hum) * scale;
    }
    if (kind == TyreKind::Inter)
    {
      return (hum - ref) * scale;
    }
    if (kind == TyreKind::Slick)
    {
      return 10.0 + hum * 0.05;
    }

    return 0.0;
  }

  double crossoverLapTime(const Session & session, const std::string & tyre_name)
  {
    const TyreSpec & spec = session.tyres().get(tyre_name);
    return session.track().base_lap_s + spec.base_offset +
           humidityCrossoverBias(spec.kind, session.humidity());
  }

  bool crossoverCheck(const Session & session, const std::string & from_tyre,
                      const std::string & to_tyre, std::ostream & out)
  {
    if (session.weather() == Weather::Dry)
    {
      return false;
    }

    if (!session.track().is_set)
    {
      return false;
    }

    if (!session.tyres().has(from_tyre) || !session.tyres().has(to_tyre))
    {
      return false;
    }

    const double from_time = crossoverLapTime(session, from_tyre);
    const double to_time = crossoverLapTime(session, to_tyre);

    out << std::fixed << std::setprecision(1);
    if (to_time < from_time)
    {
      out << "Crossover: switch to " << to_tyre << " recommended (";
    }
    else
    {
      out << "Crossover: stay on " << from_tyre << " recommended (";
    }

    out << from_tyre << ' ' << from_time << "s vs " << to_tyre << ' ' << to_time
        << "s at humidity " << session.humidity() << "%)\n";
    return true;
  }

}
