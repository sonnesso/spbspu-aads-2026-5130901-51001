#include "graph.hpp"

namespace
{

  struct StrLess
  {
    bool operator()(const std::string & lhs, const std::string & rhs) const
    {
      return lhs < rhs;
    }
  };

  struct WeightLess
  {
    bool operator()(sadovnik::Graph::Weight lhs, sadovnik::Graph::Weight rhs) const
    {
      return lhs < rhs;
    }
  };

  struct WeightedLess
  {
    bool operator()(const sadovnik::Graph::WeightedVertex & lhs,
                    const sadovnik::Graph::WeightedVertex & rhs) const
    {
      return lhs.vertex < rhs.vertex;
    }
  };

  template< class T, class Less >
  void insertSorted(sadovnik::List< T > & list, const T & value, Less less)
  {
    sadovnik::List< T > result;
    bool placed = false;

    for (auto it = list.begin(); it != list.end(); ++it)
    {
      if (!placed && !less(*it, value))
      {
        result.pushBack(value);
        placed = true;
      }
      result.pushBack(*it);
    }

    if (!placed)
    {
      result.pushBack(value);
    }

    list = result;
  }

  sadovnik::Graph::WeightedVertex * findWeighted(
    sadovnik::List< sadovnik::Graph::WeightedVertex > & list,
    const std::string & vertex)
  {
    for (auto it = list.begin(); it != list.end(); ++it)
    {
      if (it->vertex == vertex)
      {
        return &(*it);
      }
    }
    return nullptr;
  }

  void addWeighted(sadovnik::List< sadovnik::Graph::WeightedVertex > & list,
                   const std::string & vertex,
                   sadovnik::Graph::Weight weight)
  {
    sadovnik::Graph::WeightedVertex * item = findWeighted(list, vertex);
    if (item != nullptr)
    {
      insertSorted(item->weights, weight, WeightLess());
      return;
    }

    sadovnik::Graph::WeightedVertex created;
    created.vertex = vertex;
    created.weights.pushBack(weight);
    insertSorted(list, created, WeightedLess());
  }

  bool removeWeight(sadovnik::List< sadovnik::Graph::Weight > & weights,
                    sadovnik::Graph::Weight weight)
  {
    sadovnik::List< sadovnik::Graph::Weight > kept;
    bool removed = false;

    for (auto it = weights.begin(); it != weights.end(); ++it)
    {
      if (!removed && *it == weight)
      {
        removed = true;
      }
      else
      {
        kept.pushBack(*it);
      }
    }

    if (removed)
    {
      weights = kept;
    }

    return removed;
  }

}

namespace sadovnik
{

  namespace detail
  {
    const std::size_t INIT_SLOT_CNT = 16;
  }

  Graph::Graph()
    : vertices_(detail::INIT_SLOT_CNT),
      edges_(detail::INIT_SLOT_CNT)
  {
  }

  bool Graph::hasVertex(const std::string & vertex) const
  {
    return vertices_.has(vertex);
  }

  void Graph::addVertex(const std::string & vertex)
  {
    if (vertices_.has(vertex))
    {
      return;
    }

    ensureVertexCap();
    vertices_.add(vertex, true);
  }

  void Graph::bind(const std::string & from, const std::string & to, Weight weight)
  {
    const bool newFrom = !hasVertex(from);
    const bool newTo = !hasVertex(to);

    try
    {
      if (newFrom)
      {
        addVertex(from);
      }
      if (newTo)
      {
        addVertex(to);
      }

      EdgeKey key;
      key.from = from;
      key.to = to;

      List< Weight > * weights = edges_.find(key);
      if (weights == nullptr)
      {
        ensureEdgeCap();
        List< Weight > created;
        created.pushBack(weight);
        edges_.add(key, created);
        return;
      }

      weights->pushBack(weight);
    }
    catch (...)
    {
      if (newTo && hasVertex(to))
      {
        vertices_.drop(to);
      }
      if (newFrom && hasVertex(from))
      {
        vertices_.drop(from);
      }
      throw;
    }
  }

  bool Graph::cut(const std::string & from, const std::string & to, Weight weight)
  {
    EdgeKey key;
    key.from = from;
    key.to = to;

    List< Weight > * weights = edges_.find(key);
    if (weights == nullptr)
    {
      return false;
    }

    if (!removeWeight(*weights, weight))
    {
      return false;
    }

    if (weights->empty())
    {
      edges_.drop(key);
    }

    return true;
  }

  List< std::string > Graph::getVertexes() const
  {
    List< std::string > result;

    for (auto it = vertices_.begin(); it != vertices_.end(); ++it)
    {
      insertSorted(result, it->key, StrLess());
    }

    return result;
  }

  List< Graph::WeightedVertex > Graph::getOutbound(const std::string & vertex) const
  {
    List< WeightedVertex > result;

    for (auto it = edges_.begin(); it != edges_.end(); ++it)
    {
      if (it->key.from == vertex)
      {
        for (auto w = it->val.begin(); w != it->val.end(); ++w)
        {
          addWeighted(result, it->key.to, *w);
        }
      }
    }

    return result;
  }

  List< Graph::WeightedVertex > Graph::getInbound(const std::string & vertex) const
  {
    List< WeightedVertex > result;

    for (auto it = edges_.begin(); it != edges_.end(); ++it)
    {
      if (it->key.to == vertex)
      {
        for (auto w = it->val.begin(); w != it->val.end(); ++w)
        {
          addWeighted(result, it->key.from, *w);
        }
      }
    }

    return result;
  }

  std::size_t Graph::vertexCount() const noexcept
  {
    return vertices_.size();
  }

  std::size_t Graph::edgeKeyCount() const noexcept
  {
    return edges_.size();
  }

  Graph Graph::merge(const Graph & other) const
  {
    Graph result;

    for (auto it = vertices_.begin(); it != vertices_.end(); ++it)
    {
      result.addVertex(it->key);
    }

    for (auto it = other.vertices_.begin(); it != other.vertices_.end(); ++it)
    {
      result.addVertex(it->key);
    }

    for (auto it = edges_.begin(); it != edges_.end(); ++it)
    {
      for (auto w = it->val.begin(); w != it->val.end(); ++w)
      {
        result.bind(it->key.from, it->key.to, *w);
      }
    }

    for (auto it = other.edges_.begin(); it != other.edges_.end(); ++it)
    {
      for (auto w = it->val.begin(); w != it->val.end(); ++w)
      {
        result.bind(it->key.from, it->key.to, *w);
      }
    }

    return result;
  }

  Graph Graph::extract(const List< std::string > & vertices) const
  {
    Graph result;

    for (auto it = vertices.begin(); it != vertices.end(); ++it)
    {
      if (hasVertex(*it))
      {
        result.addVertex(*it);
      }
    }

    for (auto it = edges_.begin(); it != edges_.end(); ++it)
    {
      if (result.hasVertex(it->key.from) && result.hasVertex(it->key.to))
      {
        for (auto w = it->val.begin(); w != it->val.end(); ++w)
        {
          result.bind(it->key.from, it->key.to, *w);
        }
      }
    }

    return result;
  }

  void Graph::ensureVertexCap()
  {
    if (vertices_.size() == vertices_.capacity())
    {
      vertices_.rehash(vertices_.slots() * 2);
    }
  }

  void Graph::ensureEdgeCap()
  {
    if (edges_.size() == edges_.capacity())
    {
      edges_.rehash(edges_.slots() * 2);
    }
  }

}
