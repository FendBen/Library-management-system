#include "cli/CliApp.hpp"

#include <iostream>
#include <limits>
#include <string>

#include "util/Text.hpp"

namespace lms::cli {

using model::Book;
using model::BorrowRecord;
using model::Reader;

int CliApp::run() {
    while (true) {
        printMenu();
        int choice = readInt("请输入功能编号：");
        switch (choice) {
            case 0: return 0;
            case 1: addBookFlow(); break;
            case 2: removeBookFlow(); break;
            case 3: updateBookFlow(); break;
            case 4: searchBookFlow(); break;
            case 5: listBooksFlow(); break;
            case 6: addReaderFlow(); break;
            case 7: removeReaderFlow(); break;
            case 8: borrowFlow(); break;
            case 9: returnFlow(); break;
            case 10: listRecordsFlow(); break;
            case 11: showStatsFlow(); break;
            default: std::cout << "无效编号，请重新输入。\n";
        }
    }
}

void CliApp::printMenu() const {
    std::cout << "\n========== 图书管理系统 ==========\n"
              << " 1. 添加图书       6. 添加读者\n"
              << " 2. 删除图书       7. 删除读者\n"
              << " 3. 修改图书       8. 借书\n"
              << " 4. 查询图书       9. 还书\n"
              << " 5. 显示全部图书  10. 借阅记录\n"
              << "                   11. 统计信息\n"
              << " 0. 退出\n";
}

std::string CliApp::readLine(const std::string& prompt) {
    std::cout << prompt;
    std::string line;
    std::getline(std::cin, line);
    return line;
}

int CliApp::readInt(const std::string& prompt) {
    while (true) {
        std::string line = readLine(prompt);
        try {
            size_t pos = 0;
            int v = std::stoi(line, &pos);
            if (pos == line.size()) return v;
        } catch (...) {
            // 继续重试
        }
        std::cout << "请输入有效整数。\n";
    }
}

void CliApp::addBookFlow() {
    std::string title = readLine("书名：");
    std::string author = readLine("作者：");
    std::string isbn = readLine("ISBN（可空）：");
    int copies = readInt("馆藏册数：");
    std::string err;
    std::string id = svc_.addBook(title, author, isbn, copies, err);
    if (id.empty())
        std::cout << "添加失败：" << err << "\n";
    else
        std::cout << "添加成功，图书编号：" << id << "\n";
}

void CliApp::removeBookFlow() {
    std::string id = readLine("请输入要删除的图书编号：");
    std::string err;
    if (svc_.removeBook(id, err))
        std::cout << "删除成功。\n";
    else
        std::cout << "删除失败：" << err << "\n";
}

void CliApp::updateBookFlow() {
    std::string id = readLine("请输入要修改的图书编号：");
    const Book* book = svc_.findBook(id);
    if (book == nullptr) {
        std::cout << "图书不存在：" << id << "\n";
        return;
    }
    std::cout << "当前：[" << book->id << "] 《" << book->title << "》 "
              << book->author << " ISBN=" << book->isbn << " 共"
              << book->totalCopies << " 册（可借 " << book->availableCopies << " 册）\n";
    std::string title = readLine("新书名（直接回车保持不变）：");
    std::string author = readLine("新作者（直接回车保持不变）：");
    std::string isbn = readLine("新 ISBN（直接回车保持不变）：");
    std::string copiesStr = readLine("新册数（直接回车保持不变）：");
    if (title.empty()) title = book->title;
    if (author.empty()) author = book->author;
    if (isbn.empty()) isbn = book->isbn;
    int copies = book->totalCopies;
    if (!copiesStr.empty()) {
        try {
            copies = std::stoi(copiesStr);
        } catch (...) {
            std::cout << "册数格式错误，已保持原值。\n";
        }
    }
    std::string err;
    if (svc_.updateBook(id, title, author, isbn, copies, err))
        std::cout << "修改成功。\n";
    else
        std::cout << "修改失败：" << err << "\n";
}

void CliApp::searchBookFlow() {
    std::string kw = readLine("输入关键字（书名/作者/ISBN/编号，回车显示全部）：");
    auto results = svc_.searchBooks(kw);
    if (results.empty()) {
        std::cout << "没有匹配的图书。\n";
        return;
    }
    for (const Book* b : results)
        std::cout << "[" << b->id << "] 《" << b->title << "》 " << b->author
                  << " ISBN=" << (b->isbn.empty() ? "-" : b->isbn) << " 共"
                  << b->totalCopies << " 册（可借 " << b->availableCopies << " 册）\n";
}

void CliApp::listBooksFlow() {
    auto books = svc_.allBooks();
    if (books.empty()) {
        std::cout << "图书馆暂无图书。\n";
        return;
    }
    for (const Book* b : books)
        std::cout << "[" << b->id << "] 《" << b->title << "》 " << b->author
                  << " ISBN=" << (b->isbn.empty() ? "-" : b->isbn) << " 共"
                  << b->totalCopies << " 册（可借 " << b->availableCopies << " 册）\n";
}

void CliApp::addReaderFlow() {
    std::string name = readLine("读者姓名：");
    std::string phone = readLine("联系电话（可空）：");
    std::string err;
    std::string id = svc_.addReader(name, phone, err);
    if (id.empty())
        std::cout << "添加失败：" << err << "\n";
    else
        std::cout << "添加成功，读者编号：" << id << "\n";
}

void CliApp::removeReaderFlow() {
    std::string id = readLine("请输入要删除的读者编号：");
    std::string err;
    if (svc_.removeReader(id, err))
        std::cout << "删除成功。\n";
    else
        std::cout << "删除失败：" << err << "\n";
}

void CliApp::listReadersFlow() {
    auto readers = svc_.allReaders();
    if (readers.empty()) {
        std::cout << "暂无读者。\n";
        return;
    }
    for (const Reader* r : readers)
        std::cout << "[" << r->id << "] " << r->name
                  << (r->phone.empty() ? "" : " 电话：" + r->phone) << "\n";
}

void CliApp::borrowFlow() {
    std::string readerId = readLine("读者编号：");
    std::string bookId = readLine("图书编号：");
    int days = readInt("借期（天）：");
    std::string err;
    if (svc_.borrowBook(readerId, bookId, days, err))
        std::cout << "借书成功，应还日期见借阅记录。\n";
    else
        std::cout << "借书失败：" << err << "\n";
}

void CliApp::returnFlow() {
    std::string readerId = readLine("读者编号：");
    std::string bookId = readLine("图书编号：");
    std::string err;
    if (svc_.returnBook(readerId, bookId, err))
        std::cout << "还书成功。\n";
    else
        std::cout << "还书失败：" << err << "\n";
}

void CliApp::listRecordsFlow() {
    auto records = svc_.allRecords();
    if (records.empty()) {
        std::cout << "暂无借阅记录。\n";
        return;
    }
    for (const BorrowRecord* r : records) {
        const Book* b = svc_.findBook(r->bookId);
        const Reader* rd = svc_.findReader(r->readerId);
        std::string bookTitle = b ? b->title : r->bookId;
        std::string readerName = rd ? rd->name : r->readerId;
        std::string status = r->returned() ? "已还（" + r->returnDate + "）"
                                           : (svc_.isOverdue(*r) ? "在借·逾期" : "在借");
        std::cout << "[" << r->id << "] " << readerName << " 借 《" << bookTitle
                  << "》 借出 " << r->borrowDate << " 应还 " << r->dueDate
                  << " 状态：" << status << "\n";
    }
}

void CliApp::showStatsFlow() {
    auto s = svc_.stats();
    std::cout << "图书种数：" << s.bookTitles << "\n"
              << "馆藏总册数：" << s.totalCopies << "\n"
              << "可借册数：" << s.availableCopies << "\n"
              << "读者人数：" << s.readerCount << "\n"
              << "在借笔数：" << s.activeLoans << "\n"
              << "逾期笔数：" << s.overdueLoans << "\n";
}

}  // namespace lms::cli
