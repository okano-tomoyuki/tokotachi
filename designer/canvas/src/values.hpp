// 定義ファイルの値(JSON)と Tokotachi の値の変換(docs/designer/dsl-spec.md §5 の書き方)。
// 列挙型の変換は、カタログから生成した generated/enums.hpp・table.cpp にある。
#ifndef TOKOTACHI_CANVAS_VALUES_HPP
#define TOKOTACHI_CANVAS_VALUES_HPP

#include <map>
#include <string>
#include <vector>

#include <tokotachi/tokotachi.hpp>

#include "generated/enums.hpp"
#include "json.hpp"

namespace ttc
{

bool from_json(const Json& j, bool& out);
bool from_json(const Json& j, int& out);
bool from_json(const Json& j, float& out);
bool from_json(const Json& j, double& out);
bool from_json(const Json& j, std::string& out);
bool from_json(const Json& j, std::vector<std::string>& out);
bool from_json(const Json& j, std::vector<float>& out);
bool from_json(const Json& j, tokotachi::Margins& out);
bool from_json(const Json& j, tokotachi::Size& out);
bool from_json(const Json& j, tokotachi::Point& out);
bool from_json(const Json& j, tokotachi::Rect& out);
bool from_json(const Json& j, tokotachi::Font& out);
bool from_json(const Json& j, tokotachi::Color& out);
bool from_json(const Json& j, tokotachi::Date& out);
bool from_json(const Json& j, tokotachi::ThemeColor& out);
bool from_json(const Json& j, tokotachi::Shortcut& out);
bool from_json(const Json& j, tokotachi::Placement& out);
/// テーマの上書き({"colors": {"ACCENT": "#RRGGBB" | {"light", "dark"}}, "radius": 0, ...}。docs/adr/0042)。
bool from_json(const Json& j, tokotachi::ThemeOverride& out);
/// 画像は、定義ファイルに書いたパス(正規化したもの)を set_images で渡した画像に変える。無いパスは空の画像。
bool from_json(const Json& j, tokotachi::Image& out);

Json to_json(bool v);
Json to_json(int v);
Json to_json(float v);
Json to_json(double v);
Json to_json(const std::string& v);
Json to_json(const std::vector<std::string>& v);
Json to_json(const std::vector<float>& v);
Json to_json(const tokotachi::Margins& v);
Json to_json(const tokotachi::Size& v);
Json to_json(const tokotachi::Point& v);
Json to_json(const tokotachi::Rect& v);
Json to_json(const tokotachi::Font& v);
Json to_json(const tokotachi::Color& v);
Json to_json(const tokotachi::Date& v);
Json to_json(const tokotachi::ThemeColor& v);
Json to_json(const tokotachi::Shortcut& v);
Json to_json(const tokotachi::ThemeOverride& v);
/// 画像は値で表せないので null。
Json to_json(const tokotachi::Image& v);

/// 描画に使う画像(パス → 画像)。拡張が読んだファイルの中身から作る。
void set_images(const std::map<std::string, tokotachi::Image>& images);
/// base64 の文字をバイトの並びにする。読めなければ false。
bool decode_base64(const std::string& text, std::vector<unsigned char>& out);

}  // namespace ttc

#endif
