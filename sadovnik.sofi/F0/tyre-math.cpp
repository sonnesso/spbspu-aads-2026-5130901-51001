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

}
