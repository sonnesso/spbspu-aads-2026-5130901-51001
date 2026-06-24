#include "session-types.hpp"

namespace sadovnik
{

  bool parseTyreKind(const std::string & text, TyreKind & kind)
  {
    if (text == "slick")
    {
      kind = TyreKind::Slick;
      return true;
    }
    if (text == "inter")
    {
      kind = TyreKind::Inter;
      return true;
    }
    if (text == "wet")
    {
      kind = TyreKind::Wet;
      return true;
    }

    return false;
  }

  const char * tyreKindToString(TyreKind kind)
  {
    switch (kind)
    {
    case TyreKind::Slick:
      return "slick";
    case TyreKind::Inter:
      return "inter";
    case TyreKind::Wet:
      return "wet";
    }

    return "slick";
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

}
