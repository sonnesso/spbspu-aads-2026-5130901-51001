#ifndef F0_STRATEGY_OPS_HPP
#define F0_STRATEGY_OPS_HPP

#include "session-types.hpp"

#include "session.hpp"

#include <list.hpp>

#include <iosfwd>
#include <string>

namespace sadovnik
{

  bool parseCreateStrategyTokens(const List< std::string > & tokens,
                                 std::string & name, List< Stint > & stints);
  bool isCreateStrategyStintsValid(const Session & session,
                                   const List< Stint > & stints);
  void printStrategyCreatedLine(const std::string & name,
                                const List< Stint > & stints, std::ostream & out);
  void printStrategies(const Session & session, std::ostream & out);
  bool deleteStrategy(Session & session, const std::string & name,
                      std::ostream & out);
  bool validateStrategy(const Session & session, const std::string & name,
                        std::ostream & out);
  bool simulateStrategy(const Session & session, const std::string & name,
                        std::ostream & out);

}

#endif
