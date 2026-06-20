#include "atlasdb/parser/parser.hpp"

#include "atlasdb/parser/tokenizer.hpp"

#include <algorithm>
#include <charconv>
#include <cctype>
#include <stdexcept>

namespace atlasdb {
namespace {

bool equalsIgnoreCase(std::string_view lhs, std::string_view rhs) {
    return lhs.size() == rhs.size() &&
           std::ranges::equal(lhs, rhs, [](char left, char right) {
               return std::toupper(static_cast<unsigned char>(left)) ==
                      std::toupper(static_cast<unsigned char>(right));
           });
}

std::int64_t parseInt(std::string_view text) {
    std::int64_t result{};
    const auto [ptr, ec] = std::from_chars(text.data(), text.data() + text.size(), result);
    if (ec != std::errc{} || ptr != text.data() + text.size()) {
        throw std::runtime_error("invalid integer literal");
    }
    return result;
}

}  // namespace

Query Parser::parse(std::string_view sql) {
    const auto tokens = Tokenizer{}.tokenize(sql);
    return parse(tokens);
}

Query Parser::parse(std::span<const Token> tokens) {
    tokens_ = tokens;
    current_ = 0;

    if (match(TokenType::Identifier, "CREATE")) {
        if (match(TokenType::Identifier, "DATABASE")) {
            return parseCreateDatabase();
        }
        if (match(TokenType::Identifier, "TABLE")) {
            return parseCreateTable();
        }
        if (match(TokenType::Identifier, "INDEX")) {
            return parseCreateIndex();
        }
        throw std::runtime_error("expected DATABASE, TABLE, or INDEX after CREATE");
    }
    if (match(TokenType::Identifier, "INSERT")) {
        return parseInsert();
    }
    if (match(TokenType::Identifier, "SELECT")) {
        return parseSelect();
    }
    if (match(TokenType::Identifier, "UPDATE")) {
        return parseUpdate();
    }
    if (match(TokenType::Identifier, "DELETE")) {
        return parseDelete();
    }
    if (match(TokenType::Identifier, "SAVE")) {
        expect(TokenType::Identifier, "DATABASE");
        return SaveDatabase{};
    }
    if (match(TokenType::Identifier, "LOAD")) {
        expect(TokenType::Identifier, "DATABASE");
        return LoadDatabase{};
    }
    if (match(TokenType::Identifier, "EXIT")) {
        return Exit{};
    }

    throw std::runtime_error("unsupported SQL command");
}

const Token& Parser::peek() const {
    if (current_ >= tokens_.size()) {
        throw std::runtime_error("parser read past end of token stream");
    }
    return tokens_[current_];
}

const Token& Parser::advance() {
    const Token& token = peek();
    if (token.type != TokenType::End) {
        ++current_;
    }
    return token;
}

bool Parser::match(TokenType type, std::string_view lexeme) {
    const Token& token = peek();
    if (token.type != type) {
        return false;
    }
    if (!lexeme.empty() && !equalsIgnoreCase(token.lexeme, lexeme)) {
        return false;
    }
    (void)advance();
    return true;
}

void Parser::expect(TokenType type, std::string_view lexeme) {
    if (!match(type, lexeme)) {
        throw std::runtime_error("unexpected token");
    }
}

CreateDatabase Parser::parseCreateDatabase() {
    const auto name = advance();
    if (name.type != TokenType::Identifier) {
        throw std::runtime_error("expected database name");
    }
    return {name.lexeme};
}

CreateTable Parser::parseCreateTable() {
    const auto table = advance();
    if (table.type != TokenType::Identifier) {
        throw std::runtime_error("expected table name");
    }
    expect(TokenType::LeftParen);

    std::vector<Column> columns;
    do {
        const auto columnName = advance();
        if (columnName.type != TokenType::Identifier) {
            throw std::runtime_error("expected column name");
        }
        const auto typeName = advance();
        if (typeName.type != TokenType::Identifier) {
            throw std::runtime_error("expected column type");
        }
        auto type = columnTypeFromString(typeName.lexeme);
        if (!type) {
            throw std::runtime_error("unsupported column type");
        }
        columns.push_back({columnName.lexeme, *type});
    } while (match(TokenType::Comma));

    expect(TokenType::RightParen);
    return {table.lexeme, std::move(columns)};
}

CreateIndex Parser::parseCreateIndex() {
    const auto index = advance();
    if (index.type != TokenType::Identifier) {
        throw std::runtime_error("expected index name");
    }
    expect(TokenType::Identifier, "ON");
    const auto table = advance();
    if (table.type != TokenType::Identifier) {
        throw std::runtime_error("expected indexed table name");
    }
    expect(TokenType::LeftParen);
    const auto column = advance();
    if (column.type != TokenType::Identifier) {
        throw std::runtime_error("expected indexed column name");
    }
    expect(TokenType::RightParen);
    return {index.lexeme, table.lexeme, column.lexeme};
}

Insert Parser::parseInsert() {
    expect(TokenType::Identifier, "INTO");
    const auto table = advance();
    if (table.type != TokenType::Identifier) {
        throw std::runtime_error("expected table name");
    }
    expect(TokenType::Identifier, "VALUES");
    expect(TokenType::LeftParen);
    std::vector<Value> values;
    do {
        values.push_back(parseValue());
    } while (match(TokenType::Comma));
    expect(TokenType::RightParen);
    return {table.lexeme, std::move(values)};
}

Select Parser::parseSelect() {
    std::vector<std::string> columns;
    if (match(TokenType::Star)) {
        columns.emplace_back("*");
    } else {
        do {
            const auto column = advance();
            if (column.type != TokenType::Identifier) {
                throw std::runtime_error("expected selected column");
            }
            columns.push_back(column.lexeme);
        } while (match(TokenType::Comma));
    }

    expect(TokenType::Identifier, "FROM");
    const auto table = advance();
    if (table.type != TokenType::Identifier) {
        throw std::runtime_error("expected table name");
    }

    std::optional<Predicate> where;
    std::optional<std::size_t> limit;
    if (match(TokenType::Identifier, "WHERE")) {
        where = parsePredicate();
    }
    if (match(TokenType::Identifier, "LIMIT")) {
        const auto count = advance();
        if (count.type != TokenType::Number) {
            throw std::runtime_error("expected numeric limit");
        }
        limit = static_cast<std::size_t>(parseInt(count.lexeme));
    }
    return {table.lexeme, std::move(columns), std::move(where), limit};
}

Update Parser::parseUpdate() {
    const auto table = advance();
    if (table.type != TokenType::Identifier) {
        throw std::runtime_error("expected table name");
    }
    expect(TokenType::Identifier, "SET");
    const auto column = advance();
    if (column.type != TokenType::Identifier) {
        throw std::runtime_error("expected update column");
    }
    expect(TokenType::Equal);
    auto value = parseValue();
    std::optional<Predicate> where;
    if (match(TokenType::Identifier, "WHERE")) {
        where = parsePredicate();
    }
    return {table.lexeme, column.lexeme, std::move(value), std::move(where)};
}

Delete Parser::parseDelete() {
    expect(TokenType::Identifier, "FROM");
    const auto table = advance();
    if (table.type != TokenType::Identifier) {
        throw std::runtime_error("expected table name");
    }
    std::optional<Predicate> where;
    if (match(TokenType::Identifier, "WHERE")) {
        where = parsePredicate();
    }
    return {table.lexeme, std::move(where)};
}

Predicate Parser::parsePredicate() {
    const auto column = advance();
    if (column.type != TokenType::Identifier) {
        throw std::runtime_error("expected predicate column");
    }
    ComparisonOperator op{};
    if (match(TokenType::Equal)) {
        op = ComparisonOperator::Equal;
    } else if (match(TokenType::Greater)) {
        op = ComparisonOperator::Greater;
    } else if (match(TokenType::Less)) {
        op = ComparisonOperator::Less;
    } else {
        throw std::runtime_error("expected comparison operator");
    }
    return {column.lexeme, op, parseValue()};
}

Value Parser::parseValue() {
    const auto token = advance();
    if (token.type == TokenType::String) {
        return Value{token.lexeme};
    }
    if (token.type == TokenType::Number) {
        if (token.lexeme.find('.') != std::string::npos) {
            return Value{std::stod(token.lexeme)};
        }
        return Value{parseInt(token.lexeme)};
    }
    throw std::runtime_error("expected literal value");
}

}  // namespace atlasdb
