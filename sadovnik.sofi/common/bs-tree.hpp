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

    template< class Key, class Value >
    BSTNode< Key, Value > * minNode(BSTNode< Key, Value > * node)
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

    template< class Key, class Value >
    const BSTNode< Key, Value > * minNode(const BSTNode< Key, Value > * node)
    {
      return minNode(const_cast< BSTNode< Key, Value > * >(node));
    }

    template< class Key, class Value >
    BSTNode< Key, Value > * maxNode(BSTNode< Key, Value > * node)
    {
      if (node == nullptr)
      {
        return nullptr;
      }

      while (node->right != nullptr)
      {
        node = node->right;
      }
      return node;
    }

    template< class Key, class Value >
    const BSTNode< Key, Value > * maxNode(const BSTNode< Key, Value > * node)
    {
      return maxNode(const_cast< BSTNode< Key, Value > * >(node));
    }

    template< class Key, class Value >
    BSTNode< Key, Value > * nextInOrder(BSTNode< Key, Value > * node)
    {
      if (node == nullptr)
      {
        return nullptr;
      }

      if (node->right != nullptr)
      {
        return minNode(node->right);
      }

      BSTNode< Key, Value > * parent = node->parent;
      while (parent != nullptr && node == parent->right)
      {
        node = parent;
        parent = parent->parent;
      }
      return parent;
    }

    template< class Key, class Value >
    const BSTNode< Key, Value > * nextInOrder(const BSTNode< Key, Value > * node)
    {
      return nextInOrder(const_cast< BSTNode< Key, Value > * >(node));
    }

    template< class Key, class Value >
    BSTNode< Key, Value > * prevInOrder(BSTNode< Key, Value > * node)
    {
      if (node == nullptr)
      {
        return nullptr;
      }

      if (node->left != nullptr)
      {
        return maxNode(node->left);
      }

      BSTNode< Key, Value > * parent = node->parent;
      while (parent != nullptr && node == parent->left)
      {
        node = parent;
        parent = parent->parent;
      }
      return parent;
    }

    template< class Key, class Value >
    const BSTNode< Key, Value > * prevInOrder(const BSTNode< Key, Value > * node)
    {
      return prevInOrder(const_cast< BSTNode< Key, Value > * >(node));
    }

    template< class Key, class Value >
    void fillEntry(const BSTNode< Key, Value > * node, std::pair< Key, Value > & slot)
    {
      if (node == nullptr)
      {
        throw std::logic_error("dereference end iterator");
      }

      slot.first = node->key;
      slot.second = node->val;
    }

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
      : node_(nullptr),
        root_(nullptr)
    {
    }

    reference operator*() const
    {
      detail::fillEntry(node_, slot_);
      return slot_;
    }

    pointer operator->() const
    {
      detail::fillEntry(node_, slot_);
      return &slot_;
    }

    BSTIterator & operator++()
    {
      node_ = detail::nextInOrder(node_);
      return *this;
    }

    BSTIterator operator++(int)
    {
      BSTIterator tmp = *this;
      ++(*this);
      return tmp;
    }

    BSTIterator & operator--()
    {
      if (node_ == nullptr)
      {
        node_ = detail::maxNode(root_);
      }
      else
      {
        node_ = detail::prevInOrder(node_);
      }
      return *this;
    }

    BSTIterator operator--(int)
    {
      BSTIterator tmp = *this;
      --(*this);
      return tmp;
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
    explicit BSTIterator(detail::BSTNode< Key, Value > * node,
                         detail::BSTNode< Key, Value > * root)
      : node_(node),
        root_(root),
        slot_()
    {
    }

    detail::BSTNode< Key, Value > * node_;
    detail::BSTNode< Key, Value > * root_;
    mutable value_type slot_;
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
      : node_(nullptr),
        root_(nullptr)
    {
    }

    BSTConstIterator(const BSTIterator< Key, Value > & it)
      : node_(it.node_),
        root_(it.root_),
        slot_()
    {
    }

    reference operator*() const
    {
      detail::fillEntry(node_, slot_);
      return slot_;
    }

    pointer operator->() const
    {
      detail::fillEntry(node_, slot_);
      return &slot_;
    }

    BSTConstIterator & operator++()
    {
      node_ = detail::nextInOrder(node_);
      return *this;
    }

    BSTConstIterator operator++(int)
    {
      BSTConstIterator tmp = *this;
      ++(*this);
      return tmp;
    }

    BSTConstIterator & operator--()
    {
      if (node_ == nullptr)
      {
        node_ = detail::maxNode(root_);
      }
      else
      {
        node_ = detail::prevInOrder(node_);
      }
      return *this;
    }

    BSTConstIterator operator--(int)
    {
      BSTConstIterator tmp = *this;
      --(*this);
      return tmp;
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
    explicit BSTConstIterator(const detail::BSTNode< Key, Value > * node,
                              const detail::BSTNode< Key, Value > * root)
      : node_(node),
        root_(root),
        slot_()
    {
    }

    const detail::BSTNode< Key, Value > * node_;
    const detail::BSTNode< Key, Value > * root_;
    mutable value_type slot_;
  };

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
      return iterator(detail::minNode(root_), root_);
    }

    iterator end()
    {
      return iterator(nullptr, root_);
    }

    const_iterator begin() const
    {
      return const_iterator(detail::minNode(root_), root_);
    }

    const_iterator end() const
    {
      return const_iterator(nullptr, root_);
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

    iterator find(const Key & key)
    {
      return iterator(findNode(key), root_);
    }

    const_iterator find(const Key & key) const
    {
      return const_iterator(findNode(key), root_);
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

    Value drop(const Key & key)
    {
      detail::BSTNode< Key, Value > * node = findNode(key);
      if (node == nullptr)
      {
        throw std::out_of_range("missing key");
      }

      const Value result = node->val;
      dropNode(node);
      --size_;
      return result;
    }

    const_iterator rotateLeft(const_iterator it)
    {
      detail::BSTNode< Key, Value > * rising =
        const_cast< detail::BSTNode< Key, Value > * >(it.node_);
      if (rising == nullptr || rising->parent == nullptr)
      {
        throw std::logic_error("cannot rotate left");
      }

      detail::BSTNode< Key, Value > * pivot = rising->parent;
      if (pivot->right != rising)
      {
        throw std::logic_error("cannot rotate left");
      }

      detail::BSTNode< Key, Value > * leftChild = rising->left;
      pivot->right = leftChild;
      if (leftChild != nullptr)
      {
        leftChild->parent = pivot;
      }

      rising->left = pivot;
      setChild(pivot->parent, pivot, rising);
      pivot->parent = rising;

      return const_iterator(rising, root_);
    }

    const_iterator rotateRight(const_iterator it)
    {
      detail::BSTNode< Key, Value > * rising =
        const_cast< detail::BSTNode< Key, Value > * >(it.node_);
      if (rising == nullptr || rising->parent == nullptr)
      {
        throw std::logic_error("cannot rotate right");
      }

      detail::BSTNode< Key, Value > * pivot = rising->parent;
      if (pivot->left != rising)
      {
        throw std::logic_error("cannot rotate right");
      }

      detail::BSTNode< Key, Value > * rightChild = rising->right;
      pivot->left = rightChild;
      if (rightChild != nullptr)
      {
        rightChild->parent = pivot;
      }

      rising->right = pivot;
      setChild(pivot->parent, pivot, rising);
      pivot->parent = rising;

      return const_iterator(rising, root_);
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

    void setChild(detail::BSTNode< Key, Value > * parent,
                  detail::BSTNode< Key, Value > * oldNode,
                  detail::BSTNode< Key, Value > * newNode)
    {
      if (parent == nullptr)
      {
        root_ = newNode;
      }
      else if (parent->left == oldNode)
      {
        parent->left = newNode;
      }
      else
      {
        parent->right = newNode;
      }

      if (newNode != nullptr)
      {
        newNode->parent = parent;
      }
    }

    void dropNode(detail::BSTNode< Key, Value > * node)
    {
      if (node->left != nullptr && node->right != nullptr)
      {
        detail::BSTNode< Key, Value > * succ = detail::minNode(node->right);
        node->key = succ->key;
        node->val = succ->val;

        detail::BSTNode< Key, Value > * succParent = succ->parent;
        detail::BSTNode< Key, Value > * succRight = succ->right;
        setChild(succParent, succ, succRight);
        delete succ;
        return;
      }

      detail::BSTNode< Key, Value > * child =
        node->left != nullptr ? node->left : node->right;
      setChild(node->parent, node, child);
      delete node;
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
