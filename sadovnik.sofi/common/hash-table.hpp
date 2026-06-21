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

    bool add(const Key & key, const Value & val)
    {
      if (findEntry(key) != nullptr)
      {
        throw std::logic_error("duplicate key");
      }

      Entry * cell = findFreeEntry(key);
      if (cell == nullptr)
      {
        throw std::overflow_error("hash table is full");
      }

      cell->key = key;
      cell->val = val;
      cell->used = true;
      ++elemCnt_;
      return true;
    }

    bool has(const Key & key) const
    {
      return findEntry(key) != nullptr;
    }

    Value * find(const Key & key)
    {
      Entry * entry = findEntry(key);
      if (entry == nullptr)
      {
        return nullptr;
      }
      return &entry->val;
    }

    const Value * find(const Key & key) const
    {
      const Entry * entry = findEntry(key);
      if (entry == nullptr)
      {
        return nullptr;
      }
      return &entry->val;
    }

    Value & get(const Key & key)
    {
      Entry * entry = findEntry(key);
      if (entry == nullptr)
      {
        throw std::out_of_range("missing key");
      }
      return entry->val;
    }

    const Value & get(const Key & key) const
    {
      const Entry * entry = findEntry(key);
      if (entry == nullptr)
      {
        throw std::out_of_range("missing key");
      }
      return entry->val;
    }

    bool set(const Key & key, const Value & val)
    {
      Entry * entry = findEntry(key);
      if (entry != nullptr)
      {
        entry->val = val;
        return false;
      }
      add(key, val);
      return true;
    }

    bool drop(const Key & key)
    {
      Entry * entry = findEntry(key);
      if (entry == nullptr)
      {
        return false;
      }
      entry->used = false;
      --elemCnt_;
      return true;
    }

  private:
    static const std::size_t HOME_POS = 0;

    std::size_t homeSlot(const Key & key) const
    {
      return hash_(key) % slotCnt_;
    }

    std::size_t flatIdx(std::size_t slot, std::size_t pos) const
    {
      return slot * (bktSize_ + detail::SPARE_POS) + pos;
    }

    std::size_t sparePos() const
    {
      return bktSize_;
    }

    Entry * findEntry(const Key & key) noexcept
    {
      return const_cast< Entry * >(static_cast< const HashTable * >(this)->findEntry(key));
    }

    const Entry * findEntry(const Key & key) const noexcept
    {
      if (slotCnt_ == 0)
      {
        return nullptr;
      }

      const std::size_t start = homeSlot(key);
      for (std::size_t offset = HOME_POS; offset < slotCnt_; ++offset)
      {
        const std::size_t slot = (start + offset) % slotCnt_;
        for (std::size_t pos = HOME_POS; pos <= sparePos(); ++pos)
        {
          const Entry & cell = cells_[flatIdx(slot, pos)];
          if (cell.used && equal_(cell.key, key))
          {
            return &cell;
          }
        }
      }
      return nullptr;
    }

    Entry * findFreeEntry(const Key & key) noexcept
    {
      const std::size_t start = homeSlot(key);
      for (std::size_t offset = HOME_POS; offset < slotCnt_; ++offset)
      {
        const std::size_t slot = (start + offset) % slotCnt_;

        for (std::size_t pos = HOME_POS; pos < bktSize_; ++pos)
        {
          Entry & cell = cells_[flatIdx(slot, pos)];
          if (!cell.used)
          {
            return &cell;
          }
        }

        Entry & spare = cells_[flatIdx(slot, sparePos())];
        if (!spare.used)
        {
          return &spare;
        }
      }
      return nullptr;
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
