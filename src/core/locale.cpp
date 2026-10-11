#include "locale.hpp"

#include <SDL3/SDL.h>

#include <cstdlib>

#include <tokotachi/core/application.hpp>

#ifdef __EMSCRIPTEN__
#include <emscripten/emscripten.h>
#endif

namespace tokotachi
{
namespace detail
{

#ifdef __EMSCRIPTEN__
// WebAssembly: SDL の SDL_GetPreferredLocales は navigator が無い環境(Node.js)で止まるので、自分で調べる。
// navigator が無ければ Intl の既定の言語にする。
EM_JS(char*, tokotachi_js_language, (), {
    var lang = '';
    try
    {
        if (typeof navigator !== 'undefined' && navigator && navigator.language)
        {
            lang = navigator.language;
        }
        else if (typeof Intl !== 'undefined')
        {
            lang = Intl.DateTimeFormat().resolvedOptions().locale || '';
        }
    }
    catch (e)
    {
        lang = '';
    }
    return stringToNewUTF8(lang);
});
#endif

std::string system_language()
{
#ifdef __EMSCRIPTEN__
    char* p = tokotachi_js_language();
    const std::string out = p ? p : "";
    std::free(p);
    return out;
#else
    int count = 0;
    SDL_Locale** locales = SDL_GetPreferredLocales(&count);
    std::string out;
    if (locales && count > 0 && locales[0] && locales[0]->language)
    {
        out = locales[0]->language;
        if (locales[0]->country)
        {
            out += std::string("-") + locales[0]->country;
        }
    }
    SDL_free(locales);
    return out;
#endif
}

bool prefers_japanese()
{
    const Application* app = Application::instance();
    const std::string lang = app ? app->language.get() : system_language();
    return lang.size() >= 2 && lang[0] == 'j' && lang[1] == 'a';
}

}  // namespace detail
}  // namespace tokotachi
