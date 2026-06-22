#ifndef GRAPH_HPP
#define GRAPH_HPP

#include <hash-functions.hpp>
#include <hash-table.hpp>
#include <list.hpp>

#include <cstddef>
#include <string>

namespace sadovnik
{

  class Graph
  {
  public:
    using Weight = unsigned long long;

    struct WeightedVertex
    {
      std::string vertex;
      List< Weight > weights;
    };

    Graph();

    bool hasVertex(const std::string & vertex) const;
    void addVertex(const std::string & vertex);
    void bind(const std::string & from, const std::string & to, Weight weight);
    bool cut(const std::string & from, const std::string & to, Weight weight);

    List< std::string > getVertexes() const;
    List< WeightedVertex > getOutbound(const std::string & vertex) const;
    List< WeightedVertex > getInbound(const std::string & vertex) const;

    std::size_t vertexCount() const noexcept;
    std::size_t edgeKeyCount() const noexcept;

    Graph merge(const Graph & other) const;
    Graph extract(const List< std::string > & vertices) const;

  private:
    using VertexTab = HashTable< std::string, bool, StrHash, StrEq >;
    using EdgeTab = HashTable< EdgeKey, List< Weight >, EdgeKeyHash, EdgeKeyEq >;

    VertexTab vertices_;
    EdgeTab edges_;

    void ensureVertexCap();
    void ensureEdgeCap();
  };

}

#endif
