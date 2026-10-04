#include <cstdio>
#include <filesystem>
#include <iostream>
#include <memory>
#include <string>

#include "service/LibraryService.hpp"
#include "storage/JsonStorage.hpp"
#include "util/Date.hpp"
#include "util/Json.hpp"

using lms::json::Value;
using lms::service::LibraryService;
using lms::storage::JsonStorage;

static int failures = 0;

#define CHECK(cond)                                                          \
    do {                                                                     \
        if (!(cond)) {                                                       \
            std::cerr << "FAIL [line " << __LINE__ << "]: " #cond "\n";      \
            ++failures;                                                      \
        }                                                                    \
    } while (0)

#define CHECK_EQ(a, b)                                                       \
    do {                                                                     \
        auto va = (a);                                                       \
        auto vb = (b);                                                       \
        if (!(va == vb)) {                                                   \
            std::cerr << "FAIL [line " << __LINE__ << "]: " #a " == " #b     \
                      << " (" << va << " vs " << vb << ")\n";                \
            ++failures;                                                      \
        }                                                                    \
    } while (0)

// ---------- JSON ----------
static void testJson() {
    Value root = Value::object();
    root["name"] = Value("《三体》");
    root["copies"] = Value(3);
    root["ok"] = Value(true);
    root["nothing"] = Value();
    Value arr = Value::array();
    arr.push(Value(1.5));
    arr.push(Value("x"));
    root["items"] = arr;

    std::string text = root.dump();
    CHECK(text.find("\"name\":\"\\u300a\\u4e09\\u4f53\\u300b\"") != std::string::npos ||
          text.find("《三体》") != std::string::npos);

    Value parsed = Value::parse(text);
    CHECK(parsed.at("name").asString() == "《三体》");
    CHECK(parsed.at("copies").asNumber() == 3);
    CHECK(parsed.at("ok").asBool() == true);
    CHECK(parsed.at("nothing").isNull());
    CHECK(parsed.at("items").arr().size() == 2);
    CHECK(parsed.at("items").arr()[0].asNumber() == 1.5);
    CHECK(parsed.at("items").arr()[1].asString() == "x");
    CHECK(Value::parse("{}").isObject());
    CHECK(Value::parse("[]").isArray());
    CHECK(Value::parse("  null  ").isNull());
    CHECK(Value::parse("\"a\\n\\u4e2d\"").asString() == "a\n中");

    bool threw = false;
    try { Value::parse("{bad}"); } catch (const lms::json::JsonError&) { threw = true; }
    CHECK(threw);
}

// ---------- 日期 ----------
static void testDate() {
    std::string t = lms::util::today();
    CHECK(lms::util::isValidDate(t));
    CHECK_EQ(lms::util::addDays("2024-02-28", 1), "2024-02-29");  // 闰年
    CHECK_EQ(lms::util::addDays("2023-02-28", 1), "2023-03-01");  // 平年
    CHECK_EQ(lms::util::addDays("2024-01-31", 1), "2024-02-01");
    CHECK_EQ(lms::util::addDays("2024-01-01", -1), "2023-12-31");
    CHECK_EQ(lms::util::daysBetween("2024-01-01", "2024-01-10"), 9L);
    CHECK_EQ(lms::util::daysBetween("2024-01-10", "2024-01-01"), -9L);
    CHECK(!lms::util::isValidDate("2023-02-29"));
    CHECK(!lms::util::isValidDate("2024-13-01"));
    CHECK(!lms::util::isValidDate("abc"));
}

// ---------- 业务逻辑 + 持久化 ----------
static std::unique_ptr<LibraryService> makeService(const std::string& path,
                                                   bool fresh = true) {
    std::filesystem::create_directories("data_test");
    if (fresh) std::remove(path.c_str());
    auto svc = std::make_unique<LibraryService>(
        std::make_unique<JsonStorage>(path));
    svc->load();
    return svc;
}

static void testService() {
    const std::string path = "data_test/t.json";
    auto svc = makeService(path);

    std::string err;
    std::string id = svc->addBook("三体", "刘慈欣", "978-7-5366-9293-0", 3, err);
    CHECK_EQ(id, "B001");
    CHECK(err.empty());
    CHECK_EQ(svc->addBook("", "某人", "", 1, err), std::string(""));
    CHECK(!err.empty());  // 书名为空应失败

    svc->addBook("百年孤独", "马尔克斯", "", 1, err);
    svc->addBook("三体II·黑暗森林", "刘慈欣", "", 2, err);
    svc->addBook("Harry Potter", "J.K. Rowling", "", 1, err);

    auto found = svc->searchBooks("三体");
    CHECK_EQ(found.size(), 2u);
    found = svc->searchBooks("rowling");  // 忽略大小写匹配作者
    CHECK_EQ(found.size(), 1u);

    std::string r1 = svc->addReader("苏贝力", "13800000000", err);
    CHECK_EQ(r1, "R001");

    CHECK(svc->borrowBook("R001", "B001", 14, err));
    CHECK(err.empty());
    const auto* book = svc->findBook("B001");
    CHECK(book->availableCopies == 2);
    CHECK_EQ(svc->activeRecords().size(), 1u);

    // 同一本书重复借应失败
    CHECK(!svc->borrowBook("R001", "B001", 14, err));
    CHECK(!err.empty());
    // 不存在的读者/书应失败
    CHECK(!svc->borrowBook("R999", "B001", 14, err));
    CHECK(!svc->borrowBook("R001", "B999", 14, err));
    // 有未归还记录不能删书/删读者
    CHECK(!svc->removeBook("B001", err));
    CHECK(!svc->removeReader("R001", err));

    CHECK(svc->returnBook("R001", "B001", err));
    CHECK(svc->findBook("B001")->availableCopies == 3);
    CHECK_EQ(svc->activeRecords().size(), 0u);
    CHECK_EQ(svc->allRecords().size(), 1u);

    auto s = svc->stats();
    CHECK_EQ(s.bookTitles, 4);
    CHECK_EQ(s.totalCopies, 7);
    CHECK_EQ(s.availableCopies, 7);
    CHECK_EQ(s.readerCount, 1);
    CHECK_EQ(s.activeLoans, 0);

    // 持久化回读
    auto svc2 = makeService(path, /*fresh=*/false);
    CHECK_EQ(svc2->allBooks().size(), 4u);
    CHECK_EQ(svc2->allReaders().size(), 1u);
    CHECK_EQ(svc2->allRecords().size(), 1u);
    CHECK(svc2->findBook("B001")->availableCopies == 3);
    std::string newId = svc2->addBook("球状闪电", "刘慈欣", "", 1, err);
    CHECK_EQ(newId, "B005");  // 序号在加载后正确接续
    std::remove(path.c_str());
}

int main() {
    testJson();
    testDate();
    testService();
    if (failures == 0) {
        std::cout << "全部测试通过 ✓\n";
        return 0;
    }
    std::cerr << failures << " 项测试失败 ✗\n";
    return 1;
}
