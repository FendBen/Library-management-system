#pragma once

#include <string>

namespace lms::model {

// 读者实体
struct Reader {
    std::string id;    // 如 R001
    std::string name;  // 姓名
    std::string phone; // 联系电话（可为空）
};

}  // namespace lms::model
