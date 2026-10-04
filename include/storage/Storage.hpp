#pragma once

#include <string>
#include <vector>

#include "model/Book.hpp"
#include "model/BorrowRecord.hpp"
#include "model/Reader.hpp"

namespace lms::storage {

// 持久化存储抽象接口：与具体格式解耦（当前实现为 JSON 文件）
class Storage {
public:
    virtual ~Storage() = default;

    // 加载全部数据；文件不存在返回 false（视为全新开始）
    // 文件存在但内容损坏时抛出 json::JsonError
    virtual bool load(std::vector<model::Book>& books,
                      std::vector<model::Reader>& readers,
                      std::vector<model::BorrowRecord>& records) = 0;

    // 保存全部数据，失败返回 false
    virtual bool save(const std::vector<model::Book>& books,
                      const std::vector<model::Reader>& readers,
                      const std::vector<model::BorrowRecord>& records) = 0;

    virtual const std::string& path() const = 0;
};

}  // namespace lms::storage
