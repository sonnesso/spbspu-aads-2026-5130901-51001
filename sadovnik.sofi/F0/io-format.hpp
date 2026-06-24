#ifndef F0_IO_FORMAT_HPP
#define F0_IO_FORMAT_HPP

/*
  Text .dat format (version 1), space-separated tokens per line:

  F0_SESSION 1
  track <length_km> <laps> <base_lap_s>   (length_km may be 0)
  weather <dry|damp|wet> <humidity_percent>
  tyre <name> <type> <degr> <max_laps> <pit_time> [<compound>]
  strategy <name> <tyre1> <laps1> [<tyre2> <laps2> ...]
  end

  Preset files use F0_PRESET 1 and optional:
  preset_name <display_name>

  Tyre/strategy lines: commits 24-25. Filename must end with .dat.
*/

namespace sadovnik
{

  namespace ioformat
  {

    const char SESSION_MAGIC[] = "F0_SESSION";
    const char PRESET_MAGIC[] = "F0_PRESET";
    const unsigned FORMAT_VERSION = 1;

  }

}

#endif
