#pragma once

#include "theCityCRDB/parser/ast.hpp"
#include "theCityCRDB/parser/token.hpp"

#include <span>
#include <string_view>

namespace theCityCRDB {

class Parser {
public:
    [[nodiscard]] Query parse(std::string_view sql);
    [[nodiscard]] Query parse(std::span<const Token> tokens);

private:
    [[nodiscard]] const Token& peek() const;
    [[nodiscard]] const Token& advance();
    [[nodiscard]] bool match(TokenType type, std::string_view lexeme = {});
    void expect(TokenType type, std::string_view lexeme = {});

    [[nodiscard]] CreateDatabase parseCreateDatabase();
    [[nodiscard]] CreateTable parseCreateTable();
    [[nodiscard]] DropTable parseDropTable();
    [[nodiscard]] RenameTable parseRenameTable();
    [[nodiscard]] CreateIndex parseCreateIndex();
    [[nodiscard]] Insert parseInsert();
    [[nodiscard]] Select parseSelect();
    [[nodiscard]] Update parseUpdate();
    [[nodiscard]] Delete parseDelete();
    [[nodiscard]] Predicate parsePredicate();
    [[nodiscard]] Value parseValue();

    std::span<const Token> tokens_;
    std::size_t current_{0};
};

}  // namespace theCityCRDB
