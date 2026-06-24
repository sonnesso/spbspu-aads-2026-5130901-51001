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

  double baseOffsetForTyre(TyreKind kind, const std::string & compound)
  {
    if (kind == TyreKind::Inter)
    {
      return 12.0;
    }
    if (kind == TyreKind::Wet)
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

}
