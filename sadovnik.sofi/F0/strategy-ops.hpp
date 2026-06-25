#ifndef F0_STRATEGY_OPS_HPP
#define F0_STRATEGY_OPS_HPP

#include "session-types.hpp"

#include <list.hpp>

#include <iosfwd>
#include <string>

namespace sadovnik
{

  bool parseCreateStrategyTokens(const List< std::string > & tokens,
                                 std::string & name, List< Stint > & stints);
  void printStrategyCreatedLine(const std::string & name,
                                const List< Stint > & stints, std::ostream & out);

}

#endif
