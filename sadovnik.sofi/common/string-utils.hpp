#ifndef STRING_UTILS_HPP
#define STRING_UTILS_HPP

#include <list.hpp>

#include <limits>
#include <string>

namespace sadovnik
{

  inline bool isLetter(char ch)
  {
    return (ch >= 'A' && ch <= 'Z') || (ch >= 'a' && ch <= 'z');
  }

  inline bool isDigit(char ch)
  {
    return ch >= '0' && ch <= '9';
  }

  inline bool isSpace(char ch)
  {
    return ch == ' ' || ch == '\t' || ch == '\r';
  }

  inline bool isValidName(const std::string & value)
  {
    if (value.empty() || !isLetter(value[0]))
    {
      return false;
    }

    for (std::size_t i = 1; i < value.size(); ++i)
    {
      if (!isLetter(value[i]) && !isDigit(value[i]))
      {
        return false;
      }
    }

    return true;
  }

  inline List< std::string > splitTokens(const std::string & line)
  {
    List< std::string > tokens;
    std::size_t pos = 0;

    while (pos < line.size())
    {
      while (pos < line.size() && isSpace(line[pos]))
      {
        ++pos;
      }

      const std::size_t start = pos;
      while (pos < line.size() && !isSpace(line[pos]))
      {
        ++pos;
      }

      if (start != pos)
      {
        tokens.pushBack(line.substr(start, pos - start));
      }
    }

    return tokens;
  }

  inline bool parseUnsigned(const std::string & value, unsigned long long & result)
  {
    if (value.empty())
    {
      return false;
    }

    unsigned long long parsed = 0;
    const unsigned long long limit =
      std::numeric_limits< unsigned long long >::max();

    for (std::size_t i = 0; i < value.size(); ++i)
    {
      if (!isDigit(value[i]))
      {
        return false;
      }

      const unsigned long long digit =
        static_cast< unsigned long long >(value[i] - '0');
      if (parsed > (limit - digit) / 10)
      {
        return false;
      }

      parsed = parsed * 10 + digit;
    }

    result = parsed;
    return true;
  }

}

#endif
