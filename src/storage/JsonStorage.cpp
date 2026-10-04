#include "storage/JsonStorage.hpp"

#include <fstream>
#include <iterator>
#include <sstream>
#include <utility>

namespace lms::storage {

using json::Value;

JsonStorage::JsonStorage(std::string path) : path_(std::move(path)) {}

const std::string& JsonStorage::path() const { return path_; }

model::Book JsonStorage::bookFromJson(const Value& v) {
    model::Book b;
    b.id = v.at("id").asString();
    b.title = v.at("title").asString();
    b.author = v.at("author").asString();
    b.isbn = v.at("isbn").asString();
    b.totalCopies = static_cast<int>(v.at("totalCopies").asNumber());
    b.availableCopies = static_cast<int>(v.at("availableCopies").asNumber());
    return b;
}

model::Reader JsonStorage::readerFromJson(const Value& v) {
    model::Reader r;
    r.id = v.at("id").asString();
    r.name = v.at("name").asString();
    r.phone = v.at("phone").asString();
    return r;
}

model::BorrowRecord JsonStorage::recordFromJson(const Value& v) {
    model::BorrowRecord r;
    r.id = v.at("id").asString();
    r.bookId = v.at("bookId").asString();
    r.readerId = v.at("readerId").asString();
    r.borrowDate = v.at("borrowDate").asString();
    r.dueDate = v.at("dueDate").asString();
    r.returnDate = v.at("returnDate").asString();
    return r;
}

Value JsonStorage::bookToJson(const model::Book& b) {
    Value o(Value::Type::Object);
    o["id"] = Value(b.id);
    o["title"] = Value(b.title);
    o["author"] = Value(b.author);
    o["isbn"] = Value(b.isbn);
    o["totalCopies"] = Value(b.totalCopies);
    o["availableCopies"] = Value(b.availableCopies);
    return o;
}

Value JsonStorage::readerToJson(const model::Reader& r) {
    Value o(Value::Type::Object);
    o["id"] = Value(r.id);
    o["name"] = Value(r.name);
    o["phone"] = Value(r.phone);
    return o;
}

Value JsonStorage::recordToJson(const model::BorrowRecord& r) {
    Value o(Value::Type::Object);
    o["id"] = Value(r.id);
    o["bookId"] = Value(r.bookId);
    o["readerId"] = Value(r.readerId);
    o["borrowDate"] = Value(r.borrowDate);
    o["dueDate"] = Value(r.dueDate);
    o["returnDate"] = Value(r.returnDate);
    return o;
}

bool JsonStorage::load(std::vector<model::Book>& books,
                       std::vector<model::Reader>& readers,
                       std::vector<model::BorrowRecord>& records) {
    std::ifstream in(path_, std::ios::binary);
    if (!in) return false;  // 文件不存在 -> 全新开始

    std::string text((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
    Value root = Value::parse(text);  // 损坏时抛 JsonError

    const Value& bookArr = root.at("books");
    const Value& readerArr = root.at("readers");
    const Value& recordArr = root.at("records");
    for (const auto& item : bookArr.arr()) books.push_back(bookFromJson(item));
    for (const auto& item : readerArr.arr()) readers.push_back(readerFromJson(item));
    for (const auto& item : recordArr.arr()) records.push_back(recordFromJson(item));
    return true;
}

bool JsonStorage::save(const std::vector<model::Book>& books,
                       const std::vector<model::Reader>& readers,
                       const std::vector<model::BorrowRecord>& records) {
    Value root(Value::Type::Object);
    Value bookArr(Value::Type::Array);
    Value readerArr(Value::Type::Array);
    Value recordArr(Value::Type::Array);
    for (const auto& b : books) bookArr.push(bookToJson(b));
    for (const auto& r : readers) readerArr.push(readerToJson(r));
    for (const auto& r : records) recordArr.push(recordToJson(r));
    root["books"] = std::move(bookArr);
    root["readers"] = std::move(readerArr);
    root["records"] = std::move(recordArr);

    try {
        root.writeFile(path_, /*pretty=*/true);
        return true;
    } catch (const json::JsonError&) {
        return false;
    }
}

}  // namespace lms::storage
