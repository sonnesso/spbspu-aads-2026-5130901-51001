#include "calculator.hpp"

#include <fstream>
#include <iostream>
#include <string>

#include <list.hpp>

namespace
{

  bool isBlank(const std::string & line)
  {
    for (std::size_t i = 0; i < line.size(); ++i)
    {
      if (line[i] != ' ')
      {
        return false;
      }
    }
    return true;
  }

}

int main(int argc, char * argv[])
{
  using namespace sadovnik;

  try
  {
    if (argc > 2)
    {
      std::cerr << "invalid arguments\n";
      return 1;
    }

    std::istream * in = &std::cin;
    std::ifstream file;

    if (argc == 2)
    {
      file.open(argv[1]);
      if (!file)
      {
        std::cerr << "cannot open file\n";
        return 1;
      }
      in = &file;
    }

    List< Value > results;
    Calculator calc;
    std::string line;

    while (std::getline(*in, line))
    {
      if (line.empty() || isBlank(line))
      {
        continue;
      }
      results.pushBack(calc.evaluate(line));
    }

    if (results.empty())
    {
      std::cout << '\n';
      return 0;
    }

    bool first = true;
    for (LIter< Value > it = results.last();; --it)
    {
      if (!first)
      {
        std::cout << ' ';
      }
      std::cout << *it;
      first = false;

      if (it == results.begin())
      {
        break;
      }
    }
    std::cout << '\n';
    return 0;
  }
  catch (const std::overflow_error & err)
  {
    std::cerr << err.what() << '\n';
    return 1;
  }
  catch (const std::logic_error & err)
  {
    std::cerr << err.what() << '\n';
    return 1;
  }
  catch (const std::bad_alloc & err)
  {
    std::cerr << "memory error: " << err.what() << '\n';
    return 1;
  }
  catch (...)
  {
    std::cerr << "error\n";
    return 1;
  }
}
