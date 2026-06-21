#ifndef HASH_TABLE_HPP
#define HASH_TABLE_HPP

#include <cstddef>
#include <stdexcept>
#include <utility>

namespace sadovnik
{

  namespace detail
  {
    const std::size_t DEFAULT_BKT_SIZE = 4;
    const std::size_t SPARE_POS = 1;
  }

  template< class Key, class Value, class Hash, class Equal >
  class HashTable
  {
  public:
    struct Entry
    {
      Key key;
      Value val;
      bool used;

      Entry()
        : key(),
          val(),
          used(false)
      {
      }
    };

    explicit HashTable(std::size_t slotCnt, std::size_t bktSize = detail::DEFAULT_BKT_SIZE)
      : cells_(nullptr),
        slotCnt_(slotCnt),
        bktSize_(bktSize),
        elemCnt_(0),
        hash_(),
        equal_()
    {
      if (slotCnt_ == 0 || bktSize_ == 0)
      {
        throw std::invalid_argument("invalid hash table capacity");
      }
      cells_ = new Entry[capacity()];
    }

    ~HashTable()
    {
      delete[] cells_;
    }

    void clear() noexcept
    {
      for (std::size_t i = 0; i < capacity(); ++i)
      {
        cells_[i].used = false;
      }
      elemCnt_ = 0;
    }

    bool empty() const noexcept
    {
      return elemCnt_ == 0;
    }

    std::size_t size() const noexcept
    {
      return elemCnt_;
    }

    std::size_t slots() const noexcept
    {
      return slotCnt_;
    }

    std::size_t bucketSize() const noexcept
    {
      return bktSize_;
    }

    std::size_t capacity() const noexcept
    {
      return slotCnt_ * (bktSize_ + detail::SPARE_POS);
    }

  private:
    std::size_t homeSlot(const Key & key) const
    {
      return hash_(key) % slotCnt_;
    }

    std::size_t flatIdx(std::size_t slot, std::size_t pos) const
    {
      return slot * (bktSize_ + detail::SPARE_POS) + pos;
    }

    Entry * cells_;
    std::size_t slotCnt_;
    std::size_t bktSize_;
    std::size_t elemCnt_;
    Hash hash_;
    Equal equal_;
  };

}

#endif
