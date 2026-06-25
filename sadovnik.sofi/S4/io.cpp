#include "io.hpp"

#include <string-utils.hpp>

#include <istream>
#include <limits>
#include <stdexcept>
#include <string>

namespace
{

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

    throw std::logic_error("invalid dataset file");
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
      if (!sadovnik::splitTokens(line).empty())
      {
        return true;
      }
      if (in.eof())
      {
        return false;
      }
    }
  }

  bool parseKey(const std::string & token, int & result)
  {
    unsigned long long parsed = 0;
    if (!sadovnik::parseUnsigned(token, parsed) ||
        parsed > static_cast< unsigned long long >(
          std::numeric_limits< int >::max()))
    {
      return false;
    }

    result = static_cast< int >(parsed);
    return true;
  }

}

namespace sadovnik
{

  namespace detail
  {
    const std::size_t INIT_DATASET_SLOTS = 16;
  }

  DatasetTab readDatasets(std::istream & in)
  {
    DatasetTab datasets(detail::INIT_DATASET_SLOTS);
    std::string line;

    while (readNonEmptyLine(in, line))
    {
      const List< std::string > tokens = splitTokens(line);
      if (tokens.empty())
      {
        throw std::logic_error("invalid dataset file");
      }

      const std::string name = tokenAt(tokens, 0);
      if (!isValidName(name) || datasets.has(name))
      {
        throw std::logic_error("invalid dataset file");
      }

      if ((tokens.size() - 1) % 2 != 0)
      {
        throw std::logic_error("invalid dataset file");
      }

      IntStrTree tree;
      for (std::size_t i = 1; i < tokens.size(); i += 2)
      {
        int key = 0;
        if (!parseKey(tokenAt(tokens, i), key))
        {
          throw std::logic_error("invalid dataset file");
        }

        const std::string & val = tokenAt(tokens, i + 1);
        tree.push(key, val);
      }

      if (datasets.size() == datasets.capacity())
      {
        datasets.rehash(datasets.slots() * 2);
      }

      datasets.add(name, tree);
    }

    return datasets;
  }

}
