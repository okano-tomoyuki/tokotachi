// 生成したファイル(tools/canvas/table.mts がカタログから作る)。手で変えない。
#ifndef TOKOTACHI_CANVAS_GENERATED_ENUMS_HPP
#define TOKOTACHI_CANVAS_GENERATED_ENUMS_HPP

#include <tokotachi/tokotachi.hpp>

#include "../json.hpp"

namespace ttc
{

bool from_json(const Json& j, tokotachi::Align& out);
Json to_json(tokotachi::Align v);
bool from_json(const Json& j, tokotachi::Anchor& out);
Json to_json(tokotachi::Anchor v);
bool from_json(const Json& j, tokotachi::ButtonStyle& out);
Json to_json(tokotachi::ButtonStyle v);
bool from_json(const Json& j, tokotachi::ColorRole& out);
Json to_json(tokotachi::ColorRole v);
bool from_json(const Json& j, tokotachi::DialogButton& out);
Json to_json(tokotachi::DialogButton v);
bool from_json(const Json& j, tokotachi::FocusPolicy& out);
Json to_json(tokotachi::FocusPolicy v);
bool from_json(const Json& j, tokotachi::ImageFit& out);
Json to_json(tokotachi::ImageFit v);
bool from_json(const Json& j, tokotachi::MouseCursor& out);
Json to_json(tokotachi::MouseCursor v);
bool from_json(const Json& j, tokotachi::Orientation& out);
Json to_json(tokotachi::Orientation v);
bool from_json(const Json& j, tokotachi::SelectionMode& out);
Json to_json(tokotachi::SelectionMode v);
bool from_json(const Json& j, tokotachi::SortOrder& out);
Json to_json(tokotachi::SortOrder v);
bool from_json(const Json& j, tokotachi::ToolButtonStyle& out);
Json to_json(tokotachi::ToolButtonStyle v);

}  // namespace ttc

#endif
