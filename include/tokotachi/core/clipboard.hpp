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

TOKOTACHI_API std::string text();
TOKOTACHI_API void set_text(const std::string& text);
TOKOTACHI_API bool has_text();

}  // namespace clipboard
}  // namespace tokotachi

#endif
