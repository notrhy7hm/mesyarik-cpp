#pragma once

#include <cstddef>
#include <iterator>
#include <memory>
#include <type_traits>
#include <utility>

template <typename T, typename Allocator = std::allocator<T>>
class List {
private:
    struct BaseNode {
        BaseNode* prev = this;
        BaseNode* next = this;

        void swap_links(BaseNode& other) noexcept {
            using std::swap;
            swap(prev, other.prev);
            swap(next, other.next);

            if (next == &other) {
                next = this;
                prev = this;
            } else {
                next->prev = this;
                prev->next = this;
            }

            if (other.next == this) {
                other.next = &other;
                other.prev = &other;
            } else {
                other.next->prev = &other;
                other.prev->next = &other;
            }
        }
    };

    struct Node : BaseNode {
        T value;

        template <typename... Args>
        explicit Node(Args&&... args)
            : BaseNode(), value(std::forward<Args>(args)...) {}
    };

    using AllocTraits = std::allocator_traits<Allocator>;
    using NodeAllocator = typename AllocTraits::template rebind_alloc<Node>;
    using NodeAllocTraits = std::allocator_traits<NodeAllocator>;

public:
    using value_type = T;
    using allocator_type = Allocator;
    using size_type = std::size_t;
    using difference_type= std::ptrdiff_t;
    using reference = T&;
    using const_reference = const T&;

    template <bool IsConst>
    class BasicIterator {
        friend class List;
        template <bool>
        friend class BasicIterator;

        BaseNode* node_ = nullptr;

        explicit BasicIterator(BaseNode* node) noexcept : node_(node) {}

    public:
        using iterator_category = std::bidirectional_iterator_tag;
        using value_type = T;
        using difference_type = std::ptrdiff_t;
        using reference = std::conditional_t<IsConst, const T&, T&>;
        using pointer = std::conditional_t<IsConst, const T*, T*>;

        BasicIterator() noexcept = default;

        template <bool B = IsConst, typename = std::enable_if_t<B>>
        BasicIterator(const BasicIterator<false>& other) noexcept
            : node_(other.node_) {}

        reference operator*() const noexcept {
            Node* node = static_cast<Node*>(node_);
            if constexpr (IsConst) {
                return static_cast<const T&>(node->value);
            } else {
                return node->value;
            }
        }

        pointer operator->() const noexcept {
            return std::addressof(operator*());
        }

        BasicIterator& operator++() noexcept {
            node_ = node_->next;
            return *this;
        }

        BasicIterator operator++(int) noexcept {
            BasicIterator tmp(*this);
            ++(*this);
            return tmp;
        }

        BasicIterator& operator--() noexcept {
            node_ = node_->prev;
            return *this;
        }

        BasicIterator operator--(int) noexcept {
            BasicIterator tmp(*this);
            --(*this);
            return tmp;
        }

        template <bool OtherConst>
        bool operator==(const BasicIterator<OtherConst>& other) const noexcept {
            return node_ == other.node_;
        }

        template <bool OtherConst>
        bool operator!=(const BasicIterator<OtherConst>& other) const noexcept {
            return !(*this == other);
        }
    };

    using iterator = BasicIterator<false>;
    using const_iterator = BasicIterator<true>;
    using reverse_iterator = std::reverse_iterator<iterator>;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;

private:
    BaseNode sentinel_;
    size_type size_ = 0;
    [[no_unique_address]] NodeAllocator node_allocator_;

    void swap_nodes(List& other) noexcept {
        sentinel_.swap_links(other.sentinel_);
        std::swap(size_, other.size_);
    }

    void swap_all(List& other) noexcept (
        std::is_nothrow_swappable_v<NodeAllocator>) {
        swap_nodes(other);
        using std::swap;
        swap(node_allocator_, other.node_allocator_);
    }

    template <typename... Args>
    Node* create_node(Args&&... args) {
        Node* node = NodeAllocTraits::allocate(node_allocator_, 1);
        try {
            NodeAllocTraits::construct(node_allocator_, node, std::forward<Args>(args)...);
        } catch (...) { 
            NodeAllocTraits::deallocate(node_allocator_, node, 1);
            throw;
        }
        return node;
    }

    void destroy_node (Node* node) noexcept(std::is_nothrow_destructible_v<T>) {
        NodeAllocTraits::destroy(node_allocator_, node);
        NodeAllocTraits::deallocate(node_allocator_, node, 1);
    }

    iterator link_before(const_iterator pos, BaseNode* node) noexcept {
        BaseNode* right = pos.node_;
        BaseNode* left = right->prev;

        node->prev = left;
        node->next = right;
        left->next = node;
        right->prev = node;

        ++size_;
        return iterator(node);
    }

    void unlink(BaseNode* node) noexcept {
        node->prev->next = node->next;
        node->next->prev = node->prev;
    }

    void steal_nodes_from(List& other) noexcept {
        if (other.empty()) {
            sentinel_.next = &sentinel_;
            sentinel_.prev = &sentinel_;
            size_ = 0;
            return;
        }

        sentinel_.next = other.sentinel_.next;
        sentinel_.prev = other.sentinel_.prev;
        sentinel_.next->prev = &sentinel_;
        sentinel_.prev->next = &sentinel_;
        size_ = other.size_;

        other.sentinel_.next = &other.sentinel_;
        other.sentinel_.prev = &other.sentinel_;
        other.size_ = 0;
    }

public:
    List() = default;

    explicit List(const Allocator& allocator)
        : node_allocator_(allocator) {}

    explicit List(size_type count, const Allocator& allocator = Allocator())
        : List(allocator) {
        try {
            for (size_type i = 0; i < count; ++i) {
                emplace(cend());
            }
        } catch (...) {
            clear();
            throw;
        }
    }

    List (size_type count, const T& value, const Allocator& allocator = Allocator())
        : List(allocator) {
        try {
            for (size_type i = 0; i < count; ++i) {
                emplace(cend(), value);
            }
        } catch (...) {
            clear();
            throw;
        }
    }

    List (const List& other) : List(other, allocator_type(
                NodeAllocTraits::select_on_container_copy_construction(
                    other.node_allocator_))) {}

    List(const List& other, const Allocator& allocator)
        : List(allocator) {
        try {
            for (const auto& value : other) {
                push_back(value);
            }
        } catch (...) {
            clear();
            throw;
        }
    }

    List(List&& other) noexcept (
        std::is_nothrow_move_constructible_v<NodeAllocator>)
        : node_allocator_(std::move(other.node_allocator_)) {
        steal_nodes_from(other);
    }

    List(List&& other, const Allocator& allocator) 
        : List(allocator) {
        if (node_allocator_ == other.node_allocator_) {
            steal_nodes_from(other);
            return;
        }

        try {
            for (auto& value : other) {
                emplace(cend(), std::move(value));
            }
        } catch (...) {
            clear();
            throw;
        }
        other.clear();
    }

    ~List() {
        clear();
    }

    List& operator=(const List& other) {
        if (this == &other) {
            return *this;
        }

        if constexpr (AllocTraits::propagate_on_container_copy_assignment::value) {
            List tmp(other, other.get_allocator());
            swap_all(tmp);
        } else {
            List tmp(other, get_allocator());
            swap_nodes(tmp);
        }
        return *this;
    }

    List& operator=(List&& other) noexcept(
            AllocTraits::propagate_on_container_move_assignment::value
            ? std::is_nothrow_move_assignable_v<NodeAllocator>
            : AllocTraits::is_always_equal::value) {
        if (this == &other) {
            return *this;
        }

        if constexpr (AllocTraits::propagate_on_container_move_assignment::value) {
            clear();
            node_allocator_ = std::move(other.node_allocator_);
            steal_nodes_from(other);
        } else if (node_allocator_ == other.node_allocator_) {
            clear();
            steal_nodes_from(other);
        } else {
            List tmp(std::move(other), get_allocator());
            swap_nodes(tmp);
        }
        return *this;
    }

    allocator_type get_allocator() const noexcept {
        return allocator_type(node_allocator_);
    }

    [[nodiscard]] bool empty() const noexcept {
        return size_ == 0;
    }

    [[nodiscard]] size_type size() const noexcept {
        return size_;
    }

    iterator begin() noexcept {
        return iterator(sentinel_.next);
    }

    const_iterator begin() const noexcept {
        return const_iterator(sentinel_.next);
    }

    const_iterator cbegin() const noexcept {
        return const_iterator(sentinel_.next);
    }

    iterator end() noexcept {
        return iterator(&sentinel_);
    }

    const_iterator end() const noexcept {
        return const_iterator(const_cast<BaseNode*>(&sentinel_));
    }

    const_iterator cend() const noexcept {
        return const_iterator(const_cast<BaseNode*>(&sentinel_));
    }

    reverse_iterator rbegin() noexcept {
        return reverse_iterator(end());
    }

    const_reverse_iterator rbegin() const noexcept {
        return const_reverse_iterator(end());
    }

    const_reverse_iterator crbegin() const noexcept {
        return const_reverse_iterator(cend());
    }

    reverse_iterator rend() noexcept {
        return reverse_iterator(begin());
    }

    const_reverse_iterator rend() const noexcept {
        return const_reverse_iterator(begin());
    }

    const_reverse_iterator crend() const noexcept {
        return const_reverse_iterator(cbegin());
    }

    reference front() noexcept {
        return *begin();
    }

    const_reference front() const noexcept {
        return *begin();
    }

    reference back() noexcept {
        auto it = end();
        --it;
        return *it;
    }

    const_reference back() const noexcept {
        auto it = end();
        --it;
        return *it;
    }

    template <typename... Args>
    iterator emplace(const_iterator pos, Args&&... args) {
        Node* node = create_node(std::forward<Args>(args)...);
        return link_before(pos, node);
    }

    template <typename... Args>
    reference emplace_back(Args&&... args) {
        return *emplace(cend(), std::forward<Args>(args)...);
    }

    template <typename... Args>
    reference emplace_front(Args&&... args) {
        return *emplace(cbegin(), std::forward<Args>(args)...);
    }

    iterator insert(const_iterator pos, const T& value) {
        return emplace(pos, value);
    }

    iterator insert(const_iterator pos, T&& value) {
        return emplace(pos, std::move(value));
    }

    void push_back(const T& value) {
        emplace(cend(), value);
    }

    void push_back(T&& value) {
        emplace(cend(), std::move(value));
    }

    void push_front(const T& value) {
        emplace(cbegin(), value);
    }

    void push_front(T&& value) {
        emplace(cbegin(), std::move(value));
    }

    void pop_back() {
        erase(--cend());
    }

    void pop_front() {
        erase(cbegin());
    }

    iterator erase(const_iterator pos) {
        BaseNode* node = pos.node_;
        iterator result(node->next);
        unlink(node);
        destroy_node(static_cast<Node*>(node));
        --size_;
        return result;
    }

    iterator erase(const_iterator first, const_iterator last) {
        while (first != last) {
            first = erase(first);
        }
        return iterator(last.node_);
    }

    void clear() noexcept(std::is_nothrow_destructible_v<T>) {
        BaseNode* cur = sentinel_.next;
        while (cur != &sentinel_) {
            BaseNode* next = cur->next;
            destroy_node(static_cast<Node*>(cur));
            cur = next;
        }
        sentinel_.next = &sentinel_;
        sentinel_.prev = &sentinel_;
        size_ = 0;
    }

    void swap(List& other) noexcept (
            AllocTraits::propagate_on_container_swap::value
            ? std::is_nothrow_swappable_v<NodeAllocator>
            : AllocTraits::is_always_equal::value) {
        if constexpr (AllocTraits::propagate_on_containter_swap::value) {
            swap_all(other);
        } else if (node_allocator_ == other.node_allocator_){
            swap_nodes(other);
        } else {
            List lhs_copy(*this, other.get_allocator());
            List rhs_copy(other, get_allocator());
            swap_nodes(rhs_copy);
            other.swap_nodes(lhs_copy);
        }
    }

    friend void swap(List& lhs, List& rhs) noexcept(noexcept(lhs.swap(rhs))) {
        lhs.swap(rhs);
    }

    void splice(const_iterator pos, List& other, const_iterator it) {
        if (node_allocator_ != other.node_allocator_) {
            emplace(pos, std::move(*it));
            other.erase(it);
            return;
        }

        BaseNode* node = it.node_;
        other.unlink(node);
        --other.size_;
        link_before(pos, node);
    }
};
