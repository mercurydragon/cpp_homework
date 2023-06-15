#include <boost/range.hpp>
#include <boost/filesystem.hpp>
#include <iostream>
#include <map>
#include <fstream>
#include <boost/crc.hpp>

using namespace boost::filesystem;

struct HashedFiles {
    std::string hash;
    std::streamsize file_size;
    std::streamoff current_position;
    std::vector<std::string> files;

    explicit HashedFiles(std::streamsize my_size) : current_position(0) {
        file_size = my_size;
    }

    explicit HashedFiles(std::streamsize my_size, const std::string& file) : current_position(0) {
        file_size = my_size;
        files.push_back(file);
    }

    bool operator<(const HashedFiles &f) const {
        return file_size < f.file_size;
    }
};


std::string get_crc32(const char buffer[], size_t size) {
    boost::crc_32_type result;
    result.process_bytes(buffer, size);
    return std::to_string(result.checksum());
}

enum HashAlgo {
    crc32
};

std::string calc_hash(HashAlgo &algo, const char buffer[], size_t size) {
    switch (algo) {
        case crc32  :
            return get_crc32(buffer, size);
        default:
            throw std::invalid_argument("Unknown hash algo");
    }
}

std::string read_batch(const std::string &file_path, HashAlgo &algo, size_t buffer_size, std::streamoff current_position) {
    char *buffer = new char[buffer_size];

    std::ifstream fin(file_path);


    fin.seekg(current_position);
    fin.read(buffer, buffer_size);
    size_t count = fin.gcount();
    // If nothing has been read
    if (!count)
        return "";
    auto res = calc_hash(algo, buffer, buffer_size);

    fin.close();
    delete[] buffer;
    return res;
}

void print_map(std::vector<HashedFiles> &m) {
    size_t buffer_size = 1 << 20;
    auto algo = crc32;
    for (const HashedFiles &i: m) {
        std::cout << '[' << i.file_size << "] = ";
        for (const std::string &j: i.files) {
            std::cout << j << ", ";
            read_batch(j, algo, buffer_size, i.current_position);
        }
        std::cout << '\n';
    }
}

// переписать на HashedFiles
/*
 * for k, v in map:
 *  if v <= 1:
 *     remove k
 *     return
 *  if k.current_position <= k.file_size
 *      continue;
 *  for file in v:
 *      position = k.position ++
 *      h = calc_hash(position, file)
 *      ====== вы находитесь здесь ====
 *      if HashedFile(k.size, k.hash+h, k.position) in map
 *        append to vector
 *      else
 *         add new elemet
 *   remove k
 *
 *
 * */
bool calc_map(const std::vector<HashedFiles> &left, std::vector<HashedFiles> &right, size_t buffer_size) {
    for (const HashedFiles &i: left) {
        if (i.files.size() < 2){
            continue;
        }
        if (i.current_position <= i.file_size) {
            right.push_back(i);
        }
        auto position = i.current_position + buffer_size;
        for (const std::string &j: i.files) {
            std::cout << j << ", ";
            hash = read_batch(j, algo, buffer_size, i.current_position);
        }


    }
    return false;
}

//std::map<std::string, std::vector<std::string>> calc_over_map(const std::map<size_t , std::vector<std::string>>& m)
//{
//    auto algo = crc32;
//    for (const auto&[key, value]: m) {
//        std::cout << '[' << key << "] = " ;
//        for (const std::string& i: value) {
//            std::cout << i << ", ";
//            read_batch(i, algo);
//        }
//
//        std::cout << '\n';
//    }
//}



int main() {
    size_t buffer_size = 1 << 20;
    path p = current_path();
    recursive_directory_iterator it(p), end;

    std::vector<HashedFiles> sizes_vector, result;
    for (auto &entry: boost::make_iterator_range(it, end)) {
        if (is_regular_file(entry)) {
            auto size = file_size(entry.path());
            auto map_iter = std::find_if(sizes_vector.begin(), sizes_vector.end(), [&](const auto &item) {
                return (item.file_size == size);
            });
            if (map_iter == sizes_vector.end()) {
                sizes_vector.emplace_back(size, entry.path().native());
            } else {
                map_iter->files.push_back(entry.path().native());
            }
        }
    }
    calc_map(sizes_vector, result, buffer_size);
    print_map(result);
    sizes_vector.clear();
    return 0;
}