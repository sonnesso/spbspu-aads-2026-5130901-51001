#ifndef STACK_HPP
#define STACK_HPP

#include <cstddef>
#include <stdexcept>
#include <utility>

#include <list.hpp>

namespace sadovnik
{

  template< class T >
  class Stack
  {
  public:
    Stack() = default;

    Stack(const Stack & other)
      : data_(other.data_)
    {
    }

    Stack(Stack && other) noexcept
      : data_(std::move(other.data_))
    {
    }

    Stack & operator=(const Stack & other)
    {
      if (this != &other)
      {
        Stack tmp(other);
        swap(tmp);
      }
      return *this;
    }

    Stack & operator=(Stack && other) noexcept
    {
      if (this != &other)
      {
        data_ = std::move(other.data_);
      }
      return *this;
    }

    void push(const T & value)
    {
      data_.pushBack(value);
    }

    void push(T && value)
    {
      data_.pushBack(std::move(value));
    }

    void drop(T & value)
    {
      if (empty())
      {
        throw std::logic_error("drop on empty stack");
      }
      value = data_.back();
      data_.popBack();
    }

    T & top()
    {
      if (empty())
      {
        throw std::logic_error("top on empty stack");
      }
      return data_.back();
    }

    const T & top() const
    {
      if (empty())
      {
        throw std::logic_error("top on empty stack");
      }
      return data_.back();
    }

    bool empty() const noexcept
    {
      return data_.empty();
    }

    std::size_t size() const noexcept
    {
      return data_.size();
    }

    void swap(Stack & other) noexcept
    {
      data_.swap(other.data_);
    }

  private:
    List< T > data_;
  };

  template< class T >
  void swap(Stack< T > & lhs, Stack< T > & rhs) noexcept
  {
    lhs.swap(rhs);
  }

}

#endif
