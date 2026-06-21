#pragma once

#include "theCityCRDB/parser/token.hpp"

#include <string_view>
#include <vector>

namespace theCityCRDB {

class Tokenizer {
  public:
    [[nodiscard]] std::vector<Token> tokenize(std::string_view sql) const;
};

} // namespace theCityCRDB
