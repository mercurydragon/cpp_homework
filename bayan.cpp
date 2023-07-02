#include <boost/range.hpp>
#include <boost/filesystem.hpp>
#include <iostream>
#include <map>
#include <fstream>
#include <boost/crc.hpp>
#include <boost/uuid/detail/md5.hpp>
#include <boost/uuid/detail/sha1.hpp>
#include <boost/algorithm/hex.hpp>
#include <boost/program_options.hpp>


using namespace boost::filesystem;
namespace po = boost::program_options;

struct HashedFiles {
    std::string hash;
    std::streamsize file_size;
    std::streamoff current_position;
    std::vector<std::string> files;

    explicit HashedFiles(std::streamsize my_size) : current_position(0) {
        file_size = my_size;
    }

    explicit HashedFiles(std::streamsize my_size, const std::string &file) : current_position(0) {
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


std::string get_md5(const char buffer[], size_t size) {
    boost::uuids::detail::md5 hash;
    boost::uuids::detail::md5::digest_type digest;

    hash.process_bytes(buffer, size);
    hash.get_digest(digest);

    // Convert to string
    const auto charDigest = reinterpret_cast<const char *>(&digest);
    std::string result;
    boost::algorithm::hex(charDigest, charDigest + sizeof(boost::uuids::detail::md5::digest_type),
                          std::back_inserter(result));
    return result;
}

std::string get_sha1(const char buffer[], size_t size) {
    boost::uuids::detail::sha1 hash;
    boost::uuids::detail::sha1::digest_type digest;

    hash.process_bytes(buffer, size);
    hash.get_digest(digest);

    // Convert to string
    const auto charDigest = reinterpret_cast<const char *>(&digest);
    std::string result;
    boost::algorithm::hex(charDigest, charDigest + sizeof(boost::uuids::detail::sha1::digest_type),
                          std::back_inserter(result));
    return result;
}

enum HashAlgo {
    crc32,
    md5,
    sha1
};

std::string calc_hash(HashAlgo &algo, const char buffer[], size_t size) {
    switch (algo) {
        case crc32  :
            return get_crc32(buffer, size);
        case md5:
            return get_md5(buffer, size);
        case sha1:
            return get_sha1(buffer, size);
        default:
            throw std::invalid_argument("Unknown hash algo");
    }
}

std::string
read_batch(const std::string &file_path, HashAlgo &algo, size_t buffer_size, std::streamoff current_position) {
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
        std::cout << '[' << i.file_size << "-" << i.hash << "] = ";
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
bool
calc_map(const std::vector<HashedFiles> &left, std::vector<HashedFiles> &right, size_t buffer_size, HashAlgo algo) {
    bool last_iteration = true;
    for (const HashedFiles &i: left) {
        if (i.files.size() < 2) {
            continue;
        }
        if (i.current_position > i.file_size) {
            right.push_back(i);
            continue;
        }
        last_iteration = false;
        auto position = i.current_position + (long) buffer_size;
        for (const std::string &j: i.files) {
            std::cout << j << ", ";
            auto hash = read_batch(j, algo, buffer_size, i.current_position);
            std::string new_hash = i.hash + hash;
            auto current_hf_iter = std::find_if(right.begin(), right.end(), [&](const auto &item) {
                return (item.file_size == i.file_size && item.hash == new_hash && item.current_position == position);
            });
            if (current_hf_iter == right.end()) {
                auto hf = HashedFiles(i.file_size, j);
                hf.hash = new_hash;
                hf.current_position = position;
                right.push_back(hf);
            } else {
                current_hf_iter->files.push_back(j);
            }
        }
    }
    return last_iteration;
}


void
parse_args(int argc, char *const *argv, std::vector<path> &dirsToScan, std::vector<path> &dirsToExclude, int &level,
           size_t &size, std::string &mask, size_t &blockSize, HashAlgo &algo) {
    po::options_description desc("Allowed options");
    desc.add_options()
            ("dir", boost::program_options::value<std::vector<path>>()->multitoken()->composing(),
             "Directories to scan")
            ("exclude", boost::program_options::value<std::vector<path>>()->multitoken()->composing(),
             "Directories to exclude from scanning")
            ("level", po::value<int>(), "Level (0 or 1)")
            ("size", boost::program_options::value<std::size_t>(), "Size")
            ("mask", boost::program_options::value<std::string>(), "Mask")
            ("block_size", boost::program_options::value<std::size_t>(), "Block Size")
            ("algo", boost::program_options::value<std::string>(), "Algorithm (crc32, md5, sha1)");

    po::variables_map vm;
    po::store(po::parse_command_line(argc, argv, desc), vm);
    po::notify(vm);

    if (vm.count("dir")) {
        dirsToScan = vm["dir"].as<std::vector<path>>();
    }
    if (vm.count("exclude")) {
        dirsToExclude = vm["exclude"].as<std::vector<path>>();
    }
    if (vm.count("level")) {
        level = vm["level"].as<int>();
    }
    if (vm.count("size")) {
        size = vm["size"].as<std::size_t>();
    }
    if (vm.count("mask")) {
        mask = vm["mask"].as<std::string>();
    }
    if (vm.count("block_size")) {
        blockSize = vm["block_size"].as<std::size_t>();
    }
    if (vm.count("algo")) {
        std::string algoStr = vm["algo"].as<std::string>();
        if (algoStr == "crc32") {
            algo = HashAlgo::crc32;
        } else if (algoStr == "md5") {
            algo = HashAlgo::md5;
        } else if (algoStr == "sha1") {
            algo = HashAlgo::sha1;
        } else {
            throw std::invalid_argument("Unknown hash algo");
        }
    }
}

int main(int argc, char* argv[]) {
    size_t buffer_size = 100;

    std::vector<path> dirs_to_scan;
    std::vector<path> dirs_to_exclude;
    int level = 0;
    std::size_t min_size = 0;
    std::string mask;
    std::size_t blockSize = 0;
    HashAlgo algo;

    parse_args(argc, argv, dirs_to_scan, dirs_to_exclude, level, min_size, mask, blockSize, algo);


    std::vector<HashedFiles> sizes_vector, result;
    for (const auto& dir : dirs_to_scan) {
        recursive_directory_iterator it(dir), end;
        for (auto &entry: boost::make_iterator_range(it, end)) {
            if (is_regular_file(entry)) {
                auto size = (long int) file_size(entry.path());
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
    }
    auto stop = false;
    while (!stop) {
        std::cout << "it\n";
        stop = calc_map(sizes_vector, result, buffer_size, sha1);
        sizes_vector.erase(sizes_vector.begin(), sizes_vector.end());
        sizes_vector.swap(result);

    }
    print_map(sizes_vector);
    print_map(result);
    sizes_vector.clear();
    return 0;
}


/* сделать выбор папки и нагенерить файлов для тестлов +
 * вторая итерация +
 * выход из цикла +
 * отладка всего
 *
 * добавление параметров
 * */

