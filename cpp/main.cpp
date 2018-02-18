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

std::vector<const char*> paths() {
    using namespace std;

    vector<const char*> paths;
    auto env_path = getenv("PATH");

    for(auto p = strtok(env_path, ":"); p; p = strtok(NULL, ":"))
        paths.emplace_back(p);

    return paths;
}

std::vector<std::string> commands() {
    static std::vector<std::string> commands;
    if(commands.size() > 0) return commands;

    for(auto& path : paths()) try {
            for(auto& e : fs::directory_iterator(path))
                commands.emplace_back(e.path().filename().c_str());
        } catch(...) {
            // Ignore errors for now
        }

    return commands;
}

int main() {
    std::vector<std::pair<std::string, int>> cmd_list;
    std::ifstream cache("/tmp/cache");
    if(cache.is_open()) {
        do {
            std::string cmd;
            cache >> cmd;
            if(cmd == "") break;

            int n;
            cache >> n;
            cmd_list.emplace_back(std::make_pair(cmd, n));
        } while(cache.good());

        cache.close();

        std::sort(cmd_list.begin(), cmd_list.end(),
                  [](std::pair<std::string, int> a, std::pair<std::string, int> b) {
                      return a.second > b.second;
                  });
    } /*else {
        for(auto& e : commands()) cmd_list.emplace_back(std::make_pair(e. 0));
    }*/

    const char* const argv[] = {"/usr/bin/dmenu", (const char*)0};
    spawn dmenu(argv);

    if(cmd_list.size() <= 0)
        for(auto& e : commands()) dmenu.stdin << e << "\n";
    else
        for(auto& e : cmd_list) dmenu.stdin << e.first << "\n";
    dmenu.send_eof();
    dmenu.wait();

    std::string s;
    getline(dmenu.stdout, s);

    auto shell = getenv("SHELL");
    auto p = popen(shell, "w");
    fputs(s.c_str(), p);
    pclose(p);

    std::ifstream r_cache("/tmp/cache");
    if(!r_cache.is_open() || r_cache.get(), !r_cache.good()) {
        r_cache.close();
        std::ofstream w_cache("/tmp/cache");
        for(auto& e : commands()) w_cache << e << " 0\n";
    } else {
        fs::remove(fs::path("/tmp/cache"));
        std::ofstream w_cache("/tmp/cache");
        for(auto& e : cmd_list) {
            if(e.first == s) e.second++;

            w_cache << e.first << " " << e.second << "\n";
        }
    }
}