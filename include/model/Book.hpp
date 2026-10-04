#pragma once

#include <string>

namespace lms::model {

// 图书实体
struct Book {
    std::string id;            // 如 B001
    std::string title;         // 书名
    std::string author;        // 作者
    std::string isbn;          // ISBN（可为空）
    int totalCopies = 1;       // 馆藏总册数
    int availableCopies = 1;   // 当前可借册数
};

}  // namespace lms::model
