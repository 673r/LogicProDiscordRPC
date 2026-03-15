#include "token_store.h"
#include <filesystem>
#include <fstream>
#include <cstdlib>

static std::filesystem::path token_path() {
    const char* home = std::getenv("HOME");
    std::filesystem::path p = home ? home : "";
    p /= "Library/Application Support/logicrpc";
    std::filesystem::create_directories(p);
    p /= "refresh_token.txt";
    return p;
}

void token_store::clear() {
    std::filesystem::remove(token_path());
}

namespace token_store {
std::string load_refresh_token() {
    std::ifstream in(token_path());
    if (!in) return "";
    std::string t;
    std::getline(in, t);
    return t;
}

bool save_refresh_token(const std::string& token) {
    std::ofstream out(token_path(), std::ios::trunc);
    if (!out) return false;
    out << token;
    return true;
}
}
