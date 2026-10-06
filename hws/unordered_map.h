#pragma once

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <functional>
#include <iterator>
#include <memory>
#include <new>
#include <stdexcept>
#include <type_traits>
#include <utility>
#include <vector>

template <
    typename Key,
    typename Value,
    typename Hash = std::hash<Key>,
    typename EqualTo = std::equal_to<Key>,
    typename Allocator = std::allocator<std::pair<const Key, Value>>>
class UnorderedMap {
public:
    using key_type = Key;
    using mapped_type = Value;
    using value_type = std::pair<const Key, Value>;
    using size_type = std::size_t;
    using difference_type = std::ptrdiff_t;
    using hasher = Hash;
    using key_equal = EqualTo;
    using allocator_type = Allocator;
    using reference = value_type&;
    using const_reference = const value_type&;

private:
    struct BaseNode {
        BaseNode* prev = this;
        BaseNode* next = this;
    };

    struct Node : BaseNode {
        Node* bucket_next = nullptr;
        size_type hash = 0;
        alignas(value_type) std::byte storage[sizeof(value_type)];

        value_type* value_ptr() noexcept {
            return std::launder(reinterpret_cast<value_type*>(storage));
        }

        const value_type* value_ptr() const noexcept {
            return std::launder(reinterpret_cast<const value_type*>(storage));
        }
    };

    using AllocTraits = std::allocator_traits<allocator_type>;
    using NodeAllocator = typename AllocTraits::template rebind_alloc<Node>;
    using NodeAllocTraits = std::allocator_traits<NodeAllocator>;

public:
    template <bool IsConst>
    class BasicIterator {
        friend class UnorderedMap;
        template <bool>
        friend class BasicIterator;

        BaseNode* node_ = nullptr;
        explicit BasicIterator(BaseNode* node) noexcept : node_(node) {}

    public:
        using iterator_category = std::bidirectional_iterator_tag;
        using value_type = std::pair<const Key, Value>;
        using difference_type = std::ptrdiff_t;
        using reference = std::conditional_t<IsConst, const value_type&, value_type&>;
        using pointer = std::conditional_t<IsConst, const value_type*, value_type*>;

        BasicIterator() noexcept = default;

        template <bool B = IsConst, typename = std::enable_if_t<B>>
        BasicIterator(const BasicIterator<false>& other) noexcept : node_(other.node_) {}

        reference operator*() const noexcept {
            Node* node = static_cast<Node*>(node_);
            if constexpr (IsConst) {
                return static_cast<const value_type&>(*node->value_ptr());
            } else {
                return *node->value_ptr();
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

private:
    static constexpr size_type kInitialBucketCount = 8;

    BaseNode sentinel_;
    size_type size_ = 0;
    float max_load_factor_ = 1.0f;
    hasher hash_;
    key_equal equal_;
    [[no_unique_address]] allocator_type allocator_;
    std::vector<Node*> buckets_;

    NodeAllocator node_allocator() const {
        return NodeAllocator(allocator_);
    }

    void reset_sentinel() noexcept {
        sentinel_.next = &sentinel_;
        sentinel_.prev = &sentinel_;
    }

    void steal_nodes_from(UnorderedMap& other) noexcept {
        if (other.empty()) {
            reset_sentinel();
            size_ = 0;
            return;
        }

        sentinel_.next = other.sentinel_.next;
        sentinel_.prev = other.sentinel_.prev;
        sentinel_.next->prev = &sentinel_;
        sentinel_.prev->next = &sentinel_;
        size_ = other.size_;

        other.reset_sentinel();
        other.size_ = 0;
    }

    void link_global_back(Node* node) noexcept {
        BaseNode* left = sentinel_.prev;
        node->prev = left;
        node->next = &sentinel_;
        left->next = node;
        sentinel_.prev = node;
    }

    void unlink_global(Node* node) noexcept {
        node->prev->next = node->next;
        node->next->prev = node->prev;
    }

    size_type bucket_index(size_type hash_value) const noexcept {
        return hash_value % buckets_.size();
    }

    void ensure_initial_buckets() {
        if (buckets_.empty()) {
            buckets_.assign(kInitialBucketCount, nullptr);
        }
    }

    size_type minimum_bucket_count_for(size_type element_count) const {
        if (element_count == 0) {
            return 1;
        }
        return static_cast<size_type>(
                std::ceil(static_cast<double>(element_count) / static_cast<double>(max_load_factor_)));
    }

    void ensure_capacity_for_one_more() {
        ensure_initial_buckets();
        if (static_cast<double>(size_ + 1) <=
                static_cast<double>(buckets_.size()) * max_load_factor_) {
            return;
        }
        
        size_type next_count = std::max(buckets_.size() * 2,
                minimum_bucket_count_for(size_ + 1));
        rehash(next_count);
    }

    template <typename... Args>
    Node* create_node(Args&&... args) {
        NodeAllocator node_alloc = node_allocator();
        Node* node = NodeAllocTraits::allocate(node_alloc, 1);
        bool node_constructed = false;
        bool value_constructed = false;

        try {
            NodeAllocTraits::construct(node_alloc, node);
            node_constructed = true;

            AllocTraits::construct(
                    allocator_, node->value_ptr(), std::forward<Args>(args)...);
            value_constructed = true;

            node->hash = hash_(node->value_ptr()->first);
            return node;
        } catch(...) {
            if (value_constructed) {
                AllocTraits::destroy(allocator_, node->value_ptr());
            }
            if (node_constructed) {
                NodeAllocTraits::destroy(node_alloc, node);
            }
            NodeAllocTraits::deallocate(node_alloc, node, 1);
            throw;
        }
    }

    void destroy_node(Node* node) noexcept {
        AllocTraits::destroy(allocator_, node->value_ptr());
        NodeAllocator node_alloc = node_allocator();
        NodeAllocTraits::destroy(node_alloc, node);
        NodeAllocTraits::deallocate(node_alloc, node, 1);
    }

    Node* find_node_with_hash(const key_type& key, size_type hash_value) noexcept {
        if (buckets_.empty()) {
            return nullptr;
        }

        Node* cur = buckets_[bucket_index(hash_value)];
        while (cur != nullptr) {
            if (cur->hash == hash_value && equal_(cur->value_ptr()->first, key)) {
                return cur;
            }
            cur = cur->bucket_next;
        }
        return nullptr;
    }

    const Node* find_node_with_hash(const key_type& key, size_type hash_value) const noexcept {
        if (buckets_.empty()) {
            return nullptr;
        }

        const Node* cur = buckets_[bucket_index(hash_value)];
        while (cur != nullptr) {
            if (cur->hash == hash_value && equal_(cur->value_ptr()->first, key)) {
                return cur;
            }
            cur = cur->bucket_next;
        }
        return nullptr;
    }

    void link_into_bucket(Node* node) noexcept {
        size_type index = bucket_index(node->hash);
        node->bucket_next = buckets_[index];
        buckets_[index] = node;
    }

    void unlink_from_bucket(Node* node) noexcept {
        size_type index = bucket_index(node->hash);
        Node** cur = &buckets_[index];
        while (*cur != nullptr) {
            if (*cur == node) {
                *cur = node->bucket_next;
                node->bucket_next = nullptr;
                return;
            }
            cur = &((*cur)->bucket_next);
        }
    }

    iterator erase_node (Node* node) {
        iterator result(node->next);
        unlink_from_bucket(node);
        unlink_global(node);
        destroy_node(node);
        --size_;
        return result;
    }

public:
    UnorderedMap()
        : hash_(),
        equal_(),
        allocator_(),
        buckets_(kInitialBucketCount, nullptr) {}

    explicit UnorderedMap(const allocator_type& allocator)
        : hash_(),
        equal_(),
        allocator_(allocator),
        buckets_(kInitialBucketCount, nullptr) {}

    explicit UnorderedMap(
            size_type bucket_count,
            const hasher& hash = hasher(),
            const key_equal& equal = key_equal(),
            const allocator_type& allocator = allocator_type())
        : hash_(hash),
        equal_(equal),
        allocator_(allocator),
        buckets_(std::max<size_type>(bucket_count, 1), nullptr) {}

    UnorderedMap(const UnorderedMap& other) 
        : size_(0),
        max_load_factor_(other.max_load_factor_),
        hash_(other.hash_),
        equal_(other.equal_),
        allocator_(AllocTraits::select_on_container_copy_construction(other.allocator_)),
        buckets_(std::max<size_type>(other.buckets_.size(), 1), nullptr) {
        try {
            insert(other.cbegin(), other.cend());
        } catch (...) {
            clear();
            throw;
        }
    }

    UnorderedMap(const UnorderedMap& other, const allocator_type& allocator)
        : size_(0),
        max_load_factor_(other.max_load_factor_),
        hash_(other.hash_),
        equal_(other.equal_),
        allocator_(allocator),
        buckets_(std::max<size_type>(other.buckets_.size(), 1), nullptr) {
        try {
            insert(other.cbegin(), other.cend());
        } catch (...) {
            clear();
            throw;
        }
    }

    UnorderedMap(UnorderedMap&& other) noexcept (
            std::is_nothrow_move_constructible_v<hasher> &&
            std::is_nothrow_move_constructible_v<key_equal> &&
            std::is_nothrow_move_constructible_v<allocator_type>)
        : size_(0),
        max_load_factor_(other.max_load_factor_),
        hash_(std::move(other.hash_)),
        equal_(std::move(other.equal_)),
        allocator_(std::move(other.allocator_)),
        buckets_(std::move(other.buckets_)) {
        steal_nodes_from(other);
    }

    ~UnorderedMap() {
        clear();
    }

    UnorderedMap& operator=(const UnorderedMap& other) {
        if (this == &other) {
            return *this;
        }

        if constexpr (AllocTraits::propagate_on_container_copy_assignment::value) {
            UnorderedMap tmp(other, other.allocator_);
            clear();
            allocator_ = other.allocator_;
            hash_ = other.hash_;
            equal_ = other.equal_;
            max_load_factor_ = other.max_load_factor_;
            buckets_ = std::move(tmp.buckets_);
            steal_nodes_from(tmp);
        } else {
            UnorderedMap tmp(other, allocator_);
            clear();
            hash_ = other.hash_;
            equal_ = other.equal_;
            max_load_factor_ = other.max_load_factor_;
            buckets_ = std::move(tmp.buckets_);
            steal_nodes_from(tmp);
        }
        return *this;
    }

    UnorderedMap& operator=(UnorderedMap&& other) noexcept (
            AllocTraits::propagate_on_container_move_assignment::value
            ? (std::is_nothrow_move_assignable_v<allocator_type> &&
            std::is_nothrow_move_assignable_v<hasher> &&
            std::is_nothrow_move_assignable_v<key_equal>)
            : AllocTraits::is_always_equal::value) {
        if (this == &other) {
            return *this;
        }

        if constexpr (AllocTraits::propagate_on_container_move_assignment::value) {
            clear();
            allocator_ = std::move(other.allocator_);
            hash_ = std::move(other.hash_);
            equal_ = std::move(other.equal_);
            max_load_factor_ = other.max_load_factor_;
            buckets_ = std::move(other.buckets_);
            steal_nodes_from(other);
        } else if (allocator_ == other.allocator_) {
            clear();
            hash_ = std::move(other.hash_);
            equal_ = std::move(other.equal_);
            max_load_factor_ = other.max_load_factor_;
            buckets_ = std::move(other.buckets_);
            steal_nodes_from(other);
        } else {
            UnorderedMap tmp(0, other.hash_, other.equal_, allocator_);
            tmp.max_load_factor_ = other.max_load_factor_;
            tmp.reserve(other.size_);
            for (auto& element : other) {
                tmp.emplace(element.first, std::move(element.second));
            }
            clear();
            hash_ = std::move(tmp.hash_);
            equal_ = std::move(tmp.equal_);
            max_load_factor_ = tmp.max_load_factor_;
            buckets_ = std::move(tmp.buckets_);
            steal_nodes_from(tmp);
            other.clear();
        }
        return *this;
    }

    [[nodiscard]] allocator_type get_allocator() const noexcept {
        return allocator_;
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

    template <typename... Args>
    std::pair<iterator, bool> emplace(Args&&... args) {
        Node* candidate = create_node(std::forward<Args>(args)...);

        if (Node* existing = find_node_with_hash(
                    candidate->value_ptr()->first, candidate->hash)) {
            destroy_node(candidate);
            return {iterator(existing), false};
        }

        try {
            ensure_capacity_for_one_more();
        } catch (...) {
            destroy_node(candidate);
            throw;
        }

        link_global_back(candidate);
        link_into_bucket(candidate);
        ++size_;
        return {iterator(candidate), true};
    }

    std::pair<iterator, bool> insert(const value_type& value) {
        return emplace(value);
    }

    std::pair<iterator, bool> insert(value_type&& value) {
        return emplace(std::move(value));
    }

    template <typename P, typename = std::enable_if_t<
        std::is_constructible_v<value_type, P&&>>>
    std::pair<iterator, bool> insert(P&& value) {
        return emplace(std::forward<P>(value));
    }

    template <typename InputIt>
    void insert(InputIt first, InputIt last) {
        for (; first != last; ++first) {
            insert(*first);
        }
    }

    mapped_type& operator[](const key_type& key) {
        auto result = emplace(key, mapped_type{});
        return result.first->second;
    }

    mapped_type& operator[](key_type&& key) {
        auto result = emplace(std::move(key), mapped_type{});
        return result.first->second;
    }

    mapped_type& at(const key_type& key) {
        iterator it = find(key);
        if (it == end()) {
            throw std::out_of_range("UnorderedMap::at: key not found");
        }
        return it->second;
    }

    const mapped_type& at(const key_type& key) const {
        const_iterator it = find(key);
        if (it == cend()) {
            throw std::out_of_range("UnorderedMap::at: key not found");
        }
        return it->second;
    }

    iterator find(const key_type& key) {
        if (buckets_.empty()) {
            return end();
        }
        size_type hash_value = hash_(key);
        Node* node = find_node_with_hash(key, hash_value);
        return node != nullptr ? iterator(node) : end();
    }

    const_iterator find(const key_type& key) const {
        if (buckets_.empty()) {
            return cend();
        }
        size_type hash_value = hash_(key);
        const Node* node = find_node_with_hash(key, hash_value);
        return node != nullptr
            ? const_iterator(const_cast<Node*>(node))
            : cend();
    }

    [[nodiscard]] bool contains(const key_type& key) const {
        return find(key) != cend();
    }

    [[nodiscard]] size_type count(const key_type& key) const {
        return contains(key) ? 1 : 0;
    }

    iterator erase(const_iterator pos) {
        BaseNode* base = pos.node_;
        if (base == &sentinel_) {
            return end();
        }
        return erase_node(static_cast<Node*>(base));
    }

    iterator erase(iterator pos) {
        return erase(const_iterator(pos));
    }

    iterator erase(const_iterator first, const_iterator last) {
        while (first != last) {
            first = erase(first);
        }
        return iterator(last.node_);
    }

    size_type erase(const key_type& key) {
        iterator it = find(key);
        if (it == end()) {
            return 0;
        }
        erase(it);
        return 1;
    }

    void clear() noexcept {
        BaseNode* cur = sentinel_.next;
        while(cur != &sentinel_) {
            BaseNode* next = cur->next;
            destroy_node(static_cast<Node*>(cur));
            cur = next;
        }
        reset_sentinel();
        size_ = 0;
        std::fill(buckets_.begin(), buckets_.end(), nullptr);
    }

    void rehash(size_type count) {
        size_type required = minimum_bucket_count_for(size_);
        count = std::max(count, required);
        count = std::max<size_type>(count, 1);

        if (count == buckets_.size()) {
            return;
        }

        std::vector<Node*> new_buckets(count, nullptr);
        for (BaseNode* cur = sentinel_.next; cur != &sentinel_; cur = cur->next) {
            Node* node = static_cast<Node*>(cur);
            size_type index = node->hash % count;
            node->bucket_next = new_buckets[index];
            new_buckets[index] = node;
        }
        buckets_.swap(new_buckets);
    }

    void reserve(size_type count) {
        size_type required = minimum_bucket_count_for(count);
        if (required > buckets_.size()) {
            rehash(required);
        }
    }

    [[nodiscard]] size_type bucket_count() const noexcept {
        return buckets_.size();
    }

    [[nodiscard]] float load_factor() const noexcept {
        if (buckets_.empty()) {
            return 0.0f;
        }
        return static_cast<float>(size_) /
            static_cast<float>(buckets_.size());
    }

    [[nodiscard]] float max_load_factor() const noexcept {
        return max_load_factor_;
    }

    void max_load_factor(float value) {
        if (!(value > 0.0f)) {
            throw std::invalid_argument("max_load_factor must be positive");
        }
        max_load_factor_ = value;
        if (size_ > 0 && load_factor() > max_load_factor_) {
            rehash(minimum_bucket_count_for(size_));
        }
    }

    [[nodiscard]] hasher hash_function() const {
        return hash_;
    }

    [[nodiscard]] key_equal key_eq() const {
        return equal_;
    }
};
