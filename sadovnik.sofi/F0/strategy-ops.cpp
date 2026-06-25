#include "strategy-ops.hpp"

#include <string-utils.hpp>

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
    if (!session.track().is_set)
    {
      return false;
    }

    unsigned total_laps = 0;
    for (auto it = stints.begin(); it != stints.end(); ++it)
    {
      if (!session.tyres().has(it->tyre_name))
      {
        return false;
      }

      total_laps += it->laps;
    }

    if (total_laps != session.track().laps)
    {
      return false;
    }

    if (session.weather() == Weather::Dry &&
        !hasEnoughSlickCompoundsForDry(session, stints))
    {
      return false;
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

}
