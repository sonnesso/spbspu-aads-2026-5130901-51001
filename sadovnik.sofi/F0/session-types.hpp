#ifndef F0_SESSION_TYPES_HPP
#define F0_SESSION_TYPES_HPP

#include <string>

namespace sadovnik
{

  enum class Weather
  {
    Dry,
    Damp,
    Wet
  };

  enum class TyreKind
  {
    Slick,
    Inter,
    Wet
  };

  struct TyreSpec
  {
    TyreKind kind;
    std::string compound;
    double degr;
    unsigned max_laps;
    unsigned pit_time;
    double base_offset;

    TyreSpec()
      : kind(TyreKind::Slick),
        compound(),
        degr(0.0),
        max_laps(0),
        pit_time(0),
        base_offset(0.0)
    {
    }
  };

  struct Stint
  {
    std::string tyre_name;
    unsigned laps;

    Stint()
      : tyre_name(),
        laps(0)
    {
    }

    Stint(const std::string & name, unsigned lap_cnt)
      : tyre_name(name),
        laps(lap_cnt)
    {
    }
  };

  struct TrackSpec
  {
    bool is_set;
    double length_km;
    unsigned laps;
    double base_lap_s;

    TrackSpec()
      : is_set(false),
        length_km(0.0),
        laps(0),
        base_lap_s(90.0)
    {
    }
  };

}

#endif
