#include "tyre-ops.hpp"

#include <string-utils.hpp>

#include <iomanip>
#include <ostream>
#include <string>

namespace
{

  using sadovnik::List;
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

}

namespace sadovnik
{

  bool tyreNameMatchesKind(const std::string & name, TyreKind kind)
  {
    if (name == "Soft" || name == "Medium" || name == "Hard")
    {
      return kind == TyreKind::Slick;
    }
    if (name == "Intermediate")
    {
      return kind == TyreKind::Inter;
    }
    if (name == "Wet")
    {
      return kind == TyreKind::Wet;
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

    name = tokenAt(tokens, 1);
    if (!isValidName(name))
    {
      return false;
    }

    TyreKind kind = TyreKind::Slick;
    if (!parseTyreKind(tokenAt(tokens, 2), kind))
    {
      return false;
    }

    if (!tyreNameMatchesKind(name, kind))
    {
      return false;
    }

    double degr = 0.0;
    if (!parseDouble(tokenAt(tokens, 3), degr) || degr <= 0.0)
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
      if (!parseCompound(tokenAt(tokens, 6), compound))
      {
        return false;
      }
    }

    spec = TyreSpec();
    spec.kind = kind;
    spec.compound = compound;
    spec.degr = degr;
    spec.max_laps = max_laps;
    spec.pit_time = pit_time;
    spec.base_offset = baseOffsetForTyre(kind, compound);
    return true;
  }

  void sortTyreNames(List< std::string > & names)
  {
    if (names.empty())
    {
      return;
    }

    List< std::string > sorted;
    for (auto it = names.begin(); it != names.end(); ++it)
    {
      std::string cur = *it;
      List< std::string > next;
      bool placed = false;

      for (auto sit = sorted.begin(); sit != sorted.end(); ++sit)
      {
        if (!placed && cur < *sit)
        {
          next.pushBack(cur);
          placed = true;
        }
        next.pushBack(*sit);
      }

      if (!placed)
      {
        next.pushBack(cur);
      }

      sorted = next;
    }

    names = sorted;
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
    out << name << ": type=" << tyreKindToString(spec.kind);
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

    List< std::string > names;
    for (auto it = session.tyreNames().begin(); it != session.tyreNames().end();
         ++it)
    {
      names.pushBack(*it);
    }

    sortTyreNames(names);

    for (auto it = names.begin(); it != names.end(); ++it)
    {
      const TyreSpec & spec = session.tyres().get(*it);
      writeTyreLine(out, *it, spec);
    }
  }

}
