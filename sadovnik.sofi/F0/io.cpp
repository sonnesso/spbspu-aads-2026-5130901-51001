#include "io.hpp"

#include "io-format.hpp"
#include "session-types.hpp"
#include "strategy-ops.hpp"
#include "tyre-ops.hpp"

#include <string-utils.hpp>

#include <fstream>
#include <iomanip>
#include <stdexcept>
#include <string>

namespace
{

  using sadovnik::List;
  using sadovnik::Session;
  using sadovnik::Stint;
  using sadovnik::TyreSpec;
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

  void parseTyreLine(const List< std::string > & tokens, Session & session)
  {
    std::string name;
    TyreSpec spec;
    if (!sadovnik::parseTyreLineTokens(tokens, name, spec))
    {
      throw std::logic_error(INVALID_FILE);
    }

    if (session.tyres().has(name))
    {
      throw std::logic_error(INVALID_FILE);
    }

    session.tyres().add(name, spec);
    session.addTyreName(name);
  }

  void parseStrategyLine(const List< std::string > & tokens, Session & session)
  {
    std::string name;
    List< Stint > stints;
    if (!sadovnik::parseStrategyLineTokens(tokens, name, stints))
    {
      throw std::logic_error(INVALID_FILE);
    }

    if (session.strategies().has(name))
    {
      throw std::logic_error(INVALID_FILE);
    }

    if (!sadovnik::isCreateStrategyStintsValid(session, stints))
    {
      throw std::logic_error(INVALID_FILE);
    }

    session.strategies().add(name, stints);
    session.addStrategyName(name);
  }

  void parseBodyLine(const List< std::string > & tokens, Session & session,
                     std::string * preset_name)
  {
    const std::string & tag = tokenAt(tokens, 0);
    if (tag == "preset_name")
    {
      if (preset_name == nullptr || tokens.size() < 2)
      {
        throw std::logic_error(INVALID_FILE);
      }

      *preset_name = tokenAt(tokens, 1);
      for (std::size_t i = 2; i < tokens.size(); ++i)
      {
        *preset_name += ' ';
        *preset_name += tokenAt(tokens, i);
      }
      return;
    }
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
    if (tag == "tyre")
    {
      parseTyreLine(tokens, session);
      return;
    }
    if (tag == "strategy")
    {
      parseStrategyLine(tokens, session);
      return;
    }

    throw std::logic_error(INVALID_FILE);
  }

  void checkDatHeader(const List< std::string > & tokens, const char * proved)
  {
    if (tokens.size() != 2)
    {
      throw std::logic_error(INVALID_FILE);
    }

    if (tokenAt(tokens, 0) != proved)
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

  bool fileReadable(const std::string & path)
  {
    std::ifstream in(path.c_str());
    return static_cast< bool >(in);
  }

  Session readSessionFromStream(std::istream & in, const char * proved,
                                std::string * preset_name)
  {
    std::string line;
    if (!readNonEmptyLine(in, line))
    {
      throw std::logic_error(INVALID_FILE);
    }

    checkDatHeader(sadovnik::splitTokens(line), proved);

    Session session;
    while (readNonEmptyLine(in, line))
    {
      const List< std::string > tokens = sadovnik::splitTokens(line);
      if (tokens.empty())
      {
        continue;
      }

      if (tokenAt(tokens, 0) == "end")
      {
        session.clearDirty();
        return session;
      }

      parseBodyLine(tokens, session, preset_name);
    }

    throw std::logic_error(INVALID_FILE);
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
    out << ioformat::SESSION_PROVED << ' ' << ioformat::FORMAT_VERSION << '\n';
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

  void writeSessionTyres(std::ostream & out, const Session & session)
  {
    out << std::fixed << std::setprecision(2);
    for (auto it = session.tyreNames().begin(); it != session.tyreNames().end();
         ++it)
    {
      const TyreSpec & spec = session.tyres().get(*it);
      out << "tyre " << *it << ' ' << tyreTypeToString(spec.type) << ' '
          << spec.degr << ' ' << spec.max_laps << ' ' << spec.pit_time;
      if (!spec.compound.empty())
      {
        out << ' ' << spec.compound;
      }
      out << '\n';
    }
  }

  void writeSessionStrategies(std::ostream & out, const Session & session)
  {
    for (auto it = session.strategyNames().begin();
         it != session.strategyNames().end(); ++it)
    {
      const List< Stint > & stints = session.strategies().get(*it);
      out << "strategy " << *it;
      for (auto sit = stints.begin(); sit != stints.end(); ++sit)
      {
        out << ' ' << sit->tyre_name << ' ' << sit->laps;
      }
      out << '\n';
    }
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
    writeSessionTyres(out, session);
    writeSessionStrategies(out, session);
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

    return readSessionFromStream(in, ioformat::SESSION_PROVED, nullptr);
  }

  std::string resolvePresetPath(const std::string & filename)
  {
    if (fileReadable(filename))
    {
      return filename;
    }

    const std::string in_presets = std::string("presets/") + filename;
    if (fileReadable(in_presets))
    {
      return in_presets;
    }

    const std::string in_lab =
      std::string("sadovnik.sofi/F0/presets/") + filename;
    if (fileReadable(in_lab))
    {
      return in_lab;
    }

    return std::string();
  }

  Session readPreset(const std::string & filename, std::string & display_name)
  {
    if (!hasDatExtension(filename))
    {
      throw std::logic_error("filename must end with .dat");
    }

    const std::string path = resolvePresetPath(filename);
    if (path.empty())
    {
      throw std::runtime_error("cannot open file for reading");
    }

    std::ifstream in(path.c_str());
    if (!in)
    {
      throw std::runtime_error("cannot open file for reading");
    }

    display_name.clear();
    Session session =
      readSessionFromStream(in, ioformat::PRESET_PROVED, &display_name);
    if (display_name.empty())
    {
      display_name = "Preset";
    }
    return session;
  }

  void printPresetLoaded(const Session & session, const std::string & display_name,
                         std::ostream & out)
  {
    const TrackSpec & track = session.track();
    out << std::fixed << std::setprecision(3);
    out << "Preset loaded: " << display_name << " (" << track.length_km
        << " km, " << track.laps << " laps)\n";

    List< std::string > tyre_names;
    for (auto it = session.tyreNames().begin(); it != session.tyreNames().end();
         ++it)
    {
      tyre_names.pushBack(*it);
    }

    for (auto it = tyre_names.begin(); it != tyre_names.end(); ++it)
    {
      auto best = it;
      for (auto jt = it; jt != tyre_names.end(); ++jt)
      {
        if (*jt < *best)
        {
          best = jt;
        }
      }
      if (best != it)
      {
        std::string tmp = *it;
        *it = *best;
        *best = tmp;
      }
    }

    out << "Tyres: ";
    bool first = true;
    for (auto it = tyre_names.begin(); it != tyre_names.end(); ++it)
    {
      if (!first)
      {
        out << ", ";
      }
      first = false;
      out << *it;
    }
    out << '\n';

    out << "Strategies: ";
    first = true;
    for (auto it = session.strategyNames().begin();
         it != session.strategyNames().end(); ++it)
    {
      if (!first)
      {
        out << ", ";
      }
      first = false;
      out << *it;
    }
    out << '\n';
  }

}
