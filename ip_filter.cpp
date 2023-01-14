#include <cassert>
#include <cstdlib>
#include <iostream>
#include <string>
#include <vector>
#include <algorithm>

std::vector<std::string> split(const std::string &str, char d)
{
    std::vector<std::string> r;

    std::string::size_type start = 0;
    std::string::size_type stop = str.find_first_of(d);
    while (stop != std::string::npos)
    {
        r.push_back(str.substr(start, stop - start));

        start = stop + 1;
        stop = str.find_first_of(d, start);
    }

    r.push_back(str.substr(start));

    return r;
}

std::vector<int> split_ip(const std::string &str)
{
    auto d = '.';
    std::vector<int> r;

    std::string::size_type start = 0;
    std::string::size_type stop = str.find_first_of(d);
    while (stop != std::string::npos)
    {
        auto ip_byte = str.substr(start, stop - start);
        r.push_back(stoi(ip_byte));

        start = stop + 1;
        stop = str.find_first_of(d, start);
    }

    r.push_back(stoi(str.substr(start)));

    return r;
}

void print_ip_pool(std::vector<std::vector<int>> ip_pool)
{
    for (std::vector<std::vector<int>>::const_iterator ip = ip_pool.cbegin(); ip != ip_pool.cend(); ++ip)
    {

        for (std::vector<int>::const_iterator ip_part = ip->cbegin(); ip_part != ip->cend(); ++ip_part)
        {
            if (ip_part != ip->cbegin())
            {
                std::cout << ".";
            }
            std::cout << *ip_part;
        }
        std::cout << std::endl;
    }
}

void filter_ips(std::vector<std::vector<int>> pool, bool (*condition)(std::vector<int>))
{
    decltype(pool) filtered;
    std::copy_if(pool.begin(), pool.end(), std::back_inserter(filtered), condition);
    print_ip_pool(filtered);
}

int main(int argc, char const *argv[])
{
    try
    {
        std::vector<std::vector<int>> ip_pool;

        for (std::string line; std::getline(std::cin, line);)
        {
            if (line == "")
            {
                break;
            }
            std::vector<std::string> v = split(line, '\t');
            ip_pool.push_back(split_ip(v.at(0)));
        }

        // reverse lexicographically sort
        sort(ip_pool.begin(), ip_pool.end(),
             [](const std::vector<int> &a, const std::vector<int> &b)
             {
                 return a[0] > b[0] ||
                        (a[0] == b[0] && a[1] > b[1]) ||
                        (a[0] == b[0] && a[1] == b[1] && a[2] > b[2]) ||
                        (a[0] == b[0] && a[1] == b[1] && a[2] == b[2] && a[3] > b[3]);
             });
        print_ip_pool(ip_pool);

        // filter by first byte and output
        filter_ips(ip_pool, [](std::vector<int> i)
                   { return i[0] == 1; });

        // filter by first and second bytes and output
        filter_ips(ip_pool, [](std::vector<int> i)
                   { return i[0] == 46 && i[1] == 70; });

        // filter by any byte and output
        filter_ips(ip_pool, [](std::vector<int> i)
                   { return i[0] == 46 || i[1] == 46 || i[2] == 46 || i[3] == 46; });
    }

    catch (const std::exception &e)
    {
        std::cerr << e.what() << std::endl;
    }

    (void)argc;
    (void)argv;
    return 0;
}
