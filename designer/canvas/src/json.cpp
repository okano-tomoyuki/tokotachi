#include "json.hpp"

#include <cmath>
#include <cstdio>
#include <cstdlib>

namespace ttc
{

namespace
{

class Parser
{
public:
    explicit Parser(const std::string& text) : s_(text), pos_(0) {}

    bool parse(Json& out, std::string& error)
    {
        if (!value(out, 0))
        {
            error = error_.empty() ? "invalid JSON" : error_;
            return false;
        }
        skip_space();
        if (pos_ != s_.size())
        {
            error = "unexpected data after JSON";
            return false;
        }
        return true;
    }

private:
    bool fail(const char* message)
    {
        if (error_.empty())
        {
            error_ = std::string(message) + " at " + std::to_string(pos_);
        }
        return false;
    }

    void skip_space()
    {
        while (pos_ < s_.size() && (s_[pos_] == ' ' || s_[pos_] == '\t' || s_[pos_] == '\n' || s_[pos_] == '\r'))
        {
            ++pos_;
        }
    }

    bool literal(const char* word)
    {
        const std::string w(word);
        if (s_.compare(pos_, w.size(), w) != 0)
        {
            return fail("unknown literal");
        }
        pos_ += w.size();
        return true;
    }

    bool value(Json& out, int depth)
    {
        if (depth > 256)
        {
            return fail("too deep");
        }
        skip_space();
        if (pos_ >= s_.size())
        {
            return fail("unexpected end");
        }
        const char c = s_[pos_];
        if (c == '{')
        {
            return object(out, depth);
        }
        if (c == '[')
        {
            return array(out, depth);
        }
        if (c == '"')
        {
            std::string str;
            if (!string(str))
            {
                return false;
            }
            out = Json(str);
            return true;
        }
        if (c == 't')
        {
            out = Json(true);
            return literal("true");
        }
        if (c == 'f')
        {
            out = Json(false);
            return literal("false");
        }
        if (c == 'n')
        {
            out = Json();
            return literal("null");
        }
        return number(out);
    }

    bool number(Json& out)
    {
        const char* begin = s_.c_str() + pos_;
        char* end = nullptr;
        const double v = std::strtod(begin, &end);
        if (end == begin)
        {
            return fail("invalid value");
        }
        pos_ += static_cast<std::size_t>(end - begin);
        out = Json(v);
        return true;
    }

    static void append_utf8(std::string& out, unsigned cp)
    {
        if (cp < 0x80)
        {
            out += static_cast<char>(cp);
        }
        else if (cp < 0x800)
        {
            out += static_cast<char>(0xC0 | (cp >> 6));
            out += static_cast<char>(0x80 | (cp & 0x3F));
        }
        else if (cp < 0x10000)
        {
            out += static_cast<char>(0xE0 | (cp >> 12));
            out += static_cast<char>(0x80 | ((cp >> 6) & 0x3F));
            out += static_cast<char>(0x80 | (cp & 0x3F));
        }
        else
        {
            out += static_cast<char>(0xF0 | (cp >> 18));
            out += static_cast<char>(0x80 | ((cp >> 12) & 0x3F));
            out += static_cast<char>(0x80 | ((cp >> 6) & 0x3F));
            out += static_cast<char>(0x80 | (cp & 0x3F));
        }
    }

    bool hex4(unsigned& out)
    {
        if (pos_ + 4 > s_.size())
        {
            return fail("invalid escape");
        }
        out = 0;
        for (int i = 0; i < 4; ++i)
        {
            const char c = s_[pos_++];
            out <<= 4;
            if (c >= '0' && c <= '9')
            {
                out |= static_cast<unsigned>(c - '0');
            }
            else if (c >= 'a' && c <= 'f')
            {
                out |= static_cast<unsigned>(c - 'a' + 10);
            }
            else if (c >= 'A' && c <= 'F')
            {
                out |= static_cast<unsigned>(c - 'A' + 10);
            }
            else
            {
                return fail("invalid escape");
            }
        }
        return true;
    }

    bool string(std::string& out)
    {
        ++pos_;  // "
        while (pos_ < s_.size())
        {
            const char c = s_[pos_++];
            if (c == '"')
            {
                return true;
            }
            if (c != '\\')
            {
                out += c;
                continue;
            }
            if (pos_ >= s_.size())
            {
                break;
            }
            const char e = s_[pos_++];
            switch (e)
            {
            case '"':
            case '\\':
            case '/':
                out += e;
                break;
            case 'b':
                out += '\b';
                break;
            case 'f':
                out += '\f';
                break;
            case 'n':
                out += '\n';
                break;
            case 'r':
                out += '\r';
                break;
            case 't':
                out += '\t';
                break;
            case 'u':
            {
                unsigned cp = 0;
                if (!hex4(cp))
                {
                    return false;
                }
                // サロゲートの組
                if (cp >= 0xD800 && cp < 0xDC00 && pos_ + 6 <= s_.size() && s_[pos_] == '\\' && s_[pos_ + 1] == 'u')
                {
                    pos_ += 2;
                    unsigned low = 0;
                    if (!hex4(low))
                    {
                        return false;
                    }
                    cp = 0x10000 + ((cp - 0xD800) << 10) + (low - 0xDC00);
                }
                append_utf8(out, cp);
                break;
            }
            default:
                return fail("invalid escape");
            }
        }
        return fail("unterminated string");
    }

    bool array(Json& out, int depth)
    {
        ++pos_;  // [
        out = Json::array();
        skip_space();
        if (pos_ < s_.size() && s_[pos_] == ']')
        {
            ++pos_;
            return true;
        }
        for (;;)
        {
            Json v;
            if (!value(v, depth + 1))
            {
                return false;
            }
            out.push(v);
            skip_space();
            if (pos_ < s_.size() && s_[pos_] == ',')
            {
                ++pos_;
                continue;
            }
            if (pos_ < s_.size() && s_[pos_] == ']')
            {
                ++pos_;
                return true;
            }
            return fail("expected , or ]");
        }
    }

    bool object(Json& out, int depth)
    {
        ++pos_;  // {
        out = Json::object();
        skip_space();
        if (pos_ < s_.size() && s_[pos_] == '}')
        {
            ++pos_;
            return true;
        }
        for (;;)
        {
            skip_space();
            if (pos_ >= s_.size() || s_[pos_] != '"')
            {
                return fail("expected key");
            }
            std::string key;
            if (!string(key))
            {
                return false;
            }
            skip_space();
            if (pos_ >= s_.size() || s_[pos_] != ':')
            {
                return fail("expected :");
            }
            ++pos_;
            Json v;
            if (!value(v, depth + 1))
            {
                return false;
            }
            out.set(key, v);
            skip_space();
            if (pos_ < s_.size() && s_[pos_] == ',')
            {
                ++pos_;
                continue;
            }
            if (pos_ < s_.size() && s_[pos_] == '}')
            {
                ++pos_;
                return true;
            }
            return fail("expected , or }");
        }
    }

    const std::string& s_;
    std::size_t pos_;
    std::string error_;
};

void dump_string(const std::string& s, std::string& out)
{
    out += '"';
    for (std::size_t i = 0; i < s.size(); ++i)
    {
        const unsigned char c = static_cast<unsigned char>(s[i]);
        if (c == '"' || c == '\\')
        {
            out += '\\';
            out += static_cast<char>(c);
        }
        else if (c == '\n')
        {
            out += "\\n";
        }
        else if (c == '\r')
        {
            out += "\\r";
        }
        else if (c == '\t')
        {
            out += "\\t";
        }
        else if (c < 0x20)
        {
            char buf[8];
            std::snprintf(buf, sizeof(buf), "\\u%04x", c);
            out += buf;
        }
        else
        {
            out += static_cast<char>(c);
        }
    }
    out += '"';
}

void dump_value(const Json& j, std::string& out)
{
    switch (j.type())
    {
    case Json::NULL_VALUE:
        out += "null";
        break;
    case Json::BOOL:
        out += j.as_bool() ? "true" : "false";
        break;
    case Json::NUMBER:
    {
        const double v = j.as_number();
        char buf[32];
        if (!std::isfinite(v))
        {
            out += "null";
        }
        else if (v == std::floor(v) && std::fabs(v) < 1e15)
        {
            std::snprintf(buf, sizeof(buf), "%.0f", v);
            out += buf;
        }
        else
        {
            std::snprintf(buf, sizeof(buf), "%.9g", v);
            out += buf;
        }
        break;
    }
    case Json::STRING:
        dump_string(j.as_string(), out);
        break;
    case Json::ARRAY:
        out += '[';
        for (std::size_t i = 0; i < j.items().size(); ++i)
        {
            if (i > 0)
            {
                out += ',';
            }
            dump_value(j.items()[i], out);
        }
        out += ']';
        break;
    case Json::OBJECT:
        out += '{';
        for (std::size_t i = 0; i < j.members().size(); ++i)
        {
            if (i > 0)
            {
                out += ',';
            }
            dump_string(j.members()[i].first, out);
            out += ':';
            dump_value(j.members()[i].second, out);
        }
        out += '}';
        break;
    }
}

}  // namespace

bool Json::parse(const std::string& text, Json& out, std::string& error)
{
    return Parser(text).parse(out, error);
}

std::string Json::dump() const
{
    std::string out;
    dump_value(*this, out);
    return out;
}

const Json& Json::operator[](const std::string& key) const
{
    static const Json null_value;
    for (std::size_t i = 0; i < members_.size(); ++i)
    {
        if (members_[i].first == key)
        {
            return members_[i].second;
        }
    }
    return null_value;
}

bool Json::has(const std::string& key) const
{
    for (std::size_t i = 0; i < members_.size(); ++i)
    {
        if (members_[i].first == key)
        {
            return true;
        }
    }
    return false;
}

void Json::set(const std::string& key, const Json& v)
{
    for (std::size_t i = 0; i < members_.size(); ++i)
    {
        if (members_[i].first == key)
        {
            members_[i].second = v;
            return;
        }
    }
    members_.push_back(std::make_pair(key, v));
}

}  // namespace ttc
