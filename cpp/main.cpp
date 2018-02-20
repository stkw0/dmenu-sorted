#include <algorithm>
#include <cstdlib>
#include <cstring>
#include <experimental/filesystem>
#include <iostream>
#include <string>
#include <utility>
#include <vector>

#include <unistd.h>

#include "spawn.hpp"

namespace fs = std::experimental::filesystem;
using item = std::pair<std::string, int>;

// TODO: Handle errors

std::vector<const char*> paths() {
    using namespace std;

    vector<const char*> paths;
    auto env_path = getenv("PATH");

    for(auto p = strtok(env_path, ":"); p; p = strtok(NULL, ":"))
        paths.emplace_back(p);

    return paths;
}

bool is_executable(const auto& p) {
    return (fs::status(p).permissions() & (fs::perms::others_exec | fs::perms::group_exec |
                                           fs::perms::owner_exec)) != fs::perms::none;
}

std::vector<std::string> commands() {
    static std::vector<std::string> commands;
    if(commands.size() > 0) return commands;

    for(auto& path : paths()) try {
            for(auto& e : fs::directory_iterator(path)) {
                if(is_executable(e))
                    commands.emplace_back(e.path().filename().c_str());
            }
        } catch(...) {
            // Ignore errors for now
        }

    std::sort(commands.begin(), commands.end());
    commands.erase(std::unique(commands.begin(), commands.end()), commands.end());

    return commands;
}

std::vector<item> get_items(const auto& cache_file) {
    static std::vector<item> items_list;
    if(items_list.size() > 0) return items_list;

    if(std::ifstream cache(cache_file); cache.is_open()) {
        do {
            std::string cmd;
            cache >> cmd;
            if(cmd == "") break;

            int n;
            cache >> n;
            items_list.emplace_back(std::make_pair(cmd, n));
        } while(cache.good());

        std::sort(items_list.begin(), items_list.end(), [](const item& a, const item& b) {
            return a.second > b.second;
        });
    } else {
        for(auto& e : commands()) items_list.emplace_back(std::make_pair(e, 0));
    }

    return items_list;
}

void update_items_cache(const auto& cache_file, const auto& cmd) {
    std::ofstream cache(cache_file, std::ios::trunc);
    for(auto& e : get_items(cache_file)) {
        if(e.first == cmd) e.second++;

        cache << e.first << " " << e.second << "\n";
    }
}

void execute_cmd(const auto& cmd) {
    const char* argv[] = {cmd.c_str(), (const char*)0};
    spawn(argv, true);
}

int main(int, char* argv[]) {
    std::string home = getenv("HOME");
    auto cache_file = home + "/.cache/dmenu_sorted";

    argv[0] = const_cast<char*>("/usr/bin/dmenu");

    spawn dmenu(argv);

    for(auto& e : get_items(cache_file)) dmenu.stdin << e.first << "\n";
    dmenu.send_eof();
    dmenu.wait();

    std::string cmd;
    getline(dmenu.stdout, cmd);

    if(cmd=="") std::exit(1);

    update_items_cache(cache_file, cmd);
    execute_cmd(cmd);
}
