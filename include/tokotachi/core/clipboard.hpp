/// @file
/// クリップボード(テキストのみ)。
#ifndef TOKOTACHI_CORE_CLIPBOARD_HPP
#define TOKOTACHI_CORE_CLIPBOARD_HPP

#include <string>

namespace tokotachi
{
namespace clipboard
{

std::string text();
void set_text(const std::string& text);
bool has_text();

}  // namespace clipboard
}  // namespace tokotachi

#endif
