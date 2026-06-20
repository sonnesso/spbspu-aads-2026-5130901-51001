#define BOOST_TEST_MODULE S0
#include <boost/test/included/unit_test.hpp>
#include <sstream>
#include "credentials.hpp"

BOOST_AUTO_TEST_CASE(credentials_test)
{
  std::ostringstream out;
  yarmolinskaya::out_credentials(out);
  BOOST_TEST(out.str() == "yarmolinskaya.anna");
}
