// クリップボード(テキストのみ)。
#ifndef GUI_CLIPBOARD_HPP
#define GUI_CLIPBOARD_HPP

#include <string>

namespace gui
{
namespace clipboard
{

std::string text();
void set_text(const std::string& text);
bool has_text();

}  // namespace clipboard
}  // namespace gui

#endif
