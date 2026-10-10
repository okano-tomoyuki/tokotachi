// 定義ファイルの値(JSON)と Tokotachi の値の変換(docs/designer/dsl-spec.md §5 の書き方)。
// 列挙型の変換は、カタログから生成した generated/enums.hpp・table.cpp にある。
#ifndef TOKOTACHI_CANVAS_VALUES_HPP
#define TOKOTACHI_CANVAS_VALUES_HPP

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
bool from_json(const Json& j, tokotachi::ThemeColor& out);
bool from_json(const Json& j, tokotachi::Shortcut& out);
bool from_json(const Json& j, tokotachi::Placement& out);

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
Json to_json(const tokotachi::ThemeColor& v);
Json to_json(const tokotachi::Shortcut& v);

}  // namespace ttc

#endif
