#ifndef IO_HPP
#define IO_HPP

#include <iosfwd>
#include <string>
#include <utility>

#include "list.hpp"

namespace sadovnik
{

  using Seq = std::pair< std::string, List< std::size_t > >;

  List< Seq > readData(std::istream & in);
  void printNames(std::ostream & out, const List< Seq > & sequences);
  void printTransposed(std::ostream & out, const List< Seq > & sequences);
  List< std::size_t > calculateSums(const List< Seq > & sequences);
  void printSums(std::ostream & out, const List< std::size_t > & sums);

}

#endif
