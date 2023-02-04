#include <algorithm>
#include <iostream>
#include <list>
#include <vector>

template <typename T, typename std::enable_if<std::is_integral<T>::value, T>::type = 0>
void print_ip(T t)
{
    unsigned char bytes[sizeof t];
    std::copy(static_cast<const char *>(static_cast<const void *>(&t)),
              static_cast<const char *>(static_cast<const void *>(&t)) + sizeof t,
              bytes);
    for (int i = sizeof(T) - 1; i >= 0; --i)
    {
        std::cout << int(bytes[i] & 0xFF);
        if (i != 0)
        {
            std::cout << ".";
        }
    }
    std::cout << std::endl;
}


template <typename T,  typename std::enable_if<std::is_same<std::string, T>::value>::type* = nullptr>
void print_ip(T t)
{
    std::cout << std::endl;
}


int main()
{
    print_ip(int8_t{-1});                   // 255
    print_ip(int16_t{0});                   // 0.0
    print_ip(int32_t{2130706433});          // 127.0.0.1
    print_ip(int64_t{8875824491850138409}); // 123.45.67.89.101.112.131.41
    // print_ip(std::string{"Hello, World!"});         // Hello, World!
    // print_ip(std::vector<int>{100, 200, 300, 400}); // 100.200.300.400
    // print_ip(std::list<short>{400, 300, 200, 100}); // 400.300.200.100
    // print_ip(std::make_tuple(123, 456, 789, 0));    // 123.456.789.0

    return 0;
}