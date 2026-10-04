#include <gui/core/clipboard.hpp>

#include <SDL3/SDL.h>

namespace gui
{
namespace clipboard
{

std::string text()
{
    std::string out;
    if (char* t = SDL_GetClipboardText())
    {
        out = t;
        SDL_free(t);
    }
    return out;
}

void set_text(const std::string& text) { SDL_SetClipboardText(text.c_str()); }

bool has_text() { return SDL_HasClipboardText(); }

}  // namespace clipboard
}  // namespace gui
