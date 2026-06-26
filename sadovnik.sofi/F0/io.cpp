#include "io.hpp"

#include "io-format.hpp"
#include "session-types.hpp"

#include <string-utils.hpp>

#include <fstream>
#include <iomanip>
#include <stdexcept>
#include <string>

namespace
{

  using sadovnik::List;
  using sadovnik::Session;
  using sadovnik::Weather;

  const char INVALID_FILE[] = "invalid session file";

  std::string tokenAt(const List< std::string > & tokens, std::size_t index)
  {
    std::size_t cur = 0;
    for (auto it = tokens.begin(); it != tokens.end(); ++it)
    {
      if (cur == index)
      {
        return *it;
      }
      ++cur;
    }

    throw std::logic_error(INVALID_FILE);
  }

  bool readNonEmptyLine(std::istream & in, std::string & line)
  {
    while (std::getline(in, line))
    {
      if (!sadovnik::splitTokens(line).empty())
      {
        return true;
      }
    }

    return false;
  }

  void parseTrackLine(const List< std::string > & tokens, Session & session)
  {
    if (tokens.size() != 4)
    {
      throw std::logic_error(INVALID_FILE);
    }

    double length_km = 0.0;
    unsigned long long laps = 0;
    double base_lap_s = 0.0;

    if (!sadovnik::parseDouble(tokenAt(tokens, 1), length_km) || length_km < 0.0)
    {
      throw std::logic_error(INVALID_FILE);
    }

    if (!sadovnik::parseUnsigned(tokenAt(tokens, 2), laps) || laps == 0)
    {
      throw std::logic_error(INVALID_FILE);
    }

    if (!sadovnik::parseDouble(tokenAt(tokens, 3), base_lap_s) ||
        base_lap_s <= 0.0)
    {
      throw std::logic_error(INVALID_FILE);
    }

    session.setTrack(length_km, static_cast< unsigned >(laps), base_lap_s);
  }

  void parseWeatherLine(const List< std::string > & tokens, Session & session)
  {
    if (tokens.size() != 3)
    {
      throw std::logic_error(INVALID_FILE);
    }

    Weather weather = Weather::Dry;
    if (!sadovnik::parseWeather(tokenAt(tokens, 1), weather))
    {
      throw std::logic_error(INVALID_FILE);
    }

    unsigned long long humidity = 0;
    if (!sadovnik::parseUnsigned(tokenAt(tokens, 2), humidity) ||
        humidity > 100)
    {
      throw std::logic_error(INVALID_FILE);
    }

    session.setWeather(weather);
    session.setHumidity(static_cast< unsigned >(humidity));
  }

  void parseBodyLine(const List< std::string > & tokens, Session & session)
  {
    const std::string & tag = tokenAt(tokens, 0);
    if (tag == "track")
    {
      parseTrackLine(tokens, session);
      return;
    }
    if (tag == "weather")
    {
      parseWeatherLine(tokens, session);
      return;
    }
    if (tag == "tyre" || tag == "strategy")
    {
      throw std::logic_error("tyre/strategy blocks not implemented yet");
    }

    throw std::logic_error(INVALID_FILE);
  }

  void readMagicLine(const List< std::string > & tokens)
  {
    if (tokens.size() != 2)
    {
      throw std::logic_error(INVALID_FILE);
    }

    if (tokenAt(tokens, 0) != sadovnik::ioformat::SESSION_MAGIC)
    {
      throw std::logic_error(INVALID_FILE);
    }

    unsigned long long version = 0;
    if (!sadovnik::parseUnsigned(tokenAt(tokens, 1), version) ||
        version != sadovnik::ioformat::FORMAT_VERSION)
    {
      throw std::logic_error(INVALID_FILE);
    }
  }

}

namespace sadovnik
{

  bool hasDatExtension(const std::string & filename)
  {
    const std::string suffix = ".dat";
    if (filename.size() < suffix.size())
    {
      return false;
    }

    return filename.compare(filename.size() - suffix.size(), suffix.size(),
                            suffix) == 0;
  }

  void writeSessionHeader(std::ostream & out)
  {
    out << ioformat::SESSION_MAGIC << ' ' << ioformat::FORMAT_VERSION << '\n';
  }

  void writeSessionTrack(std::ostream & out, const TrackSpec & track)
  {
    if (!track.is_set)
    {
      return;
    }

    out << std::fixed << std::setprecision(3);
    out << "track " << track.length_km << ' ' << track.laps << ' '
        << track.base_lap_s << '\n';
  }

  void writeSessionWeather(std::ostream & out, Weather weather,
                           unsigned humidity)
  {
    out << "weather " << weatherToString(weather) << ' ' << humidity << '\n';
  }

  void writeSessionEnd(std::ostream & out)
  {
    out << "end\n";
  }

  void writeSession(const Session & session, const std::string & filename)
  {
    if (!hasDatExtension(filename))
    {
      throw std::logic_error("filename must end with .dat");
    }

    std::ofstream out(filename.c_str());
    if (!out)
    {
      throw std::runtime_error("cannot open file for writing");
    }

    writeSessionHeader(out);
    writeSessionTrack(out, session.track());
    writeSessionWeather(out, session.weather(), session.humidity());
    writeSessionEnd(out);

    if (!out)
    {
      throw std::runtime_error("cannot write session file");
    }
  }

  Session readSession(const std::string & filename)
  {
    if (!hasDatExtension(filename))
    {
      throw std::logic_error("filename must end with .dat");
    }

    std::ifstream in(filename.c_str());
    if (!in)
    {
      throw std::runtime_error("cannot open file for reading");
    }

    std::string line;
    if (!readNonEmptyLine(in, line))
    {
      throw std::logic_error(INVALID_FILE);
    }

    readMagicLine(splitTokens(line));

    Session session;
    while (readNonEmptyLine(in, line))
    {
      const List< std::string > tokens = splitTokens(line);
      if (tokens.empty())
      {
        continue;
      }

      if (tokenAt(tokens, 0) == "end")
      {
        session.clearDirty();
        return session;
      }

      parseBodyLine(tokens, session);
    }

    throw std::logic_error(INVALID_FILE);
  }

}
