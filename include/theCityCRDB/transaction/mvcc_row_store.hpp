#pragma once

#include "theCityCRDB/storage/row.hpp"
#include "theCityCRDB/transaction/transaction_manager.hpp"

#include <map>
#include <optional>
#include <vector>

namespace theCityCRDB {

struct RowVersion {
    TransactionId createdBy{};
    std::optional<TransactionId> deletedBy;
    Row row;
};

class MVCCRowStore {
  public:
    void write(RowId rowId, Row row, TransactionId transactionId);
    void erase(RowId rowId, TransactionId transactionId);
    [[nodiscard]] std::optional<Row> read(RowId rowId, TransactionId readerId) const;
    [[nodiscard]] std::size_t versionCount(RowId rowId) const;

  private:
    std::map<RowId, std::vector<RowVersion>> versions_;
};

} // namespace theCityCRDB
