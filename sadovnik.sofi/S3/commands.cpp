#include "commands.hpp"

#include <string-utils.hpp>

#include <istream>
#include <sstream>
#include <stdexcept>
#include <string>

namespace
{

  using sadovnik::CommandContext;
  using sadovnik::Graph;
  using sadovnik::List;

  struct StrLess
  {
    bool operator()(const std::string & lhs, const std::string & rhs) const
    {
      return lhs < rhs;
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

  std::string tokenAt(const List< std::string > & tokens, std::size_t index)
  {
    std::size_t cur = 0;
    for (auto it = tokens.begin(); it != tokens.end(); ++it)
    {
      if (cur == index)
      {
        return *it;
      }
      ++cur;
    }

    throw std::logic_error("missing token");
  }

  bool parseWeight(const std::string & token, Graph::Weight & weight)
  {
    unsigned long long parsed = 0;
    if (!sadovnik::parseUnsigned(token, parsed))
    {
      return false;
    }

    weight = parsed;
    return true;
  }

  void ensureGraphSpace(sadovnik::GraphTab & graphs)
  {
    if (graphs.size() == graphs.capacity())
    {
      graphs.rehash(graphs.slots() * 2);
    }
  }

  List< std::string > getGraphNames(const sadovnik::GraphTab & graphs)
  {
    List< std::string > names;
    for (auto it = graphs.begin(); it != graphs.end(); ++it)
    {
      insertSorted(names, it->key, StrLess());
    }
    return names;
  }

  void printLines(std::ostream & out, const List< std::string > & values)
  {
    if (values.empty())
    {
      out << '\n';
      return;
    }

    for (auto it = values.begin(); it != values.end(); ++it)
    {
      out << *it << '\n';
    }
  }

  void printAdjacent(std::ostream & out, const List< Graph::WeightedVertex > & values)
  {
    if (values.empty())
    {
      out << '\n';
      return;
    }

    for (auto it = values.begin(); it != values.end(); ++it)
    {
      out << it->vertex;
      for (auto w = it->weights.begin(); w != it->weights.end(); ++w)
      {
        out << ' ' << *w;
      }
      out << '\n';
    }
  }

  bool graphsCmd(CommandContext & context, const List< std::string > & tokens,
                 std::ostream & out)
  {
    if (tokens.size() != 1)
    {
      return false;
    }

    printLines(out, getGraphNames(context.graphs()));
    return true;
  }

  bool vertexesCmd(CommandContext & context, const List< std::string > & tokens,
                   std::ostream & out)
  {
    if (tokens.size() != 2)
    {
      return false;
    }

    const std::string graphName = tokenAt(tokens, 1);
    if (!sadovnik::isValidName(graphName) || !context.graphs().has(graphName))
    {
      return false;
    }

    printLines(out, context.graphs().get(graphName).getVertexes());
    return true;
  }

  bool outboundCmd(CommandContext & context, const List< std::string > & tokens,
                   std::ostream & out)
  {
    if (tokens.size() != 3)
    {
      return false;
    }

    const std::string graphName = tokenAt(tokens, 1);
    const std::string vertex = tokenAt(tokens, 2);
    if (!sadovnik::isValidName(graphName) || !sadovnik::isValidName(vertex) ||
        !context.graphs().has(graphName))
    {
      return false;
    }

    const Graph & graph = context.graphs().get(graphName);
    if (!graph.hasVertex(vertex))
    {
      return false;
    }

    printAdjacent(out, graph.getOutbound(vertex));
    return true;
  }

  bool inboundCmd(CommandContext & context, const List< std::string > & tokens,
                  std::ostream & out)
  {
    if (tokens.size() != 3)
    {
      return false;
    }

    const std::string graphName = tokenAt(tokens, 1);
    const std::string vertex = tokenAt(tokens, 2);
    if (!sadovnik::isValidName(graphName) || !sadovnik::isValidName(vertex) ||
        !context.graphs().has(graphName))
    {
      return false;
    }

    const Graph & graph = context.graphs().get(graphName);
    if (!graph.hasVertex(vertex))
    {
      return false;
    }

    printAdjacent(out, graph.getInbound(vertex));
    return true;
  }

  bool bindCmd(CommandContext & context, const List< std::string > & tokens,
               std::ostream &)
  {
    if (tokens.size() != 5)
    {
      return false;
    }

    const std::string graphName = tokenAt(tokens, 1);
    const std::string from = tokenAt(tokens, 2);
    const std::string to = tokenAt(tokens, 3);
    Graph::Weight weight = 0;

    if (!sadovnik::isValidName(graphName) || !sadovnik::isValidName(from) ||
        !sadovnik::isValidName(to) || !parseWeight(tokenAt(tokens, 4), weight) ||
        !context.graphs().has(graphName))
    {
      return false;
    }

    context.graphs().get(graphName).bind(from, to, weight);
    return true;
  }

  bool cutCmd(CommandContext & context, const List< std::string > & tokens,
              std::ostream &)
  {
    if (tokens.size() != 5)
    {
      return false;
    }

    const std::string graphName = tokenAt(tokens, 1);
    const std::string from = tokenAt(tokens, 2);
    const std::string to = tokenAt(tokens, 3);
    Graph::Weight weight = 0;

    if (!sadovnik::isValidName(graphName) || !sadovnik::isValidName(from) ||
        !sadovnik::isValidName(to) || !parseWeight(tokenAt(tokens, 4), weight) ||
        !context.graphs().has(graphName))
    {
      return false;
    }

    return context.graphs().get(graphName).cut(from, to, weight);
  }

  bool createCmd(CommandContext & context, const List< std::string > & tokens,
                 std::ostream &)
  {
    if (tokens.size() < 2)
    {
      return false;
    }

    const std::string graphName = tokenAt(tokens, 1);
    if (!sadovnik::isValidName(graphName) || context.graphs().has(graphName))
    {
      return false;
    }

    Graph graph;
    if (tokens.size() > 2)
    {
      unsigned long long count = 0;
      if (!sadovnik::parseUnsigned(tokenAt(tokens, 2), count) ||
          tokens.size() != count + 3)
      {
        return false;
      }

      for (std::size_t i = 0; i < count; ++i)
      {
        const std::string vertex = tokenAt(tokens, i + 3);
        if (!sadovnik::isValidName(vertex))
        {
          return false;
        }
        graph.addVertex(vertex);
      }
    }

    ensureGraphSpace(context.graphs());
    context.graphs().add(graphName, graph);
    return true;
  }

  bool mergeCmd(CommandContext & context, const List< std::string > & tokens,
                std::ostream &)
  {
    if (tokens.size() != 4)
    {
      return false;
    }

    const std::string newName = tokenAt(tokens, 1);
    const std::string leftName = tokenAt(tokens, 2);
    const std::string rightName = tokenAt(tokens, 3);

    if (!sadovnik::isValidName(newName) || !sadovnik::isValidName(leftName) ||
        !sadovnik::isValidName(rightName) || context.graphs().has(newName) ||
        !context.graphs().has(leftName) || !context.graphs().has(rightName))
    {
      return false;
    }

    Graph graph = context.graphs().get(leftName).merge(context.graphs().get(rightName));
    ensureGraphSpace(context.graphs());
    context.graphs().add(newName, graph);
    return true;
  }

  bool extractCmd(CommandContext & context, const List< std::string > & tokens,
                  std::ostream &)
  {
    if (tokens.size() < 4)
    {
      return false;
    }

    const std::string newName = tokenAt(tokens, 1);
    const std::string oldName = tokenAt(tokens, 2);
    unsigned long long count = 0;

    if (!sadovnik::isValidName(newName) || !sadovnik::isValidName(oldName) ||
        context.graphs().has(newName) || !context.graphs().has(oldName) ||
        !sadovnik::parseUnsigned(tokenAt(tokens, 3), count) ||
        tokens.size() != count + 4)
    {
      return false;
    }

    const Graph & source = context.graphs().get(oldName);
    List< std::string > vertices;
    for (std::size_t i = 0; i < count; ++i)
    {
      const std::string vertex = tokenAt(tokens, i + 4);
      if (!sadovnik::isValidName(vertex) || !source.hasVertex(vertex))
      {
        return false;
      }
      vertices.pushBack(vertex);
    }

    Graph graph = source.extract(vertices);
    ensureGraphSpace(context.graphs());
    context.graphs().add(newName, graph);
    return true;
  }

}

namespace sadovnik
{

  CommandContext::CommandContext(GraphTab & graphs)
    : graphs_(graphs)
  {
  }

  GraphTab & CommandContext::graphs()
  {
    return graphs_;
  }

  const GraphTab & CommandContext::graphs() const
  {
    return graphs_;
  }

  CommandTab createCommandTable()
  {
    CommandTab commands(4, 4);
    commands.add("graphs", graphsCmd);
    commands.add("vertexes", vertexesCmd);
    commands.add("outbound", outboundCmd);
    commands.add("inbound", inboundCmd);
    commands.add("bind", bindCmd);
    commands.add("cut", cutCmd);
    commands.add("create", createCmd);
    commands.add("merge", mergeCmd);
    commands.add("extract", extractCmd);
    return commands;
  }

  bool executeCommandLine(CommandContext & context, const CommandTab & commands,
                          const std::string & line, std::ostream & out)
  {
    const List< std::string > tokens = splitTokens(line);
    if (tokens.empty())
    {
      return true;
    }

    const CommandHandler * handler = commands.find(tokenAt(tokens, 0));
    if (handler == nullptr)
    {
      out << "invalid\n";
      return false;
    }

    bool ok = false;
    try
    {
      ok = (*handler)(context, tokens, out);
    }
    catch (const std::exception &)
    {
      ok = false;
    }

    if (!ok)
    {
      out << "invalid\n";
    }

    return ok;
  }

  void processCommands(CommandContext & context, const CommandTab & commands,
                       std::istream & in, std::ostream & out)
  {
    std::string line;
    while (true)
    {
      std::getline(in, line);
      if (line.empty() && in.eof())
      {
        break;
      }
      if (in.fail() && line.empty())
      {
        break;
      }

      executeCommandLine(context, commands, line, out);

      if (in.eof())
      {
        break;
      }
    }
  }

}
