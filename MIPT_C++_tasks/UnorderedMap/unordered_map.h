#pragma once

///////////////////////////////////////////////////////

#include <iostream>
#include <memory>
#include <vector>
#include <chrono>
#include <list>


template<typename T, typename Allocator = std::allocator<T>>
class List {
private:

    using AllocTraits = std::allocator_traits<Allocator>;

public:
    struct Node {
        Allocator alloc;

        T* value = nullptr;
        Node* prev = nullptr;
        Node* next = nullptr;

        Node() = default;

        void allocateValue() {
            value = AllocTraits::allocate(this->alloc, 1);
        }

        explicit Node(const Allocator& alloc, const T& element)
                :alloc(alloc) {

            allocateValue();
            AllocTraits::construct(this->alloc, value, element);
        }

        explicit Node(const Allocator& alloc, T&& element):
            alloc(alloc) {

            allocateValue();
            AllocTraits::construct(this->alloc, value, std::move(element));
        }

        template<typename... Args>
        explicit Node(const Allocator& alloc, Args&&... args)
            :alloc(alloc) {

            allocateValue();
            AllocTraits::construct(this->alloc, value, std::forward<Args>(args)...);
        }

        ~Node() {
            AllocTraits::destroy(alloc, value);
            AllocTraits::deallocate(alloc, value, 1);
        }

    };

private:

    using NodeAlloc = typename AllocTraits::template rebind_alloc<Node>;
    using NodeAllocTraits = std::allocator_traits<NodeAlloc>;


    NodeAlloc alloc;

    Node* firstNode = nullptr;
    Node* lastNode = nullptr;

    size_t length = 0;

    static void bind(Node* first, Node* second) {
        if (first != nullptr)
            first->next = second;
        if (second != nullptr)
            second->prev = first;
    }

    void insertAfter(Node* node, Node* newNode) {
        if (node != nullptr) {
            bind(newNode, node->next);
            bind(node, newNode);
        } else {
            bind(newNode, firstNode);
        }

        if (newNode->prev == nullptr)
            firstNode = newNode;
        if (newNode->next == nullptr)
            lastNode = newNode;

        ++length;
    }

public:

    Node* make(const T& value) {
        Node* newNode = NodeAllocTraits::allocate(alloc, 1);
        NodeAllocTraits::construct(alloc, newNode, static_cast<Allocator>(alloc), value);
        return newNode;
    }

    Node* make(T&& value) {
        Node* newNode = NodeAllocTraits::allocate(alloc, 1);
        NodeAllocTraits::construct(alloc, newNode, static_cast<Allocator>(alloc), std::move(value));
        return newNode;
    }

    template<typename... Args>
    Node* make(Args&&... args) {
        Node* newNode = NodeAllocTraits::allocate(alloc, 1);
        NodeAllocTraits::construct(alloc, newNode, static_cast<Allocator>(alloc), std::forward<Args>(args)...);
        return newNode;
    }

    void destroyNode(Node* node) {
        NodeAllocTraits::destroy(alloc, node);
        NodeAllocTraits::deallocate(alloc, node, 1);
    }

    Node* exclude(Node* node) {
        --length;

        if (node == firstNode)
            firstNode = node->next;
        if (node == lastNode)
            lastNode = node->prev;

        bind(node->prev, node->next);

        return node->next;
    }

private:

    Node* makeAndInsertAfter(Node* node, const T& value) {
        Node* newNode = make(value);
        insertAfter(node, newNode);
        return newNode;
    }

    Node* makeAndInsertAfter(Node* node, T&& value) {
        Node* newNode = make(std::move(value));
        insertAfter(node, newNode);
        return newNode;
    }

    template<typename... Args>
    Node* makeAndInsertAfter(Node* node, Args&&... args) {
        Node* newNode = make(std::forward<Args>(args)...);
        insertAfter(node, newNode);
        return newNode;
    }

    Node* excludeAndDestroy(Node* node) {
        Node* returnValue = exclude(node);
        destroyNode(node);
        return returnValue;
    }

public:
    template<bool isConst>
    class ProtoIterator {
        friend class List;

        template<bool isOtherConst>
        friend class ProtoIterator;

    private:
        using ValueT = std::conditional_t<isConst, const T, T>;

        Node* node;
        const List* container;

        ProtoIterator(Node* node, const List* container)
                :node(node), container(container)
        {   }

    public:

        using difference_type = std::ptrdiff_t;
        using value_type = ValueT;
        using pointer = ValueT*;
        using reference = ValueT&;
        using iterator_category = std::bidirectional_iterator_tag;

        Node* getNode() const {
            return node;
        }

        ProtoIterator()
                :ProtoIterator(nullptr, nullptr)
        {   }

        ProtoIterator(const ProtoIterator& other)
                :node(other.node), container(other.container) {  }

        void swap(ProtoIterator& other) {
            std::swap(node, other.node);
            std::swap(container, other.container);
        }

        ProtoIterator& operator=
                (const ProtoIterator& other) {
            ProtoIterator copy(other);
            swap(copy);
            return *this;
        }

        operator ProtoIterator<true>() const {
            return ProtoIterator<true>(node, container);
        }

        ~ProtoIterator() = default;

        reference operator*() const {
            return *node->value;
        }

        pointer operator->() const {
            return &**this;
        };



        ProtoIterator& operator++() {
            if (node == nullptr)
                node = container->firstNode;
            else
                node = node->next;
            return *this;
        }

        ProtoIterator& operator--() {
            if (node == nullptr)
                node = container->lastNode;
            else
                node = node->prev;
            return *this;
        }

        ProtoIterator operator++(int) {
            ProtoIterator copy(*this);
            ++*this;
            return copy;
        }

        ProtoIterator operator--(int) {
            ProtoIterator copy(*this);
            --*this;
            return copy;
        }

        template<bool isOtherConst>
        bool operator==(const ProtoIterator<isOtherConst>& other) {
            return node == other.node;
        }

        template<bool isOtherConst>
        bool operator!=(const ProtoIterator<isOtherConst>& other) {
            return !(*this == other);
        }

    };

    void total_swap(List& other) {
        std::swap(firstNode, other.firstNode);
        std::swap(lastNode, other.lastNode);
        std::swap(length, other.length);
        std::swap(alloc, other.alloc);
    }

public:

    using iterator = ProtoIterator<false>;
    using reverse_iterator = std::reverse_iterator<iterator>;
    using const_iterator = ProtoIterator<true>;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;

    explicit List(const Allocator& alloc = Allocator())
            :alloc(static_cast<NodeAlloc>(alloc))
    {   }

    void push_back(const T& value) {
        makeAndInsertAfter(lastNode, value);
    }

    void push_back(T&& value) {
        makeAndInsertAfter(lastNode, std::move(value));
    }

    template<typename... Args>
    void emplace_back(Args&&... args) {
        makeAndInsertAfter(lastNode, std::forward<Args>(args)...);
    }

    void pop_back() {
        excludeAndDestroy(lastNode);
    }

    Allocator get_allocator() const {
        return static_cast<Allocator>(alloc);
    }

    void push_front(const T& value) {
        makeAndInsertAfter(nullptr, value);
    }

    void push_front(T&& value) {
        makeAndInsertAfter(nullptr, std::move(value));
    }

    template<typename... Args>
    void emplace_front(Args&&... args) {
        makeAndInsertAfter(nullptr, std::forward<Args>(args)...);
    }

    void pop_front() {
        excludeAndDestroy(firstNode);
    }

    List(size_t count, const Allocator& alloc = Allocator())
            :List(alloc) {

        for (size_t i = 0; i < count; ++i) {
            makeAndInsertAfter(lastNode);
        }
    }

    List(size_t count, const T& value, const Allocator& alloc = Allocator())
            :List(alloc) {
        for (size_t i = 0; i < count; ++i) {
            push_back(value);
        }
    }

    [[nodiscard]] size_t size() const {
        return length;
    }

    [[nodiscard]] bool empty() const {
        return !length;
    }

    void clear() {
        while (!empty()) {
            pop_back();
        }
    }

    ~List() {
        clear();
    }


    [[nodiscard]] iterator begin() {
        return iterator(firstNode, this);
    }

    [[nodiscard]] const_iterator cbegin() const {
        return const_iterator(firstNode, this);
    }

    [[nodiscard]] const_iterator begin() const {
        return cbegin();
    }


    [[nodiscard]] iterator end() {
        return iterator(nullptr, this);
    }

    [[nodiscard]] const_iterator cend() const {
        return const_iterator(nullptr, this);
    }

    [[nodiscard]] const_iterator end() const {
        return cend();
    }


    [[nodiscard]] reverse_iterator rbegin() {
        return reverse_iterator(end());
    }

    [[nodiscard]] const_reverse_iterator crbegin() const {
        return const_reverse_iterator(cend());
    }

    [[nodiscard]] const_reverse_iterator rbegin() const {
        return crbegin();
    }


    [[nodiscard]] reverse_iterator rend() {
        return reverse_iterator(begin());
    }

    [[nodiscard]] const_reverse_iterator crend() const {
        return const_reverse_iterator(cbegin());
    }

    [[nodiscard]] const_reverse_iterator rend() const {
        return crend();
    }

    iterator insertNode(const_iterator position, Node* node) {
        --position;
        insertAfter(position.node, node);
        return iterator(node, this);
    }

    iterator insert(const_iterator position, const T& value) {
        --position;
        return iterator(makeAndInsertAfter(position.node, value), this);
    }

    iterator insert(const_iterator position, T&& value) {
        --position;
        return iterator(makeAndInsertAfter(position.node, std::move(value)), this);
    }

    template<typename... Args>
    iterator emplace(const_iterator position, Args&&... args) {
        --position;
        return iterator(makeAndInsertAfter(position.node,
                                           std::forward<Args>(args)...), this);
    }

    iterator erase(const_iterator position) {
        return iterator(excludeAndDestroy(position.node), this);
    }

    List(const List& other, const Allocator& alloc)
            : List(alloc) {

        for (const auto& it: other) {
            push_back(it);
        }
    }

    List(const List& other)
            : List(other, AllocTraits::select_on_container_copy_construction(static_cast<Allocator>(other.alloc)))
    {   }

    List(List&& other) noexcept
            : List(other.alloc) {

        firstNode = other.firstNode;
        lastNode = other.lastNode;
        length = other.length;

        other.firstNode = nullptr;
        other.lastNode = nullptr;
        other.length = 0;
    }


    List& operator=(const List& other) {

        if (this != &other) {
            clear();

            if (AllocTraits::propagate_on_container_copy_assignment::value)
                alloc = other.alloc;

            for (const auto& it: other) {
                push_back(it);
            }
        }

        return *this;
    }

    List& operator=(List&& other) noexcept {
        firstNode = other.firstNode;
        lastNode = other.lastNode;
        length = other.length;

        other.firstNode = nullptr;
        other.lastNode = nullptr;
        other.length = 0;

        if (AllocTraits::propagate_on_container_move_assignment::value)
            alloc = other.alloc;

        return *this;
    }

};

///////////////////////////////////////////////////////////////

#include <functional>
#include <cmath>


template<typename Key, typename Value,
         typename Hash = std::hash<Key>,
         typename Equal = std::equal_to<Key>,
         typename Alloc = std::allocator<std::pair<const Key, Value>>>
class UnorderedMap {
private:

    using KeyValPair = std::pair<const Key, Value>;
    using AllocTraits = std::allocator_traits<Alloc>;
    using KeyValList = List<KeyValPair, Alloc>;

public:
    using iterator = typename KeyValList::iterator;
    using const_iterator = typename KeyValList::const_iterator;

private:

    using IteratorAlloc = typename AllocTraits::template rebind_alloc<iterator>;
    using KeyValListNode = typename KeyValList::Node;
    using BucketVector = std::vector<iterator, IteratorAlloc>;

private:

    static const size_t START_BUCKET_COUNT = 4;
    constexpr static const double DEFAULT_MAX_LOAD_FACTOR = 1.0;

    double maxLoadFactor = DEFAULT_MAX_LOAD_FACTOR;
    size_t bucketCount;

    Hash hashFunc;
    Equal equal;
    Alloc alloc;

    KeyValList keyValList;
    BucketVector bucketBegins;
    BucketVector bucketEnds;

    [[nodiscard]] size_t getBucket(const Key& key) const {
        return hashFunc(key) % bucketCount;
    }

    std::pair<iterator, bool> insertListNode(KeyValListNode* listNode) {
        auto it = find(listNode->value->first);
        if (it != end()) {
            keyValList.destroyNode(listNode);
            return std::make_pair(it, false);
        }

        reserve(size() + 1);

        size_t bucket = getBucket(listNode->value->first);
        it = bucketBegins[bucket];

        auto newIt = keyValList.insertNode(it, listNode);

        bucketBegins[bucket] = newIt;
        if (it == end())
            bucketEnds[bucket] = newIt;

        return std::make_pair(newIt, true);
    }


public:

    UnorderedMap()
            : UnorderedMap(START_BUCKET_COUNT)
    {   }

    explicit UnorderedMap(size_t bucketCount,
            const Hash& hash = Hash(),
            const Equal& equal = Equal(),
            const Alloc& alloc = Alloc())

            : bucketCount(bucketCount), hashFunc(hash), equal(equal),
            alloc(alloc), keyValList(alloc),
            bucketBegins(bucketCount, keyValList.end(), alloc),
            bucketEnds(bucketCount, keyValList.end(), alloc)
    {   }

    UnorderedMap(size_t bucketCount, const Alloc& alloc)
            : UnorderedMap(bucketCount, Hash(), Equal(), alloc)
    {   }

    UnorderedMap(size_t bucketCount, const Hash& hash,
        const Alloc& alloc)
            : UnorderedMap(bucketCount, hash, Equal(), alloc)
    {   }

    explicit UnorderedMap(const Alloc& alloc)
            : UnorderedMap(START_BUCKET_COUNT, alloc)
    {   }

    UnorderedMap(const UnorderedMap& other, const Alloc& alloc)
        : UnorderedMap(alloc) {

        for (const auto& it: other) {
            insert(it);
        }
    }

    UnorderedMap(const UnorderedMap& other)
            : UnorderedMap(other,
                           AllocTraits::select_on_container_copy_construction(other.alloc))
    {   }

    UnorderedMap(UnorderedMap&& other)
            : maxLoadFactor(other.maxLoadFactor),
              bucketCount(other.bucketCount),
              hashFunc(std::move(other.hashFunc)),
              equal(std::move(other.equal)),
              alloc(other.alloc),
              keyValList(std::move(other.keyValList)),
              bucketBegins(std::move(other.bucketBegins)),
              bucketEnds(std::move(other.bucketEnds)) {

        other.bucketCount = 0;
    }

    ~UnorderedMap() = default;

    UnorderedMap& operator=(const UnorderedMap& other) {
        Alloc newAlloc =
                (AllocTraits::propagate_on_container_copy_assignment::value ?
                    other.alloc : alloc);
        UnorderedMap copy(other, newAlloc);
        total_swap(copy);
        return *this;
    }

    UnorderedMap& operator=(UnorderedMap&& other) noexcept {
        maxLoadFactor = other.maxLoadFactor;
        bucketCount = other.bucketCount;
        hashFunc = std::move(other.hashFunc);
        equal = std::move(other.equal);
        if (AllocTraits::propagate_on_container_move_assignment::value)
            alloc = other.alloc;
        keyValList = std::move(other.keyValList);
        bucketBegins = std::move(other.bucketBegins);
        bucketEnds = std::move(other.bucketEnds);

        return *this;
    }




    void rehash(size_t count) {
        if (bucketCount >= count)
            return;

        KeyValList oldKeyValList(alloc);
        oldKeyValList.total_swap(keyValList);

        bucketCount = count;
        bucketBegins.assign(count, keyValList.end());
        bucketEnds.assign(count, keyValList.end());

        auto it = oldKeyValList.begin();

        while (!oldKeyValList.empty()) {
            auto nextIt = it;
            ++nextIt;

            KeyValListNode* listNode = it.getNode();
            oldKeyValList.exclude(listNode);
            insertListNode(listNode);

            it = nextIt;
        }
    }

    void reserve(size_t count) {
        size_t neededBucketCount = std::ceil(count / maxLoadFactor);
        if (bucketCount < neededBucketCount)
            rehash(2 * neededBucketCount);
    }

    [[nodiscard]] size_t size() const {
        return keyValList.size();
    }

    [[nodiscard]] bool empty() const {
        return keyValList.empty();
    }

    [[nodiscard]] iterator begin() {
        return keyValList.begin();
    }

    [[nodiscard]] const_iterator begin() const {
        return keyValList.cbegin();
    }

    [[nodiscard]] const_iterator cbegin() const {
        return keyValList.cbegin();
    }

    [[nodiscard]] iterator end() {
        return keyValList.end();
    }

    [[nodiscard]] const_iterator end() const {
        return keyValList.cend();
    }

    [[nodiscard]] const_iterator cend() const {
        return keyValList.cend();
    }

    iterator find(const Key& key) {
        size_t bucket = getBucket(key);
        if (bucketBegins[bucket] == end())
            return end();

        for (auto it = bucketBegins[bucket];; ++it) {
            if (equal(it->first, key))
                return it;
            if (it == bucketEnds[bucket])
                break;
        }
        return end();
    }

    const_iterator find(const Key& key) const {
        size_t bucket = getBucket(key);
        if (bucketBegins[bucket] == cend())
            return cend();

        for (auto it = bucketBegins[bucket];; ++it) {
            if (equal(it->first, key))
                return it;
            if (it == bucketEnds[bucket])
                break;
        }
        return cend();
    }

    std::pair<iterator, bool> insert(const KeyValPair& value) {
        KeyValPair copy(value);
        return insert(std::move(copy));
    }

    std::pair<iterator, bool> insert(KeyValPair&& value) {
        KeyValListNode* listNode = keyValList.make(std::move(value));
        return insertListNode(listNode);
    }

    template<typename P>
    std::pair<iterator, bool> insert(P&& value) {
        return emplace(std::forward<P>(value));
    }

    template<typename... Args>
    std::pair<iterator, bool> emplace(Args&&... args) {
        KeyValListNode* listNode
            = keyValList.make(std::forward<Args>(args)...);

        return insertListNode(listNode);
    }

    template<typename InputIt>
    void insert(InputIt first, InputIt last) {
        for (auto it = first; it != last; ++it) {
            insert(*it);
        }
    }

    Value& operator[](const Key& key) {
        auto it = find(key);
        if (it == end())
            it = insert(KeyValPair(key, Value())).first;
        return it->second;
    }

    Value& at(const Key& key) {
        auto it = find(key);
        if (it == end())
            throw std::out_of_range("Key is not in the container.");
        return it->second;
    }

    const Value& at(const Key& key) const {
        auto it = find(key);
        if (it == end())
            throw std::out_of_range("Key is not in the container.");
        return it->second;
    }

    iterator erase(const_iterator position) {
        size_t bucket = getBucket(position->first);
        if (bucketBegins[bucket] == bucketEnds[bucket])
            bucketBegins[bucket] = bucketEnds[bucket] = keyValList.end();
        else if (bucketBegins[bucket] == position)
            ++bucketBegins[bucket];
        else if (bucketEnds[bucket] == position)
            --bucketEnds[bucket];
        return keyValList.erase(position);
    }

    const_iterator erase(const_iterator first, const_iterator last) {
        for (auto it = first; it != last;) {
            auto nextIt =  it;
            ++nextIt;
            keyValList.erase(it);
            it = nextIt;
        }
        return last;
    }

    [[nodiscard]] size_t max_size() const {
        return std::numeric_limits<std::ptrdiff_t>::max() /
               (sizeof(KeyValPair) + 2 * sizeof(iterator)) - 20;
    }

    [[nodiscard]] double load_factor() const {
        return 1.0 * size() / bucketCount;
    }

    [[nodiscard]] double max_load_factor() const {
        return maxLoadFactor;
    }

    void max_load_factor(double newMaxLoadFactor) {
        maxLoadFactor = newMaxLoadFactor;
    }
};