/// @file
/// クリップボード(テキストのみ)。
#ifndef TOKOTACHI_CORE_CLIPBOARD_HPP
#define TOKOTACHI_CORE_CLIPBOARD_HPP

#include <string>

#include "export.hpp"

namespace tokotachi
{
namespace clipboard
{

/// クリップボードの文字(UTF-8。無ければ空)。
TOKOTACHI_API std::string text();
/// クリップボードに文字を入れる。
TOKOTACHI_API void set_text(const std::string& text);
/// クリップボードに文字があるか。
TOKOTACHI_API bool has_text();

}  // namespace clipboard
}  // namespace tokotachi

#endif
