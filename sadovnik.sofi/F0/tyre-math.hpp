#ifndef F0_TYRE_MATH_HPP
#define F0_TYRE_MATH_HPP

#include "session-types.hpp"

#include "session.hpp"

namespace sadovnik
{

  double lapTime(const Session & session, const Stint & stint, unsigned lap_index);

}

#endif
