#include "tyre-ops.hpp"

#include <bs-tree.hpp>
#include <string-utils.hpp>

#include <functional>
#include <iomanip>
#include <ostream>
#include <string>

namespace
{

  using sadovnik::List;
  using sadovnik::TyreType;
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

  bool fillTyreSpecFromTokens(const List< std::string > & tokens,
                              std::string & name, TyreSpec & spec)
  {
    name = tokenAt(tokens, 1);
    if (!sadovnik::isValidName(name))
    {
      return false;
    }

    TyreType type = TyreType::Slick;
    if (!sadovnik::parseTyreType(tokenAt(tokens, 2), type))
    {
      return false;
    }

    if (!sadovnik::tyreNameMatchesType(name, type))
    {
      return false;
    }

    double degr = 0.0;
    if (!sadovnik::parseDouble(tokenAt(tokens, 3), degr) || degr <= 0.0)
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
      if (type != TyreType::Slick)
      {
        return false;
      }
      if (!sadovnik::parseCompound(tokenAt(tokens, 6), compound))
      {
        return false;
      }
    }

    spec = TyreSpec();
    spec.type = type;
    spec.compound = compound;
    spec.degr = degr;
    spec.max_laps = max_laps;
    spec.pit_time = pit_time;
    spec.base_offset = sadovnik::baseOffsetForTyre(type, compound);
    return true;
  }

}

namespace sadovnik
{

  bool tyreNameMatchesType(const std::string & name, TyreType type)
  {
    if (name == "Soft" || name == "Medium" || name == "Hard")
    {
      return type == TyreType::Slick;
    }
    if (name == "Intermediate")
    {
      return type == TyreType::Inter;
    }
    if (name == "Wet")
    {
      return type == TyreType::Wet;
    }

    return true;
  }

  bool parseAddTyreTokens(const List< std::string > & tokens, std::string & name,
                          TyreSpec & spec)
  {
    if (tokens.size() != 6 && tokens.size() != 7)
    {
      return false;
    }

    return fillTyreSpecFromTokens(tokens, name, spec);
  }

  bool parseTyreLineTokens(const List< std::string > & tokens, std::string & name,
                           TyreSpec & spec)
  {
    if (tokens.size() != 6 && tokens.size() != 7)
    {
      return false;
    }

    if (tokenAt(tokens, 0) != "tyre")
    {
      return false;
    }

    return fillTyreSpecFromTokens(tokens, name, spec);
  }

  void writeOffset(std::ostream & out, double offset)
  {
    out << "offset=";
    if (offset > 0.0)
    {
      out << '+';
    }
    out << std::fixed << std::setprecision(1) << offset;
  }

  void writeTyreLine(std::ostream & out, const std::string & name,
                     const TyreSpec & spec)
  {
    out << name << ": type=" << tyreTypeToString(spec.type);
    if (!spec.compound.empty())
    {
      out << ", compound=" << spec.compound;
    }
    out << std::fixed << std::setprecision(2);
    out << ", degr=" << spec.degr;
    out << ", max_laps=" << spec.max_laps;
    out << ", pit=" << spec.pit_time << 's';
    out << ", ";
    writeOffset(out, spec.base_offset);
    out << '\n';
  }

  void printTyres(const Session & session, std::ostream & out)
  {
    out << "Tyres:\n";

    sadovnik::BSTree< std::string, char, std::less< std::string > > ordered;
    for (auto it = session.tyreNames().begin(); it != session.tyreNames().end();
         ++it)
    {
      if (!ordered.has(*it))
      {
        ordered.push(*it, 0);
      }
    }

    for (auto it = ordered.begin(); it != ordered.end(); ++it)
    {
      const TyreSpec & spec = session.tyres().get(it->first);
      writeTyreLine(out, it->first, spec);
    }
  }

}
