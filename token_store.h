#pragma once
#include <string>


namespace token_store {
  std::string load_refresh_token();
  bool save_refresh_token(const std::string& token);
  void clear();
}
