#include "io.hpp"

#include <fstream>
#include <stdexcept>

namespace sadovnik
{

  void writeSession(const Session &, const std::string &)
  {
    throw std::logic_error("not implemented");
  }

  Session readSession(const std::string &)
  {
    throw std::logic_error("not implemented");
  }

}
