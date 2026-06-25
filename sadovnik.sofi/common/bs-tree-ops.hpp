#ifndef BS_TREE_OPS_HPP
#define BS_TREE_OPS_HPP

#include <bs-tree.hpp>

namespace sadovnik
{

  template< class Key, class Value, class Compare >
  BSTree< Key, Value, Compare > complement(
    const BSTree< Key, Value, Compare > & left,
    const BSTree< Key, Value, Compare > & right)
  {
    BSTree< Key, Value, Compare > result;
    for (auto it = left.cbegin(); it != left.cend(); ++it)
    {
      if (!right.has(it->first))
      {
        result.push(it->first, it->second);
      }
    }
    return result;
  }

  template< class Key, class Value, class Compare >
  BSTree< Key, Value, Compare > intersect(
    const BSTree< Key, Value, Compare > & left,
    const BSTree< Key, Value, Compare > & right)
  {
    BSTree< Key, Value, Compare > result;
    for (auto it = left.cbegin(); it != left.cend(); ++it)
    {
      if (right.has(it->first))
      {
        result.push(it->first, it->second);
      }
    }
    return result;
  }

  template< class Key, class Value, class Compare >
  BSTree< Key, Value, Compare > unite(
    const BSTree< Key, Value, Compare > & left,
    const BSTree< Key, Value, Compare > & right)
  {
    BSTree< Key, Value, Compare > result(left);
    for (auto it = right.cbegin(); it != right.cend(); ++it)
    {
      if (!result.has(it->first))
      {
        result.push(it->first, it->second);
      }
    }
    return result;
  }

}

#endif
