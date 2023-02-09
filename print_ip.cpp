#include <algorithm>
#include <iostream>
#include <list>
#include <vector>

// some shortcuts for type defenition
namespace checker
{
    template <typename T>
    struct is_list_or_vector
    {
        enum
        {
            value = false
        };
    };

    template <typename T, typename A>
    struct is_list_or_vector<std::vector<T, A>>
    {
        enum
        {
            value = true
        };
    };

    template <typename T, typename A>
    struct is_list_or_vector<std::list<T, A>>
    {
        enum
        {
            value = true
        };
    };

    template <typename T>
    struct is_integer
        : std::integral_constant<
              bool,
              std::is_integral<T>::value &&
                  !std::is_same<bool, T>::value &&
                  !std::is_same<char, T>::value>
    {
    };

    template <class T, class... Rest>
    inline constexpr bool are_all_same = (std::is_same_v<T, Rest> && ...);

} // namespace checker

// print for list and vector
template <typename T>
void print_ip(T t, typename std::enable_if<checker::is_list_or_vector<T>::value, T>::type * = nullptr)
{
    auto last_iter = --t.end();
    for (auto it = t.begin(); it != t.end(); ++it)
    {
        std::cout << *it;
        if (it != last_iter)
        {
            std::cout << ".";
        }
    }
    std::cout << std::endl;
}

// print for integer
template <typename T>
void print_ip(T t, typename std::enable_if<checker::is_integer<T>::value, T>::type * = nullptr)
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

// print for string
template <typename T, typename std::enable_if<std::is_same<std::string, T>::value>::type * = nullptr>
void print_ip(T t)
{
    std::cout << t << std::endl;
}

// print for tuple helper
template <typename TupType, size_t... I>
void print_ip(const TupType &_tup, std::index_sequence<I...>)
{

    (..., (std::cout << (I == 0 ? "" : ".") << std::get<I>(_tup)));
    std::cout << "\n";
}

// print for tuple
template <typename... T>
void print_ip(const std::tuple<T...> &_tup, typename std::enable_if<checker::are_all_same<T...>>::type * = nullptr)
{
    print_ip(_tup, std::make_index_sequence<sizeof...(T)>());
}

int main()
{
    print_ip(int8_t{-1});                           // 255
    print_ip(int16_t{0});                           // 0.0
    print_ip(int32_t{2130706433});                  // 127.0.0.1
    print_ip(int64_t{8875824491850138409});         // 123.45.67.89.101.112.131.41
    print_ip(std::string{"Hello, World!"});         // Hello, World!
    print_ip(std::vector<int>{100, 200, 300, 400}); // 100.200.300.400
    print_ip(std::list<short>{400, 300, 200, 100}); // 400.300.200.100
    print_ip(std::make_tuple(123, 456, 789, 0));    // 123.456.789.0
    return 0;
}