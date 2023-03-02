#ifndef __PRETTY_FUNCTION__
#include "pretty.h"
#endif

#include <iostream>
#include <map>
#include <vector>
#include <cstring>
#include "factorial.h"

#define USE_PRETTY 1

template <typename T, std::size_t n>
struct reserve_allocator
{

    using value_type = T;

    using pointer = T *;
    using const_pointer = const T *;
    using reference = T &;
    using const_reference = const T &;

    struct memory_chunk
    {
        pointer reserved;
        memory_chunk *next_chunk = nullptr;

        memory_chunk(){};

        memory_chunk(pointer p)
        {
            reserved = p;
            next_chunk = nullptr;
        };
    };

    template <typename U>
    struct rebind
    {
        using other = reserve_allocator<U, n>;
    };

    reserve_allocator()
    {
#ifndef USE_PRETTY
        std::cout << "constuctor: [n = " << n << "]" << std::endl;
#else
        std::cout << __PRETTY_FUNCTION__ << "[n = " << n << "]" << std::endl;
#endif
        max_size = n * sizeof(T);
        auto p = static_cast<T *>(std::malloc(max_size));
        first_p = new memory_chunk(p);
        if (!first_p->reserved)
            throw std::bad_alloc();
        used_size = 0;
    }

    ~reserve_allocator()
    {
        auto p = this->first_p;
        while (p->next_chunk != nullptr)
        {
            auto next = p->next_chunk;
            if (p->reserved != nullptr)
            {
                std::free(p->reserved);
                delete p;
            }
            p = next;
        }
    }

    template <typename U, std::size_t m>
    reserve_allocator(const reserve_allocator<U, n> &)
    {
    }

    T *allocate(std::size_t m)
    {
#ifndef USE_PRETTY
        std::cout << "allocate: [m = " << m << "]" << std::endl;
#else
        std::cout << __PRETTY_FUNCTION__ << "[m = " << m << "]" << std::endl;
#endif
        auto alocate_size = m * sizeof(T);
        auto full_size = used_size + alocate_size;
        if (full_size > max_size)
        {
            // удвоим размер выделенной памяти, а если не поместится - выделим ровно сколько нужно
            auto new_size = alocate_size < max_size ? max_size : alocate_size;
            std::cout << __PRETTY_FUNCTION__ << "Reallocate to " << max_size + new_size << std::endl;
            get_last_p()->next_chunk = new memory_chunk(static_cast<T *>(std::malloc(new_size)));
            this->used_size = 0;
        }
        auto old_size = this->used_size;
        this->used_size = full_size;
        return get_last_p()->reserved + old_size;
    }

    void deallocate(T *p, std::size_t m)
    {
#ifndef USE_PRETTY
        std::cout << "Dump deallocate"
                  << "[m = " << m << ", p =" << p << "]" << std::endl;
#else
        std::cout << __PRETTY_FUNCTION__ << "[m = " << m << ", p =" << p << "]"
                  << " Dump deallocate" << std::endl;
#endif
        // освобождаем всю память за раз  в дестукторе
    }

    template <typename U, typename... Args>
    void construct(U *p, Args &&...args)
    {
#ifndef USE_PRETTY
        std::cout << "construct" << std::endl;
#else
        std::cout << __PRETTY_FUNCTION__ << std::endl;
#endif
        new (p) U(std::forward<Args>(args)...);
    }

    template <typename U>
    void destroy(U *p)
    {
#ifndef USE_PRETTY
        std::cout << "destroy" << std::endl;
#else
        std::cout << __PRETTY_FUNCTION__ << std::endl;
#endif
        p->~U();
        p = nullptr;
    }

private:
    memory_chunk *first_p;
    std::size_t max_size, used_size;

    memory_chunk *get_last_p()
    {
        auto p = this->first_p;
        while (p->next_chunk != nullptr)
        {
            p = p->next_chunk;
        }
        return p;
    }
};

namespace odl
{
    template <typename T>
    struct Node
    {
        T data;
        Node<T> *next;
        Node(T data)
        {
            this->data = data;
            this->next = nullptr;
        }
        ~Node() = default;
    };

    template <
        class T,
        class Allocator = std::allocator<Node<T>>>
    struct OneDList
    {
        using value_type = Node<T>;
        using allocator_type = Allocator;
        using pointer = typename std::allocator_traits<allocator_type>::pointer;

        OneDList()
        {
            this->allocator = allocator_type();
        }

        ~OneDList()
        {
            auto node = this->head;
            while (node->next != nullptr)
            {
                auto prev = node;
                node = node->next;
                this->allocator.deallocate(prev, 1);
            }
            this->allocator.deallocate(node, 1);
        };

        void add(T data)
        {
            auto p = allocator.allocate(1);
            allocator.construct(p, data);
            if (this->head == nullptr)
            {
                this->head = p;
            }
            else if (this->tail == nullptr)
            {

                this->head->next = p;
                this->tail = p;
            }
            else
            {
                this->tail->next = p;
                this->tail = p;
            }
        }

        Node<T> *head = nullptr, *tail = nullptr;

        void print()
        {
            auto node = this->head;
            while (node != nullptr)
            {
                std::cout << node->data << std::endl;
                node = node->next;
            }
        }

    private:
        allocator_type allocator;
    };
}
int main(int, char *[])
{
    auto m_custom = std::map<int, int, std::less<int>, reserve_allocator<std::pair<const int, int>, 2>>{}; // 2 < 10 будет реаллокация 

    for (int i = 0; i < 10; ++i)
    {
        m_custom[i] = factorial(i);
    }

    for (int i = 0; i < 10; ++i)
    {
        std::cout << i << " " << m_custom[i] << std::endl;
    }
    std::cout << std::endl;

    auto odlist = odl::OneDList<int>();
    for (int i = 0; i < 10; ++i)
    {
        odlist.add(factorial(i));
    }

    odlist.print();

    auto odlist_2 = odl::OneDList<int, reserve_allocator<odl::Node<int>, 10>>();
    for (int i = 0; i < 10; ++i)
    {
        odlist_2.add(factorial(i));
    }

    odlist_2.print();

    // расширямость аллокатора
    auto odlist_3 = odl::OneDList<int, reserve_allocator<odl::Node<int>, 2>>();
    for (int i = 0; i < 10; ++i)
    {
        odlist_3.add(factorial(i));
    }

    odlist_3.print();

    return 0;
}
