#pragma once

#include <string>

namespace atlasdb {

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

}  // namespace atlasdb
