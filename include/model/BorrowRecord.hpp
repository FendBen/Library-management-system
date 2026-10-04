#pragma once

#include <string>

namespace lms::model {

// 借阅记录实体
struct BorrowRecord {
    std::string id;         // 如 BR001
    std::string bookId;     // 图书 ID
    std::string readerId;   // 读者 ID
    std::string borrowDate; // 借出日期 YYYY-MM-DD
    std::string dueDate;    // 应还日期 YYYY-MM-DD
    std::string returnDate; // 实际归还日期，空串表示未归还

    bool returned() const { return !returnDate.empty(); }
};

}  // namespace lms::model
