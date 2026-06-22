#ifndef IO_HPP
#define IO_HPP

#include "graph.hpp"

#include <hash-functions.hpp>
#include <hash-table.hpp>

#include <iosfwd>
#include <string>

namespace sadovnik
{

  using GraphTab = HashTable< std::string, Graph, StrHash, StrEq >;

  GraphTab readGraphs(std::istream & in);

}

#endif
