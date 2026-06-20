#pragma once

#include "atlasdb/parser/token.hpp"

#include <string_view>
#include <vector>

namespace atlasdb {

class Tokenizer {
public:
    [[nodiscard]] std::vector<Token> tokenize(std::string_view sql) const;
};

}  // namespace atlasdb
