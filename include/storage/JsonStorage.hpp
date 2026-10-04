#pragma once

#include <string>

#include "storage/Storage.hpp"
#include "util/Json.hpp"

namespace lms::storage {

// JSON 文件存储实现
class JsonStorage : public Storage {
public:
    explicit JsonStorage(std::string path);

    bool load(std::vector<model::Book>& books,
              std::vector<model::Reader>& readers,
              std::vector<model::BorrowRecord>& records) override;

    bool save(const std::vector<model::Book>& books,
              const std::vector<model::Reader>& readers,
              const std::vector<model::BorrowRecord>& records) override;

    const std::string& path() const override;

private:
    std::string path_;

    static model::Book bookFromJson(const json::Value& v);
    static model::Reader readerFromJson(const json::Value& v);
    static model::BorrowRecord recordFromJson(const json::Value& v);
    static json::Value bookToJson(const model::Book& b);
    static json::Value readerToJson(const model::Reader& r);
    static json::Value recordToJson(const model::BorrowRecord& r);
};

}  // namespace lms::storage
