#include "service/LibraryService.hpp"

#include <algorithm>
#include <cstdio>
#include <utility>

#include "util/Date.hpp"
#include "util/Text.hpp"

namespace lms::service {

namespace {

int idNumber(const std::string& id) {
    std::string digits;
    for (char c : id)
        if (c >= '0' && c <= '9') digits += c;
    return digits.empty() ? 0 : std::stoi(digits);
}

}  // namespace

LibraryService::LibraryService(std::unique_ptr<storage::Storage> storage)
    : storage_(std::move(storage)) {}

bool LibraryService::load() {
    bool ok = storage_->load(books_, readers_, records_);
    if (ok) {
        for (const auto& b : books_) nextBookSeq_ = std::max(nextBookSeq_, idNumber(b.id) + 1);
        for (const auto& r : readers_) nextReaderSeq_ = std::max(nextReaderSeq_, idNumber(r.id) + 1);
        for (const auto& r : records_) nextRecordSeq_ = std::max(nextRecordSeq_, idNumber(r.id) + 1);
    }
    return ok;
}

std::string LibraryService::nextBookId() {
    std::string id;
    do {
        char buf[16];
        std::snprintf(buf, sizeof(buf), "B%03d", nextBookSeq_++);
        id = buf;
    } while (findBook(id) != nullptr);
    return id;
}

std::string LibraryService::nextReaderId() {
    std::string id;
    do {
        char buf[16];
        std::snprintf(buf, sizeof(buf), "R%03d", nextReaderSeq_++);
        id = buf;
    } while (findReader(id) != nullptr);
    return id;
}

std::string LibraryService::nextRecordId() {
    std::string id;
    bool collide = false;
    do {
        char buf[16];
        std::snprintf(buf, sizeof(buf), "BR%03d", nextRecordSeq_++);
        id = buf;
        collide = false;
        for (const auto& r : records_)
            if (r.id == id) { collide = true; break; }
    } while (collide);
    return id;
}

void LibraryService::persist() { storage_->save(books_, readers_, records_); }

bool LibraryService::save() { return storage_->save(books_, readers_, records_); }

// ---------- 图书 ----------

std::string LibraryService::addBook(const std::string& title,
                                    const std::string& author,
                                    const std::string& isbn, int totalCopies,
                                    std::string& err) {
    err.clear();
    std::string t = util::trim(title);
    std::string a = util::trim(author);
    std::string i = util::trim(isbn);
    if (t.empty()) { err = "书名不能为空"; return {}; }
    if (a.empty()) { err = "作者不能为空"; return {}; }
    if (totalCopies < 1) { err = "册数必须大于等于 1"; return {}; }
    if (!i.empty()) {
        for (const auto& b : books_)
            if (b.isbn == i) { err = "ISBN 已存在：" + i; return {}; }
    }
    model::Book b;
    b.id = nextBookId();
    b.title = t;
    b.author = a;
    b.isbn = i;
    b.totalCopies = totalCopies;
    b.availableCopies = totalCopies;
    books_.push_back(std::move(b));
    persist();
    return books_.back().id;
}

bool LibraryService::removeBook(const std::string& bookId, std::string& err) {
    err.clear();
    auto it = std::find_if(books_.begin(), books_.end(),
                           [&](const model::Book& b) { return b.id == bookId; });
    if (it == books_.end()) { err = "图书不存在：" + bookId; return false; }
    if (it->availableCopies < it->totalCopies) {
        err = "该书还有未归还的借出记录，无法删除";
        return false;
    }
    books_.erase(it);
    persist();
    return true;
}

bool LibraryService::updateBook(const std::string& bookId,
                                const std::string& title,
                                const std::string& author,
                                const std::string& isbn, int totalCopies,
                                std::string& err) {
    err.clear();
    auto it = std::find_if(books_.begin(), books_.end(),
                           [&](const model::Book& b) { return b.id == bookId; });
    if (it == books_.end()) { err = "图书不存在：" + bookId; return false; }
    std::string t = util::trim(title);
    std::string a = util::trim(author);
    std::string i = util::trim(isbn);
    if (t.empty()) { err = "书名不能为空"; return false; }
    if (a.empty()) { err = "作者不能为空"; return false; }
    if (totalCopies < 1) { err = "册数必须大于等于 1"; return false; }
    if (!i.empty()) {
        for (const auto& b : books_)
            if (b.id != bookId && b.isbn == i) { err = "ISBN 已存在：" + i; return false; }
    }
    int borrowed = it->totalCopies - it->availableCopies;
    if (totalCopies < borrowed) {
        err = "新册数不能少于当前借出册数（" + std::to_string(borrowed) + "）";
        return false;
    }
    it->title = t;
    it->author = a;
    it->isbn = i;
    it->totalCopies = totalCopies;
    it->availableCopies = totalCopies - borrowed;
    persist();
    return true;
}

std::vector<const model::Book*> LibraryService::searchBooks(
    const std::string& keyword) const {
    std::string kw = util::trim(keyword);
    std::vector<const model::Book*> out;
    for (const auto& b : books_) {
        if (kw.empty() || util::containsIgnoreCase(b.title, kw) ||
            util::containsIgnoreCase(b.author, kw) ||
            util::containsIgnoreCase(b.isbn, kw) ||
            util::containsIgnoreCase(b.id, kw)) {
            out.push_back(&b);
        }
    }
    return out;
}

const model::Book* LibraryService::findBook(const std::string& bookId) const {
    auto it = std::find_if(books_.begin(), books_.end(),
                           [&](const model::Book& b) { return b.id == bookId; });
    return it == books_.end() ? nullptr : &*it;
}

std::vector<const model::Book*> LibraryService::allBooks() const {
    std::vector<const model::Book*> out;
    out.reserve(books_.size());
    for (const auto& b : books_) out.push_back(&b);
    return out;
}

// ---------- 读者 ----------

std::string LibraryService::addReader(const std::string& name,
                                      const std::string& phone,
                                      std::string& err) {
    err.clear();
    std::string n = util::trim(name);
    if (n.empty()) { err = "读者姓名不能为空"; return {}; }
    model::Reader r;
    r.id = nextReaderId();
    r.name = n;
    r.phone = util::trim(phone);
    readers_.push_back(std::move(r));
    persist();
    return readers_.back().id;
}

bool LibraryService::removeReader(const std::string& readerId, std::string& err) {
    err.clear();
    auto it = std::find_if(readers_.begin(), readers_.end(),
                           [&](const model::Reader& r) { return r.id == readerId; });
    if (it == readers_.end()) { err = "读者不存在：" + readerId; return false; }
    for (const auto& rec : records_)
        if (!rec.returned() && rec.readerId == readerId) {
            err = "该读者还有未归还的图书，无法删除";
            return false;
        }
    readers_.erase(it);
    persist();
    return true;
}

const model::Reader* LibraryService::findReader(const std::string& readerId) const {
    auto it = std::find_if(readers_.begin(), readers_.end(),
                           [&](const model::Reader& r) { return r.id == readerId; });
    return it == readers_.end() ? nullptr : &*it;
}

std::vector<const model::Reader*> LibraryService::allReaders() const {
    std::vector<const model::Reader*> out;
    out.reserve(readers_.size());
    for (const auto& r : readers_) out.push_back(&r);
    return out;
}

// ---------- 借还 ----------

bool LibraryService::borrowBook(const std::string& readerId,
                                const std::string& bookId, int loanDays,
                                std::string& err) {
    err.clear();
    if (loanDays < 1) { err = "借期必须大于等于 1 天"; return false; }
    if (findReader(readerId) == nullptr) { err = "读者不存在：" + readerId; return false; }
    const model::Book* book = findBook(bookId);
    if (book == nullptr) { err = "图书不存在：" + bookId; return false; }
    for (const auto& rec : records_)
        if (!rec.returned() && rec.readerId == readerId && rec.bookId == bookId) {
            err = "该读者已借阅此书且尚未归还";
            return false;
        }
    auto bit = std::find_if(books_.begin(), books_.end(),
                            [&](const model::Book& b) { return b.id == bookId; });
    if (bit->availableCopies < 1) { err = "该书库存不足"; return false; }

    model::BorrowRecord rec;
    rec.id = nextRecordId();
    rec.readerId = readerId;
    rec.bookId = bookId;
    rec.borrowDate = util::today();
    rec.dueDate = util::addDays(rec.borrowDate, loanDays);
    records_.push_back(std::move(rec));
    --bit->availableCopies;
    persist();
    return true;
}

bool LibraryService::returnBook(const std::string& readerId,
                                const std::string& bookId, std::string& err) {
    err.clear();
    auto rit = std::find_if(records_.begin(), records_.end(),
                            [&](const model::BorrowRecord& r) {
                                return !r.returned() && r.readerId == readerId &&
                                       r.bookId == bookId;
                            });
    if (rit == records_.end()) {
        err = "未找到该读者对这本书的未归还记录";
        return false;
    }
    rit->returnDate = util::today();
    auto bit = std::find_if(books_.begin(), books_.end(),
                            [&](const model::Book& b) { return b.id == bookId; });
    if (bit != books_.end()) ++bit->availableCopies;
    persist();
    return true;
}

std::vector<const model::BorrowRecord*> LibraryService::activeRecords() const {
    std::vector<const model::BorrowRecord*> out;
    for (const auto& r : records_)
        if (!r.returned()) out.push_back(&r);
    return out;
}

std::vector<const model::BorrowRecord*> LibraryService::allRecords() const {
    std::vector<const model::BorrowRecord*> out;
    out.reserve(records_.size());
    for (const auto& r : records_) out.push_back(&r);
    return out;
}

const model::BorrowRecord* LibraryService::findRecord(const std::string& id) const {
    auto it = std::find_if(records_.begin(), records_.end(),
                           [&](const model::BorrowRecord& r) { return r.id == id; });
    return it == records_.end() ? nullptr : &*it;
}

bool LibraryService::isOverdue(const model::BorrowRecord& record) const {
    if (record.returned()) return false;
    return util::daysBetween(record.dueDate, util::today()) > 0;
}

// ---------- 统计 ----------

LibraryService::Stats LibraryService::stats() const {
    Stats s;
    s.bookTitles = static_cast<int>(books_.size());
    s.readerCount = static_cast<int>(readers_.size());
    for (const auto& b : books_) {
        s.totalCopies += b.totalCopies;
        s.availableCopies += b.availableCopies;
    }
    for (const auto& r : records_) {
        if (!r.returned()) {
            ++s.activeLoans;
            if (isOverdue(r)) ++s.overdueLoans;
        }
    }
    return s;
}

}  // namespace lms::service
