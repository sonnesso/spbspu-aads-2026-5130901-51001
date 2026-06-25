#include "strategy-ops.hpp"

#include <string-utils.hpp>

#include <ostream>
#include <string>

namespace
{

  using sadovnik::List;
  using sadovnik::Stint;

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
