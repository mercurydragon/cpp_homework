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
        bool is_main_pointer = false;

        memory_chunk() = default;;

        memory_chunk(pointer p)
        {
            reserved = p;
            next_chunk = nullptr;
        };

        memory_chunk(std::size_t size)
        {
            reserved = static_cast<T *>(std::malloc(size));
            next_chunk = nullptr;
            is_main_pointer = true;
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
        reserve_memory();
        used_chunk = nullptr;
    }

    ~reserve_allocator()
    {
        if (used_chunk != nullptr) free_chuncks_list(this->used_chunk);
        if (free_chunk != nullptr) free_chuncks_list(this->free_chunk);
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
        // в констукторе мы нарезали чанков для одного объекта типа T (так как это самый частых сценарий аллокации), если m == 1 то вернем зарезервированный чанк, иначе - нарежем новый
        memory_chunk * used;
        if (m == 1) {
            if (free_chunk == nullptr)
                reserve_memory(); // резервируем еще n памяти
            used = free_chunk;
            free_chunk = free_chunk->next_chunk;
        } else {
            used = new memory_chunk(m * sizeof(T));
        }
        used->next_chunk = used_chunk;
        used_chunk = used;
        return used->reserved;
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
    memory_chunk *free_chunk, *used_chunk;

    void reserve_memory() {
#ifndef USE_PRETTY
        std::cout << "reserve_memory" << std::endl;
#else
        std::cout << __PRETTY_FUNCTION__ << std::endl;
#endif
        auto p = static_cast<T *>(malloc(n * sizeof(T)));
        free_chunk = new memory_chunk(p);
        free_chunk->is_main_pointer = true;
        if (!free_chunk->reserved)
            throw std::bad_alloc();
        auto current = free_chunk;
        for (unsigned long i = 1; i < n; i++) {
            current->next_chunk = new memory_chunk(static_cast<T *>(current->reserved + sizeof(T)));
            current = current->next_chunk;
        }
    }

    void free_chuncks_list(memory_chunk *p) const {
        while (p->next_chunk != nullptr)
        {
            auto next = p->next_chunk;
            if (p->reserved != nullptr)
            {
                if (p->is_main_pointer)
                    free(p->reserved);
                delete p;
            }
            p = next;
        }
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
