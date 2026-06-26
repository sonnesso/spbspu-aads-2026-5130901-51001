#ifndef F0_TYRE_MATH_HPP
#define F0_TYRE_MATH_HPP

#include "session-types.hpp"

#include "session.hpp"

#include <list.hpp>

namespace sadovnik
{

  double lapTime(const Session & session, const Stint & stint, unsigned lap_index);
  double stintTotalTime(const Session & session, const Stint & stint);
  double strategyRaceTime(const Session & session, const List< Stint > & stints);

}

#endif
