#ifndef DATASET_TAB_HPP
#define DATASET_TAB_HPP

#include <bs-tree.hpp>
#include <hash-functions.hpp>
#include <hash-table.hpp>

#include <functional>
#include <string>

namespace sadovnik
{

  using IntStrTree = BSTree< int, std::string, std::less< int > >;
  using DatasetTab = HashTable< std::string, IntStrTree, StrHash, StrEq >;

}

#endif
