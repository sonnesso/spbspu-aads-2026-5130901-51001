#ifndef CALCULATOR_HPP
#define CALCULATOR_HPP

#include <cstddef>
#include <string>

#include "queue.hpp"
#include "stack.hpp"

namespace sadovnik
{

  using Value = long long;

  class Calculator
  {
  public:
    Value evaluate(const std::string & line);

  private:
    struct Token
    {
      bool isNum = false;
      Value num = 0;
      char op = '\0';
    };

    void tokenize(const std::string & line, Queue< Token > & tokens);
    Value parseNum(const std::string & line, std::size_t & pos);
    void skipSpaces(const std::string & line, std::size_t & pos);
    bool isDigit(char ch) const;
    bool isOper(char ch) const;
    int prio(char op) const;
    Value apply(char op, Value lhs, Value rhs) const;
    void applyTop(Stack< Value > & vals, Stack< char > & oper) const;
  };

}

#endif
