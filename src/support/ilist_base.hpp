// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#ifndef ILIST_BASE_20240923_HPP
#define ILIST_BASE_20240923_HPP

#include <cstdint>
#include <iterator>
#include <type_traits>
#include <utility>

namespace tinytc {

template <typename T> class ilist_node {
  public:
    auto prev() const -> T * { return prev_; }
    void prev(T *prev) { prev_ = prev; }
    auto next() const -> T * { return next_; }
    void next(T *next) { next_ = next; }

    auto sentinel() const -> bool { return sentinel_; }
    void set_sentinel() { sentinel_ = true; }

  private:
    T *prev_ = nullptr, *next_ = nullptr;
    bool sentinel_ = false;
};

template <typename NodeT, bool IsConst> class ilist_iterator {
  public:
    using base_type = std::conditional_t<IsConst, const ilist_node<NodeT>, ilist_node<NodeT>>;
    using base_pointer = base_type *;
    using value_type = std::conditional_t<IsConst, const NodeT, NodeT>;
    using pointer = value_type *;
    using reference = value_type &;
    using difference_type = std::ptrdiff_t;

    ilist_iterator() : pos_{nullptr} {}
    ilist_iterator(base_pointer pos) : pos_{std::move(pos)} {}

    auto operator*() const -> reference { return *static_cast<pointer>(pos_); }
    auto operator->() const -> pointer { return get(); }
    auto get() const -> pointer { return static_cast<pointer>(pos_); }
    auto &operator++() {
        pos_ = static_cast<base_pointer>(pos_->next());
        return *this;
    }
    auto operator++(int) {
        auto old_pos = pos_;
        pos_ = static_cast<base_pointer>(pos_->next());
        return ilist_iterator{old_pos};
    }
    auto &operator--() {
        pos_ = static_cast<base_pointer>(pos_->prev());
        return *this;
    }
    auto operator--(int) {
        auto old_pos = pos_;
        pos_ = static_cast<base_pointer>(pos_->prev());
        return ilist_iterator{old_pos};
    }
    auto operator==(ilist_iterator const &other) const -> bool { return pos_ == other.pos_; }
    auto operator!=(ilist_iterator const &other) const -> bool { return pos_ != other.pos_; }

  private:
    base_pointer pos_;
};

template <typename NodeT> struct ilist_dummy_callback {
    static void on_insert(NodeT *) {}
    static void on_erase(NodeT *) {}
};

template <typename NodeT, typename IListCallback = ilist_dummy_callback<NodeT>>
requires requires(NodeT *node) {
    std::is_base_of_v<ilist_node<NodeT>, NodeT>;
    IListCallback::on_insert(node);
    IListCallback::on_erase(node);
}
class ilist_base {
  public:
    using value_type = NodeT;
    using size_type = std::size_t;
    using difference_type = std::ptrdiff_t;
    using pointer = value_type *;
    using reference = value_type &;
    using const_reference = const value_type &;
    using iterator = ilist_iterator<value_type, false>;
    using const_iterator = ilist_iterator<value_type, true>;
    static_assert(std::bidirectional_iterator<iterator>);

    ilist_base() {
        sentinel_.set_sentinel();
        // let's go in a circle - yay!
        sentinel_.prev(static_cast<NodeT *>(&sentinel_));
        sentinel_.next(static_cast<NodeT *>(&sentinel_));
    }
    ~ilist_base() { clear(); }

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
    void clear() { erase(begin(), end()); }

    auto insert(iterator it, pointer node) -> iterator {
        // let s = sentinel
        // |0|: s{prev->s,next->s}
        // |1|: n0{prev->s,next->s}, s{prev->n0,next->n0}
        pointer prev = it->prev();
        prev->next(node);
        node->prev(prev);
        node->next(it.get());
        it->prev(node);
        // |0| (it -> s) : node{prev->s,next->s}, s{prev->n0,next->n0}
        // |1| (it -> n0): node{prev->s,next->n0}, n0{prev->node,next->s}, s{prev->n0,next->node}
        // |1| (it -> s) : n0{prev->s,next->node}, node{prev->n0,next->s}, s{prev->node,next->n0}
        IListCallback::on_insert(node);
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

    auto erase(iterator it) -> iterator {
        // let s = sentinel
        // |0|: s{prev->s,next->s}
        // |1|: n0{prev->s,next->s}, s{prev->n0,next->n0}
        // |2|: n0{prev->s,next->n1}, n1{prev->n0,next->s}, s{prev->n1,next->n0}
        pointer prev = it->prev();
        pointer next = it->prev();
        prev->prev(next);
        next->prev(prev);
        it->prev(nullptr);
        it->next(nullptr);
        // |0| (it -> s) : s{prev->s,next->s}
        // |1| (it -> n0): s{prev->s,next->s}
        // |2| (it -> n0): n1{prev->s,next->s}, s{prev->n1,next->n1}
        // |2| (it -> n1): n0{prev->s,next->s}, s{prev->n0,next->n0}
        IListCallback::on_erase(it.get());
        return iterator{next};
    }
    auto erase(iterator begin, iterator end) -> iterator {
        while (begin != end) {
            begin = erase(begin);
        }
        return begin;
    }

  private:
    ilist_node<NodeT> sentinel_;
};

} // namespace tinytc

#endif // ILIST_BASE_20240923_HPP
