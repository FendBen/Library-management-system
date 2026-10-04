#include <filesystem>
#include <iostream>
#include <memory>
#include <string>

#include "cli/CliApp.hpp"
#include "service/LibraryService.hpp"
#include "storage/JsonStorage.hpp"

int main(int argc, char* argv[]) {
    std::string dataPath = "data/library.json";
    if (argc > 1) dataPath = argv[1];

    try {
        // 自动创建数据目录
        auto dir = std::filesystem::path(dataPath).parent_path();
        if (!dir.empty() && !std::filesystem::exists(dir))
            std::filesystem::create_directories(dir);

        auto storage = std::make_unique<lms::storage::JsonStorage>(dataPath);
        lms::service::LibraryService service(std::move(storage));
        if (!service.load()) {
            std::cout << "未找到历史数据（" << dataPath << "），已初始化空图书馆。\n";
        } else {
            std::cout << "已加载历史数据（" << dataPath << "）。\n";
        }

        lms::cli::CliApp app(service);
        return app.run();
    } catch (const std::exception& e) {
        std::cerr << "致命错误：" << e.what() << "\n";
        return 1;
    }
}
