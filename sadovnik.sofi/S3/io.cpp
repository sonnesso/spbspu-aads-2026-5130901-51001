#include "io.hpp"

#include <istream>
#include <limits>
#include <stdexcept>
#include <string>

namespace
{

  bool isLetter(char ch)
  {
    return (ch >= 'A' && ch <= 'Z') || (ch >= 'a' && ch <= 'z');
  }

  bool isDigit(char ch)
  {
    return ch >= '0' && ch <= '9';
  }

  bool isSpace(char ch)
  {
    return ch == ' ' || ch == '\t' || ch == '\r';
  }

  bool isValidName(const std::string & value)
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

  sadovnik::List< std::string > splitTokens(const std::string & line)
  {
    sadovnik::List< std::string > tokens;
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

  bool parseUnsigned(const std::string & value, unsigned long long & result)
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

  std::string tokenAt(const sadovnik::List< std::string > & tokens, std::size_t index)
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

    throw std::logic_error("invalid graph file");
  }

  bool readNonEmptyLine(std::istream & in, std::string & line)
  {
    while (true)
    {
      std::getline(in, line);
      if (line.empty() && in.eof())
      {
        return false;
      }
      if (in.fail() && line.empty())
      {
        return false;
      }
      if (!splitTokens(line).empty())
      {
        return true;
      }
      if (in.eof())
      {
        return false;
      }
    }
  }

  std::size_t parseCount(const std::string & token)
  {
    unsigned long long value = 0;
    if (!parseUnsigned(token, value) ||
        value > std::numeric_limits< std::size_t >::max())
    {
      throw std::logic_error("invalid graph file");
    }

    return static_cast< std::size_t >(value);
  }

}

namespace sadovnik
{

  namespace detail
  {
    const std::size_t INIT_GRAPH_SLOTS = 16;
  }

  GraphTab readGraphs(std::istream & in)
  {
    GraphTab graphs(detail::INIT_GRAPH_SLOTS);
    std::string line;

    while (readNonEmptyLine(in, line))
    {
      const List< std::string > header = splitTokens(line);
      if (header.size() != 2)
      {
        throw std::logic_error("invalid graph file");
      }

      const std::string name = tokenAt(header, 0);
      if (!isValidName(name) || graphs.has(name))
      {
        throw std::logic_error("invalid graph file");
      }

      const std::size_t edgeCnt = parseCount(tokenAt(header, 1));
      Graph graph;

      for (std::size_t i = 0; i < edgeCnt; ++i)
      {
        if (!readNonEmptyLine(in, line))
        {
          throw std::logic_error("invalid graph file");
        }

        const List< std::string > edge = splitTokens(line);
        if (edge.size() != 3)
        {
          throw std::logic_error("invalid graph file");
        }

        const std::string from = tokenAt(edge, 0);
        const std::string to = tokenAt(edge, 1);
        unsigned long long weight = 0;

        if (!isValidName(from) || !isValidName(to) ||
            !parseUnsigned(tokenAt(edge, 2), weight))
        {
          throw std::logic_error("invalid graph file");
        }

        graph.bind(from, to, weight);
      }

      if (graphs.size() == graphs.capacity())
      {
        graphs.rehash(graphs.slots() * 2);
      }

      graphs.add(name, graph);
    }

    return graphs;
  }

}
