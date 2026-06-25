#ifndef S4_IO_HPP
#define S4_IO_HPP

#include "dataset-tab.hpp"

#include <iosfwd>

namespace sadovnik
{

  DatasetTab readDatasets(std::istream & in);

}

#endif
