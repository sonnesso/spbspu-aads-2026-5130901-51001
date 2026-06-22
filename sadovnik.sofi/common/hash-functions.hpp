#ifndef HASH_FUNCTIONS_HPP
#define HASH_FUNCTIONS_HPP

#include <boost/hash2/blake2.hpp>
#include <boost/hash2/get_integral_result.hpp>
#include <boost/hash2/hash_append.hpp>

#include <cstddef>
#include <string>

namespace sadovnik
{

  struct StrEq
  {
    bool operator()(const std::string & lhs, const std::string & rhs) const
    {
      return lhs == rhs;
    }
  };

  struct StrHash
  {
    std::size_t operator()(const std::string & key) const
    {
      boost::hash2::blake2b_512 hasher;
      boost::hash2::hash_append(hasher, boost::hash2::default_flavor(), key);
      return boost::hash2::get_integral_result<std::size_t>(hasher);
    }
  };

  struct EdgeKey
  {
    std::string from;
    std::string to;
  };

  struct EdgeKeyEq
  {
    bool operator()(const EdgeKey & lhs, const EdgeKey & rhs) const
    {
      return lhs.from == rhs.from && lhs.to == rhs.to;
    }
  };

  struct EdgeKeyHash
  {
    std::size_t operator()(const EdgeKey & key) const
    {
      boost::hash2::blake2b_512 hasher;
      const boost::hash2::default_flavor flavor;
      boost::hash2::hash_append(hasher, flavor, key.from);
      boost::hash2::hash_append(hasher, flavor, key.to);
      return boost::hash2::get_integral_result<std::size_t>(hasher);
    }
  };

}

#endif
