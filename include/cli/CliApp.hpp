#pragma once

#include <string>

#include "service/LibraryService.hpp"

namespace lms::cli {

// 命令行交互层
class CliApp {
public:
    explicit CliApp(service::LibraryService& service) : svc_(service) {}
    int run();

private:
    service::LibraryService& svc_;

    void printMenu() const;
    void addBookFlow();
    void removeBookFlow();
    void updateBookFlow();
    void searchBookFlow();
    void listBooksFlow();
    void addReaderFlow();
    void removeReaderFlow();
    void listReadersFlow();
    void borrowFlow();
    void returnFlow();
    void listRecordsFlow();
    void showStatsFlow();

    static std::string readLine(const std::string& prompt);
    static int readInt(const std::string& prompt);
};

}  // namespace lms::cli
