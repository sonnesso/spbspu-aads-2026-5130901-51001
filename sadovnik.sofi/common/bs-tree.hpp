#ifndef BS_TREE_HPP
#define BS_TREE_HPP

#include <cstddef>
#include <iterator>
#include <stdexcept>
#include <utility>

namespace sadovnik
{

  template< class Key, class Value, class Compare >
  class BSTree;

  namespace detail
  {

    template< class Key, class Value >
    struct BSTNode;

  }

  template< class Key, class Value >
  class BSTConstIterator;

  template< class Key, class Value >
  class BSTIterator
  {
    template< class K, class V, class C >
    friend class BSTree;

    friend class BSTConstIterator< Key, Value >;

  public:
    using iterator_category = std::bidirectional_iterator_tag;
    using value_type = std::pair< Key, Value >;
    using difference_type = std::ptrdiff_t;
    using pointer = value_type *;
    using reference = value_type &;

    BSTIterator()
      : node_(nullptr)
    {
    }

    bool operator==(const BSTIterator & other) const
    {
      return node_ == other.node_;
    }

    bool operator!=(const BSTIterator & other) const
    {
      return !(*this == other);
    }

  private:
    explicit BSTIterator(detail::BSTNode< Key, Value > * node)
      : node_(node)
    {
    }

    detail::BSTNode< Key, Value > * node_;
  };

  template< class Key, class Value >
  class BSTConstIterator
  {
    template< class K, class V, class C >
    friend class BSTree;

    friend class BSTIterator< Key, Value >;

  public:
    using iterator_category = std::bidirectional_iterator_tag;
    using value_type = std::pair< Key, Value >;
    using difference_type = std::ptrdiff_t;
    using pointer = const value_type *;
    using reference = const value_type &;

    BSTConstIterator()
      : node_(nullptr)
    {
    }

    BSTConstIterator(const BSTIterator< Key, Value > & it)
      : node_(it.node_)
    {
    }

    bool operator==(const BSTConstIterator & other) const
    {
      return node_ == other.node_;
    }

    bool operator!=(const BSTConstIterator & other) const
    {
      return !(*this == other);
    }

  private:
    explicit BSTConstIterator(const detail::BSTNode< Key, Value > * node)
      : node_(node)
    {
    }

    const detail::BSTNode< Key, Value > * node_;
  };

  namespace detail
  {

    template< class Key, class Value >
    struct BSTNode
    {
      Key key;
      Value val;
      BSTNode * left;
      BSTNode * right;
      BSTNode * parent;

      BSTNode(const Key & keyVal, const Value & valueVal)
        : key(keyVal),
          val(valueVal),
          left(nullptr),
          right(nullptr),
          parent(nullptr)
      {
      }
    };

  }

  template< class Key, class Value, class Compare >
  class BSTree
  {
  public:
    using iterator = BSTIterator< Key, Value >;
    using const_iterator = BSTConstIterator< Key, Value >;

    BSTree()
      : root_(nullptr),
        size_(0),
        cmp_()
    {
    }

    ~BSTree()
    {
      clear();
    }

    bool empty() const noexcept
    {
      return size_ == 0;
    }

    std::size_t size() const noexcept
    {
      return size_;
    }

    void clear() noexcept
    {
      destroySub(root_);
      root_ = nullptr;
      size_ = 0;
    }

    iterator begin()
    {
      return iterator(minNode(root_));
    }

    iterator end()
    {
      return iterator(nullptr);
    }

    const_iterator begin() const
    {
      return const_iterator(minNode(root_));
    }

    const_iterator end() const
    {
      return const_iterator(nullptr);
    }

    const_iterator cbegin() const
    {
      return begin();
    }

    const_iterator cend() const
    {
      return end();
    }

    std::size_t height() const
    {
      return heightSub(root_);
    }

    std::size_t height(const_iterator it) const
    {
      if (it.node_ == nullptr)
      {
        return 0;
      }
      return heightSub(it.node_);
    }

    void push(const Key & key, const Value & val)
    {
      if (root_ == nullptr)
      {
        root_ = new detail::BSTNode< Key, Value >(key, val);
        ++size_;
        return;
      }

      detail::BSTNode< Key, Value > * cur = root_;
      while (cur != nullptr)
      {
        if (keysEqual(key, cur->key))
        {
          throw std::logic_error("duplicate key");
        }

        if (cmp_(key, cur->key))
        {
          if (cur->left == nullptr)
          {
            cur->left = new detail::BSTNode< Key, Value >(key, val);
            cur->left->parent = cur;
            ++size_;
            return;
          }
          cur = cur->left;
        }
        else
        {
          if (cur->right == nullptr)
          {
            cur->right = new detail::BSTNode< Key, Value >(key, val);
            cur->right->parent = cur;
            ++size_;
            return;
          }
          cur = cur->right;
        }
      }
    }

    bool has(const Key & key) const noexcept
    {
      return findNode(key) != nullptr;
    }

    Value get(const Key & key) const
    {
      const detail::BSTNode< Key, Value > * node = findNode(key);
      if (node == nullptr)
      {
        throw std::out_of_range("missing key");
      }
      return node->val;
    }

  private:
    bool keysEqual(const Key & lhs, const Key & rhs) const
    {
      return !cmp_(lhs, rhs) && !cmp_(rhs, lhs);
    }

    detail::BSTNode< Key, Value > * findNode(const Key & key) noexcept
    {
      return const_cast< detail::BSTNode< Key, Value > * >(
        static_cast< const BSTree * >(this)->findNode(key));
    }

    const detail::BSTNode< Key, Value > * findNode(const Key & key) const noexcept
    {
      const detail::BSTNode< Key, Value > * cur = root_;
      while (cur != nullptr)
      {
        if (keysEqual(key, cur->key))
        {
          return cur;
        }

        if (cmp_(key, cur->key))
        {
          cur = cur->left;
        }
        else
        {
          cur = cur->right;
        }
      }
      return nullptr;
    }

    static detail::BSTNode< Key, Value > * minNode(detail::BSTNode< Key, Value > * node)
    {
      if (node == nullptr)
      {
        return nullptr;
      }

      while (node->left != nullptr)
      {
        node = node->left;
      }
      return node;
    }

    static void destroySub(detail::BSTNode< Key, Value > * node)
    {
      if (node == nullptr)
      {
        return;
      }

      destroySub(node->left);
      destroySub(node->right);
      delete node;
    }

    static std::size_t heightSub(const detail::BSTNode< Key, Value > * node)
    {
      if (node == nullptr)
      {
        return 0;
      }

      const std::size_t leftH = heightSub(node->left);
      const std::size_t rightH = heightSub(node->right);
      const std::size_t maxH = leftH > rightH ? leftH : rightH;
      return maxH + 1;
    }

    detail::BSTNode< Key, Value > * root_;
    std::size_t size_;
    Compare cmp_;
  };

}

#endif
