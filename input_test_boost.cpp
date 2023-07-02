#include <iostream>
#include <vector>
#include <string>
#include <boost/program_options.hpp>
#include <boost/filesystem.hpp>

namespace po = boost::program_options;
namespace fs = boost::filesystem;

enum class HashAlgo {
    crc32,
    md5,
    sha1
};

void
parse_args(int argc, char *const *argv, std::vector<fs::path> &dirsToScan, std::vector<fs::path> &dirsToExclude, int &level,
           size_t &size, std::string &mask, size_t &blockSize, HashAlgo &algo);

int main(int argc, char* argv[]) {
    std::vector<fs::path> dirsToScan;
    std::vector<fs::path> dirsToExclude;
    int level = 0;
    std::size_t size = 0;
    std::string mask;
    std::size_t blockSize = 0;
    HashAlgo algo;

    parse_args(argc, argv, dirsToScan, dirsToExclude, level, size, mask, blockSize, algo);

    // Print the parsed values for testing
    for (const auto& dir : dirsToScan) {
        std::cout << "Dir: " << dir << std::endl;
    }
    for (const auto& dir : dirsToExclude) {
        std::cout << "Exclude Dir: " << dir << std::endl;
    }
    std::cout << "Level: " << level << std::endl;
    std::cout << "Size: " << size << std::endl;
    std::cout << "Mask: " << mask << std::endl;
    std::cout << "Block Size: " << blockSize << std::endl;
    std::cout << "Algorithm: ";
    switch (algo) {
        case HashAlgo::crc32:
            std::cout << "CRC32";
            break;
        case HashAlgo::md5:
            std::cout << "MD5";
            break;
        case HashAlgo::sha1:
            std::cout << "SHA1";
            break;
    }
    std::cout << std::endl;

    return 0;
}

void
parse_args(int argc, char *const *argv, std::vector<fs::path> &dirsToScan, std::vector<fs::path> &dirsToExclude, int &level,
           size_t &size, std::string &mask, size_t &blockSize, HashAlgo &algo) {
    po::options_description desc("Allowed options");
    desc.add_options()
            ("dir", boost::program_options::value<std::vector<fs::path>>()->multitoken()->composing(), "Directories to scan")
            ("exclude", boost::program_options::value<std::vector<fs::path>>()->multitoken()->composing(), "Directories to exclude from scanning")
            ("level", po::value<int>(), "Level (0 or 1)")
            ("size", boost::program_options::value<std::size_t>(), "Size")
            ("mask", boost::program_options::value<std::string>(), "Mask")
            ("block_size", boost::program_options::value<std::size_t>(), "Block Size")
            ("algo", boost::program_options::value<std::string>(), "Algorithm (crc32, md5, sha1)");

    po::variables_map vm;
    po::store(po::parse_command_line(argc, argv, desc), vm);
    po::notify(vm);

    if (vm.count("dir")) {
        dirsToScan = vm["dir"].as<std::vector<fs::path>>();
    }
    if (vm.count("exclude")) {
        dirsToExclude = vm["exclude"].as<std::vector<fs::path>>();
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
            std::cerr << "Invalid algorithm specified." << std::endl;
        }
    }
}
