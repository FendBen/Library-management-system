#pragma once

#include <memory>
#include <string>
#include <vector>

#include "model/Book.hpp"
#include "model/BorrowRecord.hpp"
#include "model/Reader.hpp"
#include "storage/Storage.hpp"

namespace lms::service {

// 业务服务层：图书/读者/借还/统计，所有变更即时持久化
class LibraryService {
public:
    explicit LibraryService(std::unique_ptr<storage::Storage> storage);

    // 加载历史数据；返回 false 表示无历史文件（从空库开始）
    bool load();

    // ---------- 图书 ----------
    // 成功返回新图书 id，失败返回空串并在 err 中给出原因
    std::string addBook(const std::string& title, const std::string& author,
                        const std::string& isbn, int totalCopies, std::string& err);
    bool removeBook(const std::string& bookId, std::string& err);
    bool updateBook(const std::string& bookId, const std::string& title,
                    const std::string& author, const std::string& isbn,
                    int totalCopies, std::string& err);
    // 按关键字（书名/作者/ISBN/编号，忽略大小写）搜索
    std::vector<const model::Book*> searchBooks(const std::string& keyword) const;
    const model::Book* findBook(const std::string& bookId) const;
    std::vector<const model::Book*> allBooks() const;

    // ---------- 读者 ----------
    std::string addReader(const std::string& name, const std::string& phone,
                          std::string& err);
    bool removeReader(const std::string& readerId, std::string& err);
    const model::Reader* findReader(const std::string& readerId) const;
    std::vector<const model::Reader*> allReaders() const;

    // ---------- 借还 ----------
    bool borrowBook(const std::string& readerId, const std::string& bookId,
                    int loanDays, std::string& err);
    bool returnBook(const std::string& readerId, const std::string& bookId,
                    std::string& err);
    std::vector<const model::BorrowRecord*> activeRecords() const;
    std::vector<const model::BorrowRecord*> allRecords() const;
    const model::BorrowRecord* findRecord(const std::string& id) const;
    bool isOverdue(const model::BorrowRecord& record) const;

    // ---------- 统计 ----------
    struct Stats {
        int bookTitles = 0;        // 图书种数
        int totalCopies = 0;       // 馆藏总册数
        int availableCopies = 0;   // 可借总册数
        int readerCount = 0;       // 读者人数
        int activeLoans = 0;       // 在借笔数
        int overdueLoans = 0;      // 逾期笔数
    };
    Stats stats() const;

    // 手动保存（一般由内部 persist 触发，暴露用于测试/退出前落盘）
    bool save();

private:
    std::unique_ptr<storage::Storage> storage_;
    std::vector<model::Book> books_;
    std::vector<model::Reader> readers_;
    std::vector<model::BorrowRecord> records_;
    int nextBookSeq_ = 1;
    int nextReaderSeq_ = 1;
    int nextRecordSeq_ = 1;

    std::string nextBookId();
    std::string nextReaderId();
    std::string nextRecordId();
    void persist();
};

}  // namespace lms::service
