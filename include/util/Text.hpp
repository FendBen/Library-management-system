#pragma once

#include <string>

namespace lms::util {

// 去除首尾空白（空格、制表符、换行）
std::string trim(const std::string& s);

// 转小写（ASCII 范围，用于忽略大小写搜索）
std::string toLower(const std::string& s);

// haystack 是否包含 needle（忽略大小写）
bool containsIgnoreCase(const std::string& haystack, const std::string& needle);

}  // namespace lms::util
