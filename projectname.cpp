#include <cstdio>
#include <array>
#include <string>
#include <chrono>
#include <mutex>

std::string get_logic_project_name();

static std::string cached_project_name;
static std::chrono::steady_clock::time_point last_check;
static std::mutex cache_mutex;
static const int CACHE_DURATION_MS = 1500;

static std::string rtrim_newlines(std::string s) {
    while (!s.empty() && (s.back() == '\n' || s.back() == '\r'))
        s.pop_back();
    return s;
}

static std::string run_cmd(const std::string& cmd) {
    std::array<char, 4096> buf{};
    std::string out;

    FILE* pipe = popen(cmd.c_str(), "r");
    if (!pipe) return "";

    while (fgets(buf.data(), buf.size(), pipe))
        out += buf.data();

    pclose(pipe);
    return rtrim_newlines(out);
}

static std::string strip_suffix(std::string s, const std::string& suffix) {
    if (s.size() >= suffix.size() &&
        s.compare(s.size() - suffix.size(), suffix.size(), suffix) == 0) {
        s.resize(s.size() - suffix.size());
    }
    return s;
}

// Single-quote for the shell so osascript gets the script exactly as written.
static std::string shell_quote(const std::string& s) {
    std::string out;
    out.reserve(s.size() + 2);
    out.push_back('\'');
    for (char c : s) {
        if (c == '\'') out += "'\\''";  // end quote, escape quote, reopen
        else out.push_back(c);
    }
    out.push_back('\'');
    return out;
}

std::string get_logic_project_name() {
    std::lock_guard<std::mutex> lock(cache_mutex);
    
    auto now = std::chrono::steady_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - last_check).count();
    
    if (elapsed < CACHE_DURATION_MS && !cached_project_name.empty()) {
        return cached_project_name;
    }
    
    const char* script = R"applescript(
    if application "Logic Pro" is not running then
        return ""
    end if

    tell application "Logic Pro"
        try
            return name of front document
        on error
            return ""
        end try
    end tell
    )applescript";

    std::string cmd = "osascript -e " + shell_quote(script);
    std::string name = run_cmd(cmd);
    
    if (name.empty()) {
        cached_project_name = "";
    } else {
        cached_project_name = strip_suffix(name, ".logicx");
    }
    
    last_check = now;
    return cached_project_name;
}
