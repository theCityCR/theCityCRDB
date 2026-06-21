#include "theCityCRDB/common/value.hpp"

#include <algorithm>
#include <cctype>
#include <sstream>

namespace theCityCRDB {

Value::Value(std::int64_t value) : data_(value) {}

Value::Value(int value) : data_(static_cast<std::int64_t>(value)) {}

Value::Value(double value) : data_(value) {}

Value::Value(std::string value) : data_(std::move(value)) {}

ColumnType Value::type() const {
    if (std::holds_alternative<std::int64_t>(data_)) {
        return ColumnType::Int;
    }
    if (std::holds_alternative<double>(data_)) {
        return ColumnType::Double;
    }
    return ColumnType::String;
}

const ValueData& Value::data() const noexcept {
    return data_;
}

std::string Value::toString() const {
    return std::visit(
        [](const auto& item) {
            std::ostringstream stream;
            stream << item;
            return stream.str();
        },
        data_);
}

bool operator<(const Value& lhs, const Value& rhs) {
    if (lhs.type() != rhs.type()) {
        return static_cast<int>(lhs.type()) < static_cast<int>(rhs.type());
    }
    return lhs.data_ < rhs.data_;
}

std::ostream& operator<<(std::ostream& os, const Value& value) {
    os << value.toString();
    return os;
}

std::string toString(ColumnType type) {
    switch (type) {
        case ColumnType::Int:
            return "INT";
        case ColumnType::Double:
            return "DOUBLE";
        case ColumnType::String:
            return "STRING";
    }
    return "UNKNOWN";
}

std::optional<ColumnType> columnTypeFromString(std::string_view text) {
    std::string normalized{text};
    std::ranges::transform(normalized, normalized.begin(), [](unsigned char ch) {
        return static_cast<char>(std::toupper(ch));
    });

    if (normalized == "INT") {
        return ColumnType::Int;
    }
    if (normalized == "DOUBLE") {
        return ColumnType::Double;
    }
    if (normalized == "STRING") {
        return ColumnType::String;
    }
    return std::nullopt;
}

}  // namespace theCityCRDB
