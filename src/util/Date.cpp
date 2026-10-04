#include "util/Date.hpp"

#include <cstdio>
#include <ctime>
#include <stdexcept>

namespace lms::util {

namespace {

// 1970-01-01 起的天数（Howard Hinnant 的 civil calendar 算法）
long long daysFromCivil(int y, int m, int d) {
    y -= m <= 2;
    const long long era = (y >= 0 ? y : y - 399) / 400;
    const unsigned yoe = static_cast<unsigned>(y - era * 400);
    const unsigned doy =
        static_cast<unsigned>((153 * (m + (m > 2 ? -3 : 9)) + 2) / 5 + d - 1);
    const unsigned doe = yoe * 365 + yoe / 4 - yoe / 100 + doy;
    return era * 146097 + static_cast<long long>(doe) - 719468;
}

void civilFromDays(long long z, int& y, int& m, int& d) {
    z += 719468;
    const long long era = (z >= 0 ? z : z - 146096) / 146097;
    const unsigned doe = static_cast<unsigned>(z - era * 146097);
    const unsigned yoe =
        (doe - doe / 1460 + doe / 36524 - doe / 146096) / 365;
    const long long yy = static_cast<long long>(yoe) + era * 400;
    const unsigned doy = doe - (365 * yoe + yoe / 4 - yoe / 100);
    const unsigned mp = (5 * doy + 2) / 153;
    d = static_cast<int>(doy - (153 * mp + 2) / 5 + 1);
    m = static_cast<int>(mp < 10 ? mp + 3 : mp - 9);
    y = static_cast<int>(m <= 2 ? yy + 1 : yy);
}

bool isLeap(int y) { return (y % 4 == 0 && y % 100 != 0) || y % 400 == 0; }

int daysInMonth(int y, int m) {
    static const int kDays[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    if (m == 2 && isLeap(y)) return 29;
    return kDays[m - 1];
}

bool parseDate(const std::string& s, int& y, int& m, int& d) {
    if (s.size() != 10) return false;
    if (std::sscanf(s.c_str(), "%d-%d-%d", &y, &m, &d) != 3) return false;
    if (m < 1 || m > 12) return false;
    if (d < 1 || d > daysInMonth(y, m)) return false;
    return true;
}

std::string format(int y, int m, int d) {
    char buf[16];
    std::snprintf(buf, sizeof(buf), "%04d-%02d-%02d", y, m, d);
    return buf;
}

}  // namespace

std::string today() {
    std::time_t t = std::time(nullptr);
    std::tm tm{};
#ifdef _WIN32
    localtime_s(&tm, &t);
#else
    localtime_r(&t, &tm);
#endif
    return format(tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday);
}

std::string addDays(const std::string& date, int days) {
    int y = 0, m = 0, d = 0;
    if (!parseDate(date, y, m, d)) throw std::invalid_argument("invalid date: " + date);
    long long z = daysFromCivil(y, m, d) + days;
    civilFromDays(z, y, m, d);
    return format(y, m, d);
}

long daysBetween(const std::string& from, const std::string& to) {
    int y1 = 0, m1 = 0, d1 = 0, y2 = 0, m2 = 0, d2 = 0;
    if (!parseDate(from, y1, m1, d1)) throw std::invalid_argument("invalid date: " + from);
    if (!parseDate(to, y2, m2, d2)) throw std::invalid_argument("invalid date: " + to);
    return static_cast<long>(daysFromCivil(y2, m2, d2) - daysFromCivil(y1, m1, d1));
}

bool isValidDate(const std::string& date) {
    int y = 0, m = 0, d = 0;
    return parseDate(date, y, m, d);
}

}  // namespace lms::util
