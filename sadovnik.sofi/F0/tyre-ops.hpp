#ifndef F0_TYRE_OPS_HPP
#define F0_TYRE_OPS_HPP

#include "session-types.hpp"

#include "session.hpp"

#include <list.hpp>

#include <iosfwd>
#include <string>

namespace sadovnik
{

  bool tyreNameMatchesType(const std::string & name, TyreType type);
  bool parseAddTyreTokens(const List< std::string > & tokens, std::string & name,
                          TyreSpec & spec);
  bool parseTyreLineTokens(const List< std::string > & tokens, std::string & name,
                           TyreSpec & spec);
  void printTyres(const Session & session, std::ostream & out);

}

#endif
