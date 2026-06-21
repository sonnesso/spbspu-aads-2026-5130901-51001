#include "calculator.hpp"

#include <limits>
#include <stdexcept>

namespace sadovnik
{

  namespace
  {

    constexpr Value kMaxVal = std::numeric_limits< Value >::max();
    constexpr Value kMinVal = std::numeric_limits< Value >::min();

    Value absVal(Value val)
    {
      if (val == kMinVal)
      {
        throw std::overflow_error("integer overflow");
      }
      return val < 0 ? -val : val;
    }

    bool mulOverflow(Value lhs, Value rhs)
    {
      if (lhs > 0 && rhs > 0)
      {
        return lhs > kMaxVal / rhs;
      }
      if (lhs < 0 && rhs < 0)
      {
        return lhs < kMaxVal / rhs;
      }
      if (lhs > 0 && rhs < 0)
      {
        return rhs < kMinVal / lhs;
      }
      if (lhs < 0 && rhs > 0)
      {
        return lhs < kMinVal / rhs;
      }
      return false;
    }

  }

  void Calculator::skipSpaces(const std::string & line, std::size_t & pos)
  {
    while (pos < line.size() && line[pos] == ' ')
    {
      ++pos;
    }
  }

  bool Calculator::isDigit(char ch) const
  {
    return ch >= '0' && ch <= '9';
  }

  bool Calculator::isOper(char ch) const
  {
    return ch == '+' || ch == '-' || ch == '*' || ch == '/'
      || ch == '%' || ch == '|';
  }

  int Calculator::prio(char op) const
  {
    if (op == '|')
    {
      return 1;
    }
    if (op == '+' || op == '-')
    {
      return 2;
    }
    if (op == '*' || op == '/' || op == '%')
    {
      return 3;
    }
    return 0;
  }

  Value Calculator::parseNum(const std::string & line, std::size_t & pos)
  {
    Value val = 0;
    while (pos < line.size() && isDigit(line[pos]))
    {
      if (val > kMaxVal / 10)
      {
        throw std::overflow_error("integer overflow");
      }
      val = (val * 10) + (line[pos] - '0');
      ++pos;
    }
    return val;
  }

  void Calculator::tokenize(const std::string & line, Queue< Token > & tokens)
  {
    std::size_t pos = 0;
    while (pos < line.size())
    {
      skipSpaces(line, pos);
      if (pos >= line.size())
      {
        break;
      }

      if (isDigit(line[pos]))
      {
        Token tok;
        tok.isNum = true;
        tok.num = parseNum(line, pos);
        tokens.push(tok);
      }
      else if (isOper(line[pos]) || line[pos] == '(' || line[pos] == ')')
      {
        Token tok;
        tok.isNum = false;
        tok.op = line[pos];
        tokens.push(tok);
        ++pos;
      }
      else
      {
        throw std::logic_error("invalid token");
      }
    }
  }

  Value Calculator::apply(char op, Value lhs, Value rhs) const
  {
    switch (op)
    {
      case '+':
        if ((rhs > 0 && lhs > kMaxVal - rhs)
          || (rhs < 0 && lhs < kMinVal - rhs))
        {
          throw std::overflow_error("integer overflow");
        }
        return lhs + rhs;

      case '-':
        if ((rhs > 0 && lhs < kMinVal + rhs)
          || (rhs < 0 && lhs > kMaxVal + rhs))
        {
          throw std::overflow_error("integer overflow");
        }
        return lhs - rhs;

      case '*':
        if (lhs == 0 || rhs == 0)
        {
          return 0;
        }
        if ((lhs == kMinVal && rhs == -1) || (rhs == kMinVal && lhs == -1))
        {
          throw std::overflow_error("integer overflow");
        }
        if (mulOverflow(lhs, rhs))
        {
          throw std::overflow_error("integer overflow");
        }
        return lhs * rhs;

      case '/':
        if (rhs == 0)
        {
          throw std::logic_error("division by zero");
        }
        if (lhs == kMinVal && rhs == -1)
        {
          throw std::overflow_error("integer overflow");
        }
        return lhs / rhs;

      case '%':
        if (rhs == 0)
        {
          throw std::logic_error("modulo by zero");
        }
        if (lhs == kMinVal && rhs == -1)
        {
          throw std::overflow_error("integer overflow");
        }
        {
          Value rem = lhs % rhs;
          if (rem < 0)
          {
            rem += absVal(rhs);
          }
          return rem;
        }

      case '|':
        return lhs | rhs;

      default:
        throw std::logic_error("unknown operator");
    }
  }

  void Calculator::applyTop(Stack< Value > & vals, Stack< char > & oper) const
  {
    if (vals.size() < 2)
    {
      throw std::logic_error("invalid expression");
    }

    char op = '\0';
    Value rhs = 0;
    Value lhs = 0;
    oper.drop(op);
    vals.drop(rhs);
    vals.drop(lhs);
    vals.push(apply(op, lhs, rhs));
  }

  Value Calculator::evaluate(const std::string & line)
  {
    Queue< Token > tokens;
    tokenize(line, tokens);

    Stack< Value > vals;
    Stack< char > oper;
    bool needVal = true;

    while (!tokens.empty())
    {
      Token tok;
      tokens.drop(tok);

      if (tok.isNum)
      {
        if (!needVal)
        {
          throw std::logic_error("invalid expression");
        }
        vals.push(tok.num);
        needVal = false;
      }
      else if (tok.op == '(')
      {
        if (!needVal)
        {
          throw std::logic_error("invalid expression");
        }
        oper.push('(');
        needVal = true;
      }
      else if (tok.op == ')')
      {
        if (needVal)
        {
          throw std::logic_error("invalid expression");
        }
        while (!oper.empty() && oper.top() != '(')
        {
          applyTop(vals, oper);
        }
        if (oper.empty())
        {
          throw std::logic_error("mismatched parentheses");
        }
        char open = '\0';
        oper.drop(open);
        needVal = false;
      }
      else if (isOper(tok.op))
      {
        if (needVal)
        {
          throw std::logic_error("invalid expression");
        }
        while (!oper.empty() && oper.top() != '('
          && prio(oper.top()) >= prio(tok.op))
        {
          applyTop(vals, oper);
        }
        oper.push(tok.op);
        needVal = true;
      }
    }

    if (needVal)
    {
      throw std::logic_error("invalid expression");
    }

    while (!oper.empty())
    {
      applyTop(vals, oper);
    }

    if (vals.size() != 1)
    {
      throw std::logic_error("invalid expression");
    }

    Value ans = 0;
    vals.drop(ans);
    return ans;
  }

}
