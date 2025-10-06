// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#ifndef ILIST_BASE_20240923_HPP
#define ILIST_BASE_20240923_HPP

#include <cstdint>
#include <iterator>
#include <type_traits>
#include <utility>

namespace tinytc {

template <typename NodeT, bool IsConst> class ilist_iterator;

template <typename NodeT> class ilist_node {
  public:
    using node_type = NodeT;

    auto prev() const -> ilist_node<NodeT> * { return prev_; }
    void prev(ilist_node<NodeT> *prev) { prev_ = prev; }
    auto next() const -> ilist_node<NodeT> * { return next_; }
    void next(ilist_node<NodeT> *next) { next_ = next; }

    auto sentinel() const -> bool { return sentinel_; }
    void set_sentinel() { sentinel_ = true; }

    auto iterator() -> ilist_iterator<ilist_node<NodeT>, false> { return {this}; }

  private:
    ilist_node<NodeT> *prev_ = nullptr, *next_ = nullptr;
    bool sentinel_ = false;
};

template <typename NodeT, typename ParentT>
class ilist_node_with_parent : public ilist_node<NodeT> {
  public:
    auto parent() const -> ParentT * { return parent_; }
    void parent(ParentT *parent) { parent_ = parent; }

  private:
    ParentT *parent_ = nullptr;
};

template <typename IListNodeT, bool IsConst> class ilist_iterator {
  public:
    using base_type = std::conditional_t<IsConst, const IListNodeT, IListNodeT>;
    using base_pointer = base_type *;
    using node_type = typename IListNodeT::node_type;
    using value_type = std::conditional_t<IsConst, const node_type, node_type>;
    using pointer = value_type *;
    using reference = value_type &;
    using difference_type = std::ptrdiff_t;

    ilist_iterator() : pos_{nullptr} {}
    ilist_iterator(base_pointer pos) : pos_{std::move(pos)} {}

    auto operator*() const -> reference { return *static_cast<pointer>(pos_); }
    auto operator->() const -> pointer { return get(); }
    auto get() const -> pointer { return static_cast<pointer>(pos_); }
    auto get_base() const -> base_pointer { return pos_; }
    auto &operator++() {
        pos_ = pos_->next();
        return *this;
    }
    auto operator++(int) {
        auto old_pos = pos_;
        pos_ = pos_->next();
        return ilist_iterator{old_pos};
    }
    auto &operator--() {
        pos_ = pos_->prev();
        return *this;
    }
    auto operator--(int) {
        auto old_pos = pos_;
        pos_ = pos_->prev();
        return ilist_iterator{old_pos};
    }
    auto operator==(ilist_iterator const &other) const -> bool { return pos_ == other.pos_; }
    auto operator!=(ilist_iterator const &other) const -> bool { return pos_ != other.pos_; }

  private:
    base_pointer pos_;
};

enum class ilist_clear_order { forward, reverse };

template <typename NodeT, typename IListCallback,
          ilist_clear_order ClearOrder = ilist_clear_order::reverse>
requires requires(IListCallback &cb, NodeT *node) {
    std::is_base_of_v<ilist_node<NodeT>, NodeT>;
    cb.node_added(node);
    cb.node_removed(node);
}
class ilist_base : protected IListCallback {
  public:
    using base_type = ilist_node<NodeT>;
    using base_pointer = base_type *;
    using value_type = NodeT;
    using size_type = std::size_t;
    using difference_type = std::ptrdiff_t;
    using pointer = value_type *;
    using reference = value_type &;
    using const_reference = const value_type &;
    using iterator = ilist_iterator<base_type, false>;
    using const_iterator = ilist_iterator<base_type, true>;
    static_assert(std::bidirectional_iterator<iterator>);

    ilist_base() {
        sentinel_.set_sentinel();
        // let's go in a circle - yay!
        sentinel_.prev(&sentinel_);
        sentinel_.next(&sentinel_);
    }
    ~ilist_base() { clear(); }

    ilist_base(ilist_base const &other) = delete;
    ilist_base &operator=(ilist_base const &other) = delete;

    ilist_base(ilist_base &&other) {
        sentinel_.set_sentinel();
        sentinel_.prev(&sentinel_);
        sentinel_.next(&sentinel_);

        swap(other);
    }
    ilist_base &operator=(ilist_base &&other) {
        swap(other);
        return *this;
    }

    auto begin() -> iterator { return ++iterator{&sentinel_}; }
    auto begin() const -> const_iterator { return cbegin(); }
    auto cbegin() const -> const_iterator { return ++const_iterator{&sentinel_}; }
    auto end() -> iterator { return iterator{&sentinel_}; }
    auto end() const -> const_iterator { return cend(); }
    auto cend() const -> const_iterator { return const_iterator{&sentinel_}; }

    auto empty() const -> bool {
        return sentinel_.prev() == &sentinel_ && sentinel_.next() == &sentinel_;
    }
    auto size() const -> std::size_t {
        std::size_t s = 0;
        for (auto it = begin(); it != end(); ++it) {
            ++s;
        }
        return s;
    }

    void push_front(pointer node) { insert(begin(), node); }
    void push_back(pointer node) { insert(end(), node); }
    void pop_front() { erase(begin()); }
    void pop_back() { erase(--end()); }
    void clear() {
        if constexpr (ClearOrder == ilist_clear_order::reverse) {
            auto it = end();
            while (it != begin()) {
                it = erase(--it);
            }
        } else {
            erase(begin(), end());
        }
    }

    auto insert(iterator it, pointer node) -> iterator {
        // let s = sentinel
        // |0|: s{prev->s,next->s}
        // |1|: n0{prev->s,next->s}, s{prev->n0,next->n0}
        base_pointer prev = it.get_base()->prev();
        prev->next(node);
        node->prev(prev);
        node->next(it.get());
        it->prev(node);
        // |0| (it -> s) : node{prev->s,next->s}, s{prev->n0,next->n0}
        // |1| (it -> n0): node{prev->s,next->n0}, n0{prev->node,next->s}, s{prev->n0,next->node}
        // |1| (it -> s) : n0{prev->s,next->node}, node{prev->n0,next->s}, s{prev->node,next->n0}
        this->node_added(node);
        return iterator{node};
    }
    template <typename ItT> auto insert(iterator it, ItT begin, ItT end) -> iterator {
        if (begin != end) {
            it = insert(it, *begin++);
            auto first_it = it;
            for (; begin != end; ++begin) {
                it = insert(it, *begin);
                ++it; // skip over just inserted value
            }
            return first_it;
        }
        return it;
    }
    auto insert_after(iterator it, pointer node) -> iterator { return insert(++it, node); }

    auto unlink(iterator it) -> iterator {
        // let s = sentinel
        // |1|: n0{prev->s,next->s}, s{prev->n0,next->n0}
        // |2|: n0{prev->s,next->n1}, n1{prev->n0,next->s}, s{prev->n1,next->n0}
        base_pointer prev = it.get_base()->prev();
        base_pointer next = it.get_base()->next();
        prev->next(next);
        next->prev(prev);
        it.get_base()->prev(nullptr);
        it.get_base()->next(nullptr);
        // |1| (it -> n0): s{prev->s,next->s}
        // |2| (it -> n0): n1{prev->s,next->s}, s{prev->n1,next->n1}
        // |2| (it -> n1): n0{prev->s,next->s}, s{prev->n0,next->n0}
        return iterator{next};
    }
    auto unlink(iterator begin, iterator end) -> iterator {
        while (begin != end) {
            begin = unlínk(begin);
        }
        return begin;
    }

    auto erase(iterator it) -> iterator {
        auto next = unlink(it);
        this->node_removed(&*it);
        return next;
    }
    auto erase(iterator begin, iterator end) -> iterator {
        while (begin != end) {
            begin = erase(begin);
        }
        return begin;
    }

  private:
    void swap(ilist_base &o) {
        auto o_prev = sentinel_.prev() != &sentinel_ ? sentinel_.prev() : &o.sentinel_;
        auto o_next = sentinel_.next() != &sentinel_ ? sentinel_.next() : &o.sentinel_;
        sentinel_.prev(o.sentinel_.prev() != &o.sentinel_ ? o.sentinel_.prev() : &sentinel_);
        sentinel_.next(o.sentinel_.next() != &o.sentinel_ ? o.sentinel_.next() : &sentinel_);
        sentinel_.next()->prev(&sentinel_);
        sentinel_.prev()->next(&sentinel_);
        o.sentinel_.prev(o_prev);
        o.sentinel_.next(o_next);
        o.sentinel_.next()->prev(&o.sentinel_);
        o.sentinel_.prev()->next(&o.sentinel_);

        for (auto &node : *this) {
            this->node_moved(&node);
        }
        for (auto &node : o) {
            this->node_moved(&node);
        }
    }

    base_type sentinel_;
};

} // namespace tinytc

#endif // ILIST_BASE_20240923_HPP
