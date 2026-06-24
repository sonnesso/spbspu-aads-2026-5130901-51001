#ifndef F0_IO_HPP
#define F0_IO_HPP

#include "session.hpp"

#include <iosfwd>
#include <string>

namespace sadovnik
{

  void writeSession(const Session & session, const std::string & filename);
  Session readSession(const std::string & filename);

}

#endif
