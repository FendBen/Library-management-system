#pragma once

#include <string>

namespace lms::util {

// 本地今天日期，格式 YYYY-MM-DD
std::string today();

// 在 YYYY-MM-DD 基础上加 days 天（可为负），返回 YYYY-MM-DD
std::string addDays(const std::string& date, int days);

// to - from 的天数差（同一天为 0，to 早于 from 为负）
long daysBetween(const std::string& from, const std::string& to);

// 校验字符串是否为合法日期（YYYY-MM-DD）
bool isValidDate(const std::string& date);

}  // namespace lms::util
