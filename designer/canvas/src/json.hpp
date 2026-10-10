// キャンバスの描画の部分(docs/adr/0039)が拡張とやり取りする JSON の読み書き。
// やり取りする内容(操作の並び・結果)に足りるだけの、小さな実装。
#ifndef TOKOTACHI_CANVAS_JSON_HPP
#define TOKOTACHI_CANVAS_JSON_HPP

#include <string>
#include <utility>
#include <vector>

namespace ttc
{

class Json
{
public:
    enum Type
    {
        NULL_VALUE,
        BOOL,
        NUMBER,
        STRING,
        ARRAY,
        OBJECT
    };

    Json() : type_(NULL_VALUE), bool_(false), number_(0) {}
    explicit Json(bool b) : type_(BOOL), bool_(b), number_(0) {}
    explicit Json(double n) : type_(NUMBER), bool_(false), number_(n) {}
    explicit Json(const std::string& s) : type_(STRING), bool_(false), number_(0), string_(s) {}
    static Json array() { Json j; j.type_ = ARRAY; return j; }
    static Json object() { Json j; j.type_ = OBJECT; return j; }

    /// text を読む。読めなければ false(error に理由)。
    static bool parse(const std::string& text, Json& out, std::string& error);
    /// 1 行の JSON にする。
    std::string dump() const;

    Type type() const { return type_; }
    bool is_null() const { return type_ == NULL_VALUE; }
    bool is_bool() const { return type_ == BOOL; }
    bool is_number() const { return type_ == NUMBER; }
    bool is_string() const { return type_ == STRING; }
    bool is_array() const { return type_ == ARRAY; }
    bool is_object() const { return type_ == OBJECT; }

    bool as_bool() const { return bool_; }
    double as_number() const { return number_; }
    const std::string& as_string() const { return string_; }

    // 配列
    const std::vector<Json>& items() const { return items_; }
    void push(const Json& v) { items_.push_back(v); }

    // オブジェクト(書いた順を保つ)
    const std::vector<std::pair<std::string, Json> >& members() const { return members_; }
    /// キーの値(無ければ null の値)。
    const Json& operator[](const std::string& key) const;
    bool has(const std::string& key) const;
    void set(const std::string& key, const Json& v);

private:
    Type type_;
    bool bool_;
    double number_;
    std::string string_;
    std::vector<Json> items_;
    std::vector<std::pair<std::string, Json> > members_;
};

}  // namespace ttc

#endif
