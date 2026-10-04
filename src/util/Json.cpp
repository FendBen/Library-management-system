#include "util/Json.hpp"

#include <cmath>
#include <cstdio>
#include <fstream>
#include <sstream>

namespace lms::json {

namespace {

std::string escape(const std::string& s) {
    std::string out;
    out.reserve(s.size() + 8);
    for (unsigned char c : s) {
        switch (c) {
            case '"': out += "\\\""; break;
            case '\\': out += "\\\\"; break;
            case '\b': out += "\\b"; break;
            case '\f': out += "\\f"; break;
            case '\n': out += "\\n"; break;
            case '\r': out += "\\r"; break;
            case '\t': out += "\\t"; break;
            default:
                if (c < 0x20) {
                    char buf[8];
                    std::snprintf(buf, sizeof(buf), "\\u%04x", c);
                    out += buf;
                } else {
                    out += static_cast<char>(c);
                }
        }
    }
    return out;
}

std::string toUtf8(unsigned cp) {
    std::string out;
    if (cp < 0x80) {
        out += static_cast<char>(cp);
    } else if (cp < 0x800) {
        out += static_cast<char>(0xC0 | (cp >> 6));
        out += static_cast<char>(0x80 | (cp & 0x3F));
    } else if (cp < 0x10000) {
        out += static_cast<char>(0xE0 | (cp >> 12));
        out += static_cast<char>(0x80 | ((cp >> 6) & 0x3F));
        out += static_cast<char>(0x80 | (cp & 0x3F));
    } else {
        out += static_cast<char>(0xF0 | (cp >> 18));
        out += static_cast<char>(0x80 | ((cp >> 12) & 0x3F));
        out += static_cast<char>(0x80 | ((cp >> 6) & 0x3F));
        out += static_cast<char>(0x80 | (cp & 0x3F));
    }
    return out;
}

class Parser {
public:
    explicit Parser(const std::string& text) : s_(text), pos_(0) {}

    Value parse() {
        skipWs();
        Value v = parseValue();
        skipWs();
        if (pos_ != s_.size())
            throw JsonError("unexpected trailing characters after JSON value");
        return v;
    }

private:
    const std::string& s_;
    size_t pos_ = 0;

    void skipWs() {
        while (pos_ < s_.size()) {
            char c = s_[pos_];
            if (c == ' ' || c == '\t' || c == '\n' || c == '\r')
                ++pos_;
            else
                break;
        }
    }

    char peek() const { return pos_ < s_.size() ? s_[pos_] : '\0'; }

    char take() {
        if (pos_ >= s_.size()) throw JsonError("unexpected end of input");
        return s_[pos_++];
    }

    void expect(char c) {
        char got = take();
        if (got != c)
            throw JsonError(std::string("expected '") + c + "' but found '" + got + "'");
    }

    Value parseValue() {
        char c = peek();
        switch (c) {
            case '{': return parseObject();
            case '[': return parseArray();
            case '"': return Value(parseString());
            case 't': expect('t'); expect('r'); expect('u'); expect('e'); return Value(true);
            case 'f': expect('f'); expect('a'); expect('l'); expect('s'); expect('e'); return Value(false);
            case 'n': expect('n'); expect('u'); expect('l'); expect('l'); return Value();
            default:
                if (c == '-' || (c >= '0' && c <= '9')) return parseNumber();
                throw JsonError(std::string("unexpected character '") + c + "'");
        }
    }

    Value parseObject() {
        expect('{');
        Value obj(Value::Type::Object);
        skipWs();
        if (peek() == '}') {
            take();
            return obj;
        }
        while (true) {
            skipWs();
            std::string key = parseString();
            skipWs();
            expect(':');
            skipWs();
            obj[key] = parseValue();
            skipWs();
            char c = take();
            if (c == ',') continue;
            if (c == '}') break;
            throw JsonError("expected ',' or '}' in object");
        }
        return obj;
    }

    Value parseArray() {
        expect('[');
        Value arr(Value::Type::Array);
        skipWs();
        if (peek() == ']') {
            take();
            return arr;
        }
        while (true) {
            skipWs();
            arr.push(parseValue());
            skipWs();
            char c = take();
            if (c == ',') continue;
            if (c == ']') break;
            throw JsonError("expected ',' or ']' in array");
        }
        return arr;
    }

    std::string parseString() {
        expect('"');
        std::string out;
        while (true) {
            char c = take();
            if (c == '"') return out;
            if (c == '\\') {
                char e = take();
                switch (e) {
                    case '"': out += '"'; break;
                    case '\\': out += '\\'; break;
                    case '/': out += '/'; break;
                    case 'b': out += '\b'; break;
                    case 'f': out += '\f'; break;
                    case 'n': out += '\n'; break;
                    case 'r': out += '\r'; break;
                    case 't': out += '\t'; break;
                    case 'u': {
                        unsigned cp = 0;
                        for (int i = 0; i < 4; ++i) {
                            char h = take();
                            cp <<= 4;
                            if (h >= '0' && h <= '9')
                                cp |= static_cast<unsigned>(h - '0');
                            else if (h >= 'a' && h <= 'f')
                                cp |= static_cast<unsigned>(h - 'a' + 10);
                            else if (h >= 'A' && h <= 'F')
                                cp |= static_cast<unsigned>(h - 'A' + 10);
                            else
                                throw JsonError("invalid unicode escape");
                        }
                        out += toUtf8(cp);
                        break;
                    }
                    default: throw JsonError(std::string("invalid escape '\\") + e + "'");
                }
            } else if (static_cast<unsigned char>(c) < 0x20) {
                throw JsonError("unescaped control character in string");
            } else {
                out += c;
            }
        }
    }

    Value parseNumber() {
        size_t start = pos_;
        if (peek() == '-') take();
        while (peek() >= '0' && peek() <= '9') take();
        if (peek() == '.') {
            take();
            while (peek() >= '0' && peek() <= '9') take();
        }
        if (peek() == 'e' || peek() == 'E') {
            take();
            if (peek() == '+' || peek() == '-') take();
            while (peek() >= '0' && peek() <= '9') take();
        }
        std::string tok = s_.substr(start, pos_ - start);
        if (tok.empty() || tok == "-") throw JsonError("invalid number");
        if (tok.size() > 1 && tok[0] == '0') throw JsonError("invalid number with leading zero");
        if (tok.size() > 2 && tok[0] == '-' && tok[1] == '0')
            throw JsonError("invalid number with leading zero");
        char* endp = nullptr;
        double d = std::strtod(tok.c_str(), &endp);
        if (endp == tok.c_str()) throw JsonError("invalid number");
        return Value(d);
    }
};

void writeValue(const Value& v, std::string& out, int depth, int indent) {
    switch (v.type()) {
        case Value::Type::Null: out += "null"; break;
        case Value::Type::Bool: out += v.asBool() ? "true" : "false"; break;
        case Value::Type::Number: {
            double d = v.asNumber();
            if (std::floor(d) == d && std::abs(d) < 1e15) {
                char buf[64];
                std::snprintf(buf, sizeof(buf), "%lld", static_cast<long long>(d));
                out += buf;
            } else {
                char buf[64];
                std::snprintf(buf, sizeof(buf), "%.17g", d);
                out += buf;
            }
            break;
        }
        case Value::Type::String: out += '"' + escape(v.asString()) + '"'; break;
        case Value::Type::Array: {
            if (v.arr().empty()) {
                out += "[]";
                break;
            }
            out += '[';
            bool first = true;
            for (const auto& item : v.arr()) {
                if (!first) out += ',';
                first = false;
                if (indent >= 0) {
                    out += '\n';
                    out += std::string(static_cast<size_t>(indent * (depth + 1)), ' ');
                }
                writeValue(item, out, depth + 1, indent);
            }
            if (indent >= 0) {
                out += '\n';
                out += std::string(static_cast<size_t>(indent * depth), ' ');
            }
            out += ']';
            break;
        }
        case Value::Type::Object: {
            if (v.obj().empty()) {
                out += "{}";
                break;
            }
            out += '{';
            bool first = true;
            for (const auto& kv : v.obj()) {
                if (!first) out += ',';
                first = false;
                if (indent >= 0) {
                    out += '\n';
                    out += std::string(static_cast<size_t>(indent * (depth + 1)), ' ');
                }
                out += '"' + escape(kv.first) + "\":";
                if (indent >= 0) out += ' ';
                writeValue(kv.second, out, depth + 1, indent);
            }
            if (indent >= 0) {
                out += '\n';
                out += std::string(static_cast<size_t>(indent * depth), ' ');
            }
            out += '}';
            break;
        }
    }
}

}  // namespace

const Value& Value::at(const std::string& key) const {
    auto it = obj_.find(key);
    if (it == obj_.end()) throw JsonError("missing key: " + key);
    return it->second;
}

std::string Value::dump() const {
    std::string out;
    writeValue(*this, out, 0, -1);
    return out;
}

std::string Value::dumpPretty(int indent) const {
    std::string out;
    writeValue(*this, out, 0, indent);
    return out;
}

Value Value::parse(const std::string& text) { return Parser(text).parse(); }

Value Value::parseFile(const std::string& path) {
    std::ifstream in(path, std::ios::binary);
    if (!in) throw JsonError("cannot open file: " + path);
    std::ostringstream ss;
    ss << in.rdbuf();
    return parse(ss.str());
}

void Value::writeFile(const std::string& path, bool pretty) const {
    std::ofstream out(path, std::ios::binary);
    if (!out) throw JsonError("cannot write file: " + path);
    out << (pretty ? dumpPretty() : dump());
    out << '\n';
}

}  // namespace lms::json
