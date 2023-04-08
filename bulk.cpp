#include <iostream>
#include <forward_list>
#include <fstream>


std::string format_commnads(std::forward_list<std::string> &bulk) {
    std::string result;
    for (auto it = bulk.begin(); it != bulk.end(); ++it) {
        result.insert(0, *it);
        if (std::next(it) != bulk.end()) {
            result.insert(0, ", ");
        }
    }
    result.insert(0, "bulk: ");
    return result;
}

void out_to_console(const std::string &cmd_str) { std::cout << cmd_str << std::endl; }

void save_file(const std::string &cmd_str, long timestamp) {
    auto file_name = "bulk" + std::to_string(timestamp) + ".log";
    std::ofstream log_file(file_name);
    log_file << cmd_str;
    log_file.close();
}

void process_commands(std::forward_list<std::string> &bulk, long timestamp) {
    if (std::distance(bulk.begin(), bulk.end()) == 0) {
        return;
    }
    std::string cmd_str = format_commnads(bulk);
    save_file(cmd_str, timestamp);
    out_to_console(cmd_str);
    bulk.clear();
}

void run_command_processing(int n) {
    auto block_count = 0;
    long tmstmp = 0;
    std::string cmd;
    std::forward_list<std::string> bulk;
    while (std::getline(std::cin, cmd)) {
        if (cmd == "{") {
            if (block_count == 0) {
                process_commands(bulk, tmstmp);
            }
            block_count++;
        } else if (cmd == "}") {
            if (block_count > 0) {
                block_count--;
                if (block_count == 0) {
                    // блок закрылся выводим блок
                    process_commands(bulk, tmstmp);
                }
            }
        } else {
            if (bulk.empty()) {
                tmstmp = time(nullptr);
            }
            bulk.push_front(cmd);
            if (block_count == 0 && std::distance(bulk.begin(), bulk.end()) == n) {
                process_commands(bulk, tmstmp);
            }
        }

    }
    if (block_count == 0) {
        process_commands(bulk, 0);
    }
}

int main(int argc, char *argv[]) {
    auto n = 0;
    if (argc != 2) {
        throw std::invalid_argument("You should pass only one arg");
    }
    try {
        n = std::stoi(argv[1]);
    }
    catch (std::exception const &) {
        throw std::invalid_argument("You should pass a number");
    };
    if (n <= 0) {
        throw std::invalid_argument("You should pass a natural number");
    }

    run_command_processing(n);
    return 0;
}