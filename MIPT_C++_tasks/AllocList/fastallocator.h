#pragma once

#include <iostream>
#include <memory>
#include <vector>
#include <chrono>
#include <list>

namespace Helpers {

    template<typename T, bool isConst>
    struct ConstHanger {
        using type = T;
    };

    template<typename T>
    struct ConstHanger<T, true> {
        using type = const T;
    };


} // namespace Helpers

template<size_t chunkSize>
class FixedAllocator {
private:
    char* pool = new char[POOL_SIZE];
    size_t next = 0;

    static const size_t POOL_SIZE = (1u << 27);

public:
    FixedAllocator() = default;

    ~FixedAllocator() {
        delete[] pool;
    }

    FixedAllocator(const FixedAllocator& other) = delete;

    FixedAllocator& operator=(const FixedAllocator& other) = delete;

    bool isInside(void* pointer) {
        return pool <= pointer && pointer < pool + POOL_SIZE;
    }

    void* allocate(size_t neededSize) {
        // pool size check is missing to increase performance
        size_t byteNeededSize = neededSize * chunkSize;
        next += byteNeededSize;
        return static_cast<void*>(pool + (next - byteNeededSize));
    }

    void deallocate(void* pointer, size_t count) {  pointer = pointer; count = count;  }
};


template<typename T>
class FastAllocator {

    template<typename U>
    friend class FastAllocator;

private:
    static const size_t CHUNK_SIZE = 1;
    static const size_t MAX_FIXED_ALLOCATION_SIZE = 128;

    using FixedAlloc = FixedAllocator<CHUNK_SIZE>;

    std::shared_ptr<FixedAlloc> fixedAlloc =
            std::make_shared<FixedAlloc>();


public:
    using value_type = T;

    FastAllocator() = default;

    ~FastAllocator() = default;

    template<typename U>
    FastAllocator(const FastAllocator<U>& other)
            :fixedAlloc(other.fixedAlloc)
    {   }

    void swap(FastAllocator& other) {
        std::swap(fixedAlloc, other.fixedAlloc);
    }

    FastAllocator& operator=(const FastAllocator& other) {
        FastAllocator copy(other);
        swap(copy);
        return *this;
    }

    value_type* allocate(size_t count) {
        size_t neededSize = count * sizeof(T);
        if (neededSize <= MAX_FIXED_ALLOCATION_SIZE)
            return reinterpret_cast<T*>(fixedAlloc->allocate(neededSize));
        return reinterpret_cast<T*>(::operator new(neededSize));
    }

    void deallocate(value_type* pointer, size_t count) {
        void* castPointer = reinterpret_cast<void*>(pointer);
        if (fixedAlloc->isInside(castPointer))
            fixedAlloc->deallocate(castPointer, count);
        else
            ::operator delete(castPointer);
    }

    bool operator==(const FastAllocator& other) const {
        return fixedAlloc == other.fixedAlloc;
    }

    bool operator!=(const FastAllocator& other) const {
        return !(*this == other);
    }

};


template<typename T, typename Allocator = std::allocator<T>>
class List {
private:

    using AllocTraits = std::allocator_traits<Allocator>;

    struct Node {
        T value;

        Node* prev = nullptr;
        Node* next = nullptr;

        Node() = default;

        explicit Node(const T& value)
                :value(value)
        {   }
    };


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

    Node* makeAndInsertAfter(Node* node, const T& value) {
        Node* newNode = NodeAllocTraits::allocate(alloc, 1);
        NodeAllocTraits::construct(alloc, newNode, value);
        insertAfter(node, newNode);
        return newNode;
    }

    Node* exclude(Node* node) {
        --length;

        if (node == firstNode)
            firstNode = node->next;
        if (node == lastNode)
            lastNode = node->prev;

        bind(node->prev, node->next);

        Node* returnValue = node->next;

        NodeAllocTraits::destroy(alloc, node);
        NodeAllocTraits::deallocate(alloc, node, 1);

        return returnValue;
    }

public:
    template<bool isConst>
    class ProtoIterator {
        friend class List;

        template<bool isOtherConst>
        friend class ProtoIterator;

    private:
        using ValueT = typename Helpers::template ConstHanger<T, isConst>::type;

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
            return node->value;
        }

        pointer operator->() const {
            return &*this;
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

    void pop_back() {
        exclude(lastNode);
    }

    Allocator get_allocator() const {
        return static_cast<Allocator>(alloc);
    }

    void push_front(const T& value) {
        makeAndInsertAfter(nullptr, value);
    }

    void pop_front() {
        exclude(firstNode);
    }

    List(size_t count, const Allocator& alloc = Allocator())
        :List(alloc) {

        for (size_t i = 0; i < count; ++i) {
            Node* newNode = NodeAllocTraits::allocate(this->alloc, 1);
            NodeAllocTraits::construct(this->alloc, newNode);
            insertAfter(lastNode, newNode);
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


    iterator insert(const_iterator position, const T& value) {
        --position;
        return iterator(makeAndInsertAfter(position.node, value), this);
    }

    iterator erase(const_iterator position) {
        return iterator(exclude(position.node), this);
    }


    List(const List& other)
            :alloc(static_cast<NodeAlloc>(
                    AllocTraits::select_on_container_copy_construction(static_cast<Allocator>(other.alloc)))) {

        for (const auto& it: other) {
            push_back(it);
        }
    }

    void swap(List& other) {
        std::swap(firstNode, other.firstNode);
        std::swap(lastNode, other.lastNode);
        std::swap(length, other.length);

        if (NodeAllocTraits::propagate_on_container_swap::value)
            std::swap(alloc, other.alloc);
    }

    List& operator=(const List& other) {

        if (this != &other) {
            clear();

            if (NodeAllocTraits::propagate_on_container_copy_assignment::value)
                alloc = other.alloc;

            for (const auto& it: other) {
                push_back(it);
            }
        }

        return *this;
    }

};