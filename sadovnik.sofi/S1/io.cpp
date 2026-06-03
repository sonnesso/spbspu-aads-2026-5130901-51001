#include "io.hpp"

#include <ios>
#include <iostream>
#include <limits>
#include <stdexcept>

namespace
{

  void consumeLineTerminator(std::istream & in)
  {
    if (in.peek() == '\r')
    {
      static_cast<void>(in.get());
    }
    if (in.peek() == '\n')
    {
      static_cast<void>(in.get());
    }
  }

  bool atEndOfLine(std::istream & in)
  {
    const int ch = in.peek();
    return ch == '\n' || ch == '\r';
  }

  std::size_t getElementAt(const sadovnik::List< std::size_t > & list,
    std::size_t index,
    bool & exists)
  {
    sadovnik::LCIter< std::size_t > it = list.cbegin();
    std::size_t i = 0;

    while (it != list.cend() && i < index)
    {
      ++it;
      ++i;
    }

    if (it == list.cend())
    {
      exists = false;
      return 0;
    }

    exists = true;
    return *it;
  }

  std::size_t maxSequenceLength(const sadovnik::List< sadovnik::Seq > & sequences)
  {
    std::size_t maxLen = 0;
    for (sadovnik::LCIter< sadovnik::Seq > it = sequences.cbegin();
      it != sequences.cend();
      ++it)
    {
      const std::size_t cur = (*it).second.size();
      if (cur > maxLen)
      {
        maxLen = cur;
      }
    }
    return maxLen;
  }

  void readNumbers(std::istream & in, sadovnik::List< std::size_t > & numbers)
  {
    if (atEndOfLine(in))
    {
      consumeLineTerminator(in);
      return;
    }

    while (true)
    {
      std::size_t number = 0;
      if (!(in >> number))
      {
        if (in.eof())
        {
          in.clear();
        }
        else
        {
          in.clear();
          in.ignore(std::numeric_limits< std::streamsize >::max(), '\n');
        }
        break;
      }
      numbers.pushBack(number);

      const int ch = in.peek();
      if (ch == '\n' || ch == '\r')
      {
        consumeLineTerminator(in);
        break;
      }
      if (ch == std::char_traits< char >::eof())
      {
        break;
      }
    }
  }

}

namespace sadovnik
{

  List< Seq > readData(std::istream & in)
  {
    List< Seq > sequences;
    std::string name;

    while (in >> name)
    {
      List< std::size_t > numbers;
      readNumbers(in, numbers);
      sequences.pushBack(Seq(name, numbers));
    }

    return sequences;
  }

  void printNames(std::ostream & out, const List< Seq > & sequences)
  {
    if (sequences.empty())
    {
      return;
    }

    bool first = true;
    for (LCIter< Seq > it = sequences.cbegin(); it != sequences.cend(); ++it)
    {
      if (!first)
      {
        out << ' ';
      }
      out << (*it).first;
      first = false;
    }
    out << '\n';
  }

  void printTransposed(std::ostream & out, const List< Seq > & sequences)
  {
    if (sequences.empty())
    {
      return;
    }

    const std::size_t maxLen = maxSequenceLength(sequences);
    if (maxLen == 0)
    {
      return;
    }

    for (std::size_t i = 0; i < maxLen; ++i)
    {
      bool firstCol = true;
      bool printAny = false;

      for (LCIter< Seq > it = sequences.cbegin(); it != sequences.cend(); ++it)
      {
        bool exists = false;
        const std::size_t value = getElementAt((*it).second, i, exists);
        if (exists)
        {
          if (!firstCol)
          {
            out << ' ';
          }
          out << value;
          firstCol = false;
          printAny = true;
        }
      }

      if (printAny)
      {
        out << '\n';
      }
    }
  }

  List< std::size_t > calculateSums(const List< Seq > & sequences)
  {
    List< std::size_t > sums;
    if (sequences.empty())
    {
      return sums;
    }

    const std::size_t maxLen = maxSequenceLength(sequences);
    if (maxLen == 0)
    {
      return sums;
    }

    bool overflow = false;
    for (std::size_t i = 0; i < maxLen; ++i)
    {
      std::size_t rowSum = 0;
      bool hasAny = false;

      for (LCIter< Seq > it = sequences.cbegin(); it != sequences.cend(); ++it)
      {
        bool exists = false;
        const std::size_t value = getElementAt((*it).second, i, exists);
        if (exists)
        {
          if (rowSum > std::numeric_limits< std::size_t >::max() - value)
          {
            overflow = true;
          }
          rowSum += value;
          hasAny = true;
        }
      }

      if (hasAny)
      {
        sums.pushBack(rowSum);
      }
    }

    if (overflow)
    {
      throw std::overflow_error("integer overflow");
    }

    return sums;
  }

  void printSums(std::ostream & out, const List< std::size_t > & sums)
  {
    if (sums.empty())
    {
      return;
    }

    bool first = true;
    for (LCIter< std::size_t > it = sums.cbegin(); it != sums.cend(); ++it)
    {
      if (!first)
      {
        out << ' ';
      }
      out << *it;
      first = false;
    }
    out << '\n';
  }

}
