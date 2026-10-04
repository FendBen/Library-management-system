#pragma once

#include <cstddef>
#include <map>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

namespace lms::json {

// 轻量 JSON 解析/序列化错误
class JsonError : public std::runtime_error {
public:
    explicit JsonError(const std::string& msg) : std::runtime_error(msg) {}
};

// 极简 JSON Value：支持 null/bool/number/string/array/object
class Value {
public:
    enum class Type { Null, Bool, Number, String, Array, Object };

    Value() : type_(Type::Null) {}
    Value(std::nullptr_t) : type_(Type::Null) {}
    Value(bool b) : type_(Type::Bool), bool_(b) {}
    Value(int n) : type_(Type::Number), num_(static_cast<double>(n)) {}
    Value(double n) : type_(Type::Number), num_(n) {}
    Value(const char* s) : type_(Type::String), str_(s) {}
    Value(std::string s) : type_(Type::String), str_(std::move(s)) {}
    explicit Value(Type t) : type_(t) {}

    Type type() const { return type_; }
    bool isNull() const { return type_ == Type::Null; }
    bool isBool() const { return type_ == Type::Bool; }
    bool isNumber() const { return type_ == Type::Number; }
    bool isString() const { return type_ == Type::String; }
    bool isArray() const { return type_ == Type::Array; }
    bool isObject() const { return type_ == Type::Object; }

    bool asBool() const { return bool_; }
    double asNumber() const { return num_; }
    const std::string& asString() const { return str_; }

    std::vector<Value>& arr() { return arr_; }
    const std::vector<Value>& arr() const { return arr_; }
    std::map<std::string, Value>& obj() { return obj_; }
    const std::map<std::string, Value>& obj() const { return obj_; }

    // 对象访问：不存在时返回默认 Value，写入用 operator[]
    const Value& at(const std::string& key) const;
    Value& operator[](const std::string& key) { return obj_[key]; }
    Value& push(Value v) { arr_.push_back(std::move(v)); return arr_.back(); }

    static Value array() { return Value(Type::Array); }
    static Value object() { return Value(Type::Object); }

    // 序列化：dump() 紧凑，dumpPretty() 带缩进
    std::string dump() const;
    std::string dumpPretty(int indent = 2) const;

    // 解析：文本 / 文件（文件不存在或解析失败抛 JsonError）
    static Value parse(const std::string& text);
    static Value parseFile(const std::string& path);
    void writeFile(const std::string& path, bool pretty = true) const;

private:
    Type type_;
    bool bool_ = false;
    double num_ = 0.0;
    std::string str_;
    std::vector<Value> arr_;
    std::map<std::string, Value> obj_;
};

}  // namespace lms::json
