#pragma once

#include <string>

namespace theCityCRDB {

enum class TokenType {
    Identifier,
    Number,
    String,
    Comma,
    Semicolon,
    LeftParen,
    RightParen,
    Star,
    Equal,
    Greater,
    Less,
    End,
};

struct Token {
    TokenType type;
    std::string lexeme;
};

}  // namespace theCityCRDB
