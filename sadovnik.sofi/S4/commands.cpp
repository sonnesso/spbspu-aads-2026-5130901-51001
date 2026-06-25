#include "commands.hpp"

#include <bs-tree-ops.hpp>
#include <string-utils.hpp>

#include <istream>
#include <sstream>
#include <string>

namespace
{

  using sadovnik::CommandContext;
  using sadovnik::IntStrTree;
  using sadovnik::List;

  const char INVALID_COMMAND[] = "<INVALID COMMAND>";
  const char EMPTY_DATASET[] = "<EMPTY>";

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

  void ensureDatasetSpace(sadovnik::DatasetTab & datasets)
  {
    if (datasets.size() == datasets.capacity())
    {
      datasets.rehash(datasets.slots() * 2);
    }
  }

  bool isValidDatasetName(const std::string & name)
  {
    return sadovnik::isValidName(name);
  }

  bool hasDataset(const sadovnik::DatasetTab & datasets, const std::string & name)
  {
    return isValidDatasetName(name) && datasets.has(name);
  }

  void printDataset(std::ostream & out, const std::string & name, const IntStrTree & tree)
  {
    if (tree.empty())
    {
      out << EMPTY_DATASET << '\n';
      return;
    }

    out << name;
    for (auto it = tree.cbegin(); it != tree.cend(); ++it)
    {
      out << ' ' << it->first << ' ' << it->second;
    }
    out << '\n';
  }

  bool printCmd(CommandContext & context, const List< std::string > & tokens,
                std::ostream & out)
  {
    if (tokens.size() != 2)
    {
      return false;
    }

    const std::string datasetName = tokenAt(tokens, 1);
    if (!hasDataset(context.datasets(), datasetName))
    {
      return false;
    }

    printDataset(out, datasetName, context.datasets().get(datasetName));
    return true;
  }

  bool complementCmd(CommandContext & context, const List< std::string > & tokens,
                     std::ostream &)
  {
    if (tokens.size() != 4)
    {
      return false;
    }

    const std::string newName = tokenAt(tokens, 1);
    const std::string leftName = tokenAt(tokens, 2);
    const std::string rightName = tokenAt(tokens, 3);

    if (!isValidDatasetName(newName) || context.datasets().has(newName) ||
        !hasDataset(context.datasets(), leftName) ||
        !hasDataset(context.datasets(), rightName))
    {
      return false;
    }

    const IntStrTree result = sadovnik::complement(
      context.datasets().get(leftName), context.datasets().get(rightName));

    ensureDatasetSpace(context.datasets());
    context.datasets().add(newName, result);
    return true;
  }

  bool intersectCmd(CommandContext & context, const List< std::string > & tokens,
                    std::ostream &)
  {
    if (tokens.size() != 4)
    {
      return false;
    }

    const std::string newName = tokenAt(tokens, 1);
    const std::string leftName = tokenAt(tokens, 2);
    const std::string rightName = tokenAt(tokens, 3);

    if (!isValidDatasetName(newName) || context.datasets().has(newName) ||
        !hasDataset(context.datasets(), leftName) ||
        !hasDataset(context.datasets(), rightName))
    {
      return false;
    }

    const IntStrTree result = sadovnik::intersect(
      context.datasets().get(leftName), context.datasets().get(rightName));

    ensureDatasetSpace(context.datasets());
    context.datasets().add(newName, result);
    return true;
  }

  bool unionCmd(CommandContext & context, const List< std::string > & tokens,
                std::ostream &)
  {
    if (tokens.size() != 4)
    {
      return false;
    }

    const std::string newName = tokenAt(tokens, 1);
    const std::string leftName = tokenAt(tokens, 2);
    const std::string rightName = tokenAt(tokens, 3);

    if (!isValidDatasetName(newName) || context.datasets().has(newName) ||
        !hasDataset(context.datasets(), leftName) ||
        !hasDataset(context.datasets(), rightName))
    {
      return false;
    }

    const IntStrTree result = sadovnik::unite(
      context.datasets().get(leftName), context.datasets().get(rightName));

    ensureDatasetSpace(context.datasets());
    context.datasets().add(newName, result);
    return true;
  }

}

namespace sadovnik
{

  CommandContext::CommandContext(DatasetTab & datasets)
    : datasets_(datasets)
  {
  }

  DatasetTab & CommandContext::datasets()
  {
    return datasets_;
  }

  const DatasetTab & CommandContext::datasets() const
  {
    return datasets_;
  }

  CommandTab createCommandTable()
  {
    CommandTab commands(4, 4);
    commands.add("print", printCmd);
    commands.add("complement", complementCmd);
    commands.add("intersect", intersectCmd);
    commands.add("union", unionCmd);
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
      out << INVALID_COMMAND << '\n';
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
      out << INVALID_COMMAND << '\n';
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
