#include "config.h"
#include <fstream>
#include <sstream>
#include <map>
#include <iostream>
#include <sys/stat.h>

namespace config {
    static int update_interval = 2;
    static std::string custom_state_prefix = "Working on";
    static bool show_elapsed_time = true;
    
    static std::string get_config_path() {
        const char* home = getenv("HOME");
        if (!home) return "";
        return std::string(home) + "/.logicrpc_config";
    }
    
    int get_update_interval() { return update_interval; }
    void set_update_interval(int seconds) { 
        if (seconds >= 1 && seconds <= 60) {
            update_interval = seconds;
        }
    }
    
    std::string get_custom_state_prefix() { return custom_state_prefix; }
    void set_custom_state_prefix(const std::string& prefix) { 
        if (!prefix.empty() && prefix.length() <= 50) {
            custom_state_prefix = prefix;
        }
    }
    
    bool get_show_elapsed_time() { return show_elapsed_time; }
    void set_show_elapsed_time(bool show) { show_elapsed_time = show; }
    
    void load() {
        std::string path = get_config_path();
        if (path.empty()) return;
        
        std::ifstream file(path);
        if (!file.is_open()) return;
        
        std::string line;
        while (std::getline(file, line)) {
            size_t pos = line.find('=');
            if (pos == std::string::npos) continue;
            
            std::string key = line.substr(0, pos);
            std::string value = line.substr(pos + 1);
            
            if (key == "update_interval") {
                try {
                    set_update_interval(std::stoi(value));
                } catch (...) {}
            } else if (key == "custom_state_prefix") {
                set_custom_state_prefix(value);
            } else if (key == "show_elapsed_time") {
                set_show_elapsed_time(value == "true" || value == "1");
            }
        }
        
        std::cout << "⚙️  Configuration loaded\n";
    }
    
    void save() {
        std::string path = get_config_path();
        if (path.empty()) return;
        
        std::ofstream file(path);
        if (!file.is_open()) return;
        
        file << "update_interval=" << update_interval << "\n";
        file << "custom_state_prefix=" << custom_state_prefix << "\n";
        file << "show_elapsed_time=" << (show_elapsed_time ? "true" : "false") << "\n";
        
        chmod(path.c_str(), 0600);
    }
}
