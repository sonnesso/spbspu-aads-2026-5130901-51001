#ifndef F0_IO_HPP
#define F0_IO_HPP

#include "session.hpp"

#include <iosfwd>
#include <string>

namespace sadovnik
{

  bool hasDatExtension(const std::string & filename);
  void writeSessionHeader(std::ostream & out);
  void writeSessionTrack(std::ostream & out, const TrackSpec & track);
  void writeSessionWeather(std::ostream & out, Weather weather,
                            unsigned humidity);
  void writeSessionEnd(std::ostream & out);
  void writeSession(const Session & session, const std::string & filename);
  Session readSession(const std::string & filename);

}

#endif
