#ifndef QUEUE_HPP
#define QUEUE_HPP

#include <cstddef>
#include <stdexcept>
#include <utility>

#include <list.hpp>

namespace sadovnik
{

  template< class T >
  class Queue
  {
  public:
    Queue() = default;

    Queue(const Queue & other)
      : data_(other.data_)
    {
    }

    Queue(Queue && other) noexcept
      : data_(std::move(other.data_))
    {
    }

    Queue & operator=(const Queue & other)
    {
      if (this != &other)
      {
        Queue tmp(other);
        swap(tmp);
      }
      return *this;
    }

    Queue & operator=(Queue && other) noexcept
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
        throw std::logic_error("drop on empty queue");
      }
      value = data_.front();
      data_.popFront();
    }

    T & front()
    {
      if (empty())
      {
        throw std::logic_error("front on empty queue");
      }
      return data_.front();
    }

    const T & front() const
    {
      if (empty())
      {
        throw std::logic_error("front on empty queue");
      }
      return data_.front();
    }

    bool empty() const noexcept
    {
      return data_.empty();
    }

    std::size_t size() const noexcept
    {
      return data_.size();
    }

    void swap(Queue & other) noexcept
    {
      data_.swap(other.data_);
    }

  private:
    List< T > data_;
  };

  template< class T >
  void swap(Queue< T > & lhs, Queue< T > & rhs) noexcept
  {
    lhs.swap(rhs);
  }

}

#endif
