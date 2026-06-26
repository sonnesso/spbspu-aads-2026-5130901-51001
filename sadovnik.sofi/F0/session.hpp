#ifndef F0_SESSION_HPP
#define F0_SESSION_HPP

#include "session-tab.hpp"

#include <list.hpp>

#include <string>

namespace sadovnik
{

  class Session
  {
  public:
    Session();

    const TrackSpec & track() const;
    TrackSpec & track();

    Weather weather() const;
    void setWeather(Weather value);

    unsigned humidity() const;
    void setHumidity(unsigned value);

    bool isDirty() const;
    void markDirty();
    void clearDirty();

    TyreTab & tyres();
    const TyreTab & tyres() const;

    void addTyreName(const std::string & name);
    const List< std::string > & tyreNames() const;

    StrategyTab & strategies();
    const StrategyTab & strategies() const;

    void addStrategyName(const std::string & name);
    void removeStrategyName(const std::string & name);
    const List< std::string > & strategyNames() const;

    void setTrack(double length_km, unsigned laps, double base_lap_s);

  private:
    TrackSpec track_;
    Weather weather_;
    unsigned humidity_;
    bool dirty_;
    TyreTab tyres_;
    List< std::string > tyre_names_;
    StrategyTab strategies_;
    List< std::string > strategy_names_;
  };

}

#endif
