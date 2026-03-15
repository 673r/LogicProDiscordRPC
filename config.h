#ifndef CONFIG_H
#define CONFIG_H

#include <string>

namespace config {
    int get_update_interval();
    void set_update_interval(int seconds);
    
    std::string get_custom_state_prefix();
    void set_custom_state_prefix(const std::string& prefix);
    
    bool get_show_elapsed_time();
    void set_show_elapsed_time(bool show);
    
    void load();
    void save();
}

#endif
