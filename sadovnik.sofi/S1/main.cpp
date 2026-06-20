#include "io.hpp"

#include <iostream>
#include <new>

int main()
{
  using namespace sadovnik;

  try
  {
    List< Seq > sequences = readData(std::cin);

    if (sequences.empty())
    {
      std::cout << "0\n";
      return 0;
    }

    printNames(std::cout, sequences);

    bool hasNumbers = false;
    for (LCIter< Seq > it = sequences.cbegin(); it != sequences.cend(); ++it)
    {
      if (!(*it).second.empty())
      {
        hasNumbers = true;
        break;
      }
    }

    if (!hasNumbers)
    {
      std::cout << "0\n";
      return 0;
    }

    printTransposed(std::cout, sequences);
    List< std::size_t > sums = calculateSums(sequences);
    printSums(std::cout, sums);
    return 0;
  }
  catch (const std::overflow_error & error)
  {
    std::cerr << error.what() << '\n';
    return 1;
  }
  catch (const std::bad_alloc & error)
  {
    std::cerr << "memory error: " << error.what() << '\n';
    return 1;
  }
  catch (...)
  {
    std::cerr << "error\n";
    return 1;
  }
}
