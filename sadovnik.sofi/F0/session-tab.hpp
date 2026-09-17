#ifndef F0_SESSION_TAB_HPP
#define F0_SESSION_TAB_HPP

#include "session-types.hpp"

#include <hash-functions.hpp>
#include <hash-table.hpp>
#include <list.hpp>

#include <string>

namespace sadovnik
{

  using StrategyStints = List< Stint >;
  using TyreTab = HashTable< std::string, TyreSpec, StrHash, StrEq >;
  using StrategyTab = HashTable< std::string, StrategyStints, StrHash, StrEq >;

}

#endif
