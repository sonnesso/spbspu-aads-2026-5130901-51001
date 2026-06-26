#include "tyre-math.hpp"

#include <stdexcept>

namespace sadovnik
{

  double lapTime(const Session & session, const Stint & stint, unsigned lap_index)
  {
    if (lap_index == 0)
    {
      throw std::logic_error("invalid lap index");
    }

    const TyreSpec & tyre = session.tyres().get(stint.tyre_name);
    const unsigned wear_laps = lap_index - 1;
    return session.track().base_lap_s + tyre.base_offset +
           static_cast< double >(wear_laps) * tyre.degr;
  }

  double stintTotalTime(const Session & session, const Stint & stint)
  {
    double total = 0.0;
    for (unsigned lap = 1; lap <= stint.laps; ++lap)
    {
      total += lapTime(session, stint, lap);
    }

    return total;
  }

  double strategyRaceTime(const Session & session, const List< Stint > & stints)
  {
    double total = 0.0;
    bool first_stint = true;
    for (auto it = stints.begin(); it != stints.end(); ++it)
    {
      if (!first_stint)
      {
        const TyreSpec & tyre = session.tyres().get(it->tyre_name);
        total += static_cast< double >(tyre.pit_time);
      }
      first_stint = false;
      total += stintTotalTime(session, *it);
    }

    return total;
  }

}
