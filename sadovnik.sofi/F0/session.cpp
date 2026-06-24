#include "session.hpp"

namespace sadovnik
{

  Session::Session()
    : track_(),
      weather_(Weather::Dry),
      humidity_(0),
      dirty_(false),
      tyres_(8, 4),
      strategies_(8, 4)
  {
  }

  const TrackSpec & Session::track() const
  {
    return track_;
  }

  TrackSpec & Session::track()
  {
    return track_;
  }

  Weather Session::weather() const
  {
    return weather_;
  }

  void Session::setWeather(Weather value)
  {
    weather_ = value;
    markDirty();
  }

  unsigned Session::humidity() const
  {
    return humidity_;
  }

  void Session::setHumidity(unsigned value)
  {
    humidity_ = value;
    markDirty();
  }

  bool Session::isDirty() const
  {
    return dirty_;
  }

  void Session::markDirty()
  {
    dirty_ = true;
  }

  void Session::clearDirty()
  {
    dirty_ = false;
  }

  TyreTab & Session::tyres()
  {
    return tyres_;
  }

  const TyreTab & Session::tyres() const
  {
    return tyres_;
  }

  void Session::addTyreName(const std::string & name)
  {
    tyre_names_.pushBack(name);
  }

  const List< std::string > & Session::tyreNames() const
  {
    return tyre_names_;
  }

  StrategyTab & Session::strategies()
  {
    return strategies_;
  }

  const StrategyTab & Session::strategies() const
  {
    return strategies_;
  }

  void Session::setTrack(double length_km, unsigned laps, double base_lap_s)
  {
    track_.is_set = true;
    track_.length_km = length_km;
    track_.laps = laps;
    track_.base_lap_s = base_lap_s;
    markDirty();
  }

}
