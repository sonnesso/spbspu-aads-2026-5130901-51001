#include "session-types.hpp"

#include <iomanip>
#include <ostream>

namespace sadovnik
{

  bool parseTyreType(const std::string & text, TyreType & type)
  {
    if (text == "slick")
    {
      type = TyreType::Slick;
      return true;
    }
    if (text == "inter")
    {
      type = TyreType::Inter;
      return true;
    }
    if (text == "wet")
    {
      type = TyreType::Wet;
      return true;
    }

    return false;
  }

  const char * tyreTypeToString(TyreType type)
  {
    switch (type)
    {
    case TyreType::Slick:
      return "slick";
    case TyreType::Inter:
      return "inter";
    case TyreType::Wet:
      return "wet";
    }

    return "slick";
  }

  bool parseCompound(const std::string & text, std::string & compound)
  {
    if (text == "C1")
    {
      compound = "C1";
      return true;
    }
    if (text == "C2")
    {
      compound = "C2";
      return true;
    }
    if (text == "C3")
    {
      compound = "C3";
      return true;
    }
    if (text == "C4")
    {
      compound = "C4";
      return true;
    }
    if (text == "C5")
    {
      compound = "C5";
      return true;
    }

    return false;
  }

  double baseOffsetForTyre(TyreType type, const std::string & compound)
  {
    if (type == TyreType::Inter)
    {
      return 12.0;
    }
    if (type == TyreType::Wet)
    {
      return 20.0;
    }

    if (compound == "C1")
    {
      return 2.0;
    }
    if (compound == "C2")
    {
      return 1.0;
    }
    if (compound == "C3" || compound.empty())
    {
      return 0.0;
    }
    if (compound == "C4")
    {
      return -0.8;
    }
    if (compound == "C5")
    {
      return -1.5;
    }

    return 0.0;
  }

  bool parseWeather(const std::string & text, Weather & weather)
  {
    if (text == "dry")
    {
      weather = Weather::Dry;
      return true;
    }
    if (text == "damp")
    {
      weather = Weather::Damp;
      return true;
    }
    if (text == "wet")
    {
      weather = Weather::Wet;
      return true;
    }

    return false;
  }

  const char * weatherToString(Weather weather)
  {
    switch (weather)
    {
    case Weather::Dry:
      return "dry";
    case Weather::Damp:
      return "damp";
    case Weather::Wet:
      return "wet";
    }

    return "dry";
  }

  const char * humidityTrend(unsigned percent)
  {
    if (percent < 50)
    {
      return "drying";
    }
    if (percent > 50)
    {
      return "wetting";
    }

    return "stable";
  }

  void printTrackSetLine(const TrackSpec & track, std::ostream & out)
  {
    out << "Track set: ";
    out << std::fixed << std::setprecision(3) << track.length_km << " km, "
        << track.laps << " laps, base lap ";
    out << std::setprecision(1) << track.base_lap_s << " s\n";
  }

  void printWeatherSetLine(Weather weather, std::ostream & out)
  {
    out << "Weather set: " << weatherToString(weather) << " (";
    switch (weather)
    {
    case Weather::Dry:
      out << "Slick tyres may be faster";
      break;
    case Weather::Damp:
      out << "Intermediate tyres may be faster";
      break;
    case Weather::Wet:
      out << "Wet tyres may be faster";
      break;
    }
    out << ")\n";
  }

  void printHumiditySetLine(unsigned percent, std::ostream & out)
  {
    out << "Track humidity: " << percent << "% (" << humidityTrend(percent)
        << ")\n";
  }

}
