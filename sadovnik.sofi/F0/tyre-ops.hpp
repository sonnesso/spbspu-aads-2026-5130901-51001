#ifndef F0_TYRE_OPS_HPP
#define F0_TYRE_OPS_HPP

#include "session-types.hpp"

#include <list.hpp>

#include <string>

namespace sadovnik
{

  bool tyreNameMatchesKind(const std::string & name, TyreKind kind);
  bool parseAddTyreTokens(const List< std::string > & tokens, std::string & name,
                          TyreSpec & spec);

}

#endif
