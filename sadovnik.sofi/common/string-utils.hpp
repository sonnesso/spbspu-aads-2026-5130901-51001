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

  inline bool parseDouble(const std::string & value, double & result)
  {
    if (value.empty())
    {
      return false;
    }

    std::size_t pos = 0;
    int sign = 1;
    if (value[pos] == '-')
    {
      sign = -1;
      ++pos;
    }
    else if (value[pos] == '+')
    {
      ++pos;
    }

    if (pos >= value.size())
    {
      return false;
    }

    double intPart = 0.0;
    bool hasDigits = false;
    while (pos < value.size() && isDigit(value[pos]))
    {
      hasDigits = true;
      intPart = intPart * 10.0 + static_cast< double >(value[pos] - '0');
      ++pos;
    }

    double fracPart = 0.0;
    double fracDiv = 1.0;
    bool sawDot = false;
    bool fracDigits = false;
    if (pos < value.size() && value[pos] == '.')
    {
      sawDot = true;
      ++pos;
      while (pos < value.size() && isDigit(value[pos]))
      {
        hasDigits = true;
        fracDigits = true;
        fracDiv = fracDiv * 10.0;
        fracPart = fracPart * 10.0 + static_cast< double >(value[pos] - '0');
        ++pos;
      }
      if (fracDigits)
      {
        fracPart = fracPart / fracDiv;
      }
    }

    if (!hasDigits || (sawDot && !fracDigits))
    {
      return false;
    }

    if (pos != value.size())
    {
      return false;
    }

    result = static_cast< double >(sign) * (intPart + fracPart);
    return true;
  }

}

#endif
