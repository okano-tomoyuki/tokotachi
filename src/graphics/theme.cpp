#include <tokotachi/graphics/theme.hpp>

#include <tokotachi/core/application.hpp>

#include <cstdint>

#include "../widgets/theme.hpp"
#include "../core/application_impl.hpp"

namespace tokotachi
{

namespace
{

struct RoleColor
{
    ColorRole role;
    std::uint32_t rgb;
};

const RoleColor LIGHT_COLORS[COLOR_ROLE_COUNT] = {
    {ColorRole::WINDOW, 0xf5f6f8},
    {ColorRole::POPUP, 0xffffff},
    {ColorRole::TEXT, 0x111827},
    {ColorRole::TEXT_SECONDARY, 0x475569},
    {ColorRole::TEXT_DISABLED, 0x9ca3af},
    {ColorRole::PLACEHOLDER, 0x9ca3af},
    {ColorRole::ACCENT, 0x2563eb},
    {ColorRole::ACCENT_HOVER, 0x1d4ed8},
    {ColorRole::ACCENT_PRESSED, 0x1e40af},
    {ColorRole::ACCENT_DISABLED, 0xbfdbfe},
    {ColorRole::ON_ACCENT, 0xffffff},
    {ColorRole::FOCUS_RING, 0x93c5fd},
    {ColorRole::FIELD, 0xffffff},
    {ColorRole::FIELD_DISABLED, 0xf3f4f6},
    {ColorRole::BORDER, 0xc8ccd2},
    {ColorRole::SELECTION, 0x2563eb},
    {ColorRole::SELECTION_INACTIVE, 0xd1d5db},
    {ColorRole::ON_SELECTION, 0xffffff},
    {ColorRole::PREEDIT_UNDERLINE, 0x334155},
    {ColorRole::SCROLL_TRACK, 0xeef0f3},
    {ColorRole::SCROLL_THUMB, 0xb8bec7},
    {ColorRole::SCROLL_THUMB_ACTIVE, 0x8b93a0},
    {ColorRole::TOOLTIP, 0x1f2937},
    {ColorRole::ON_TOOLTIP, 0xffffff},
};

const RoleColor DARK_COLORS[COLOR_ROLE_COUNT] = {
    {ColorRole::WINDOW, 0x1e2024},
    {ColorRole::POPUP, 0x2a2d33},
    {ColorRole::TEXT, 0xe5e7eb},
    {ColorRole::TEXT_SECONDARY, 0x9ca3af},
    {ColorRole::TEXT_DISABLED, 0x6b7280},
    {ColorRole::PLACEHOLDER, 0x6b7280},
    {ColorRole::ACCENT, 0x3b82f6},
    {ColorRole::ACCENT_HOVER, 0x60a5fa},
    {ColorRole::ACCENT_PRESSED, 0x2563eb},
    {ColorRole::ACCENT_DISABLED, 0x27416b},
    {ColorRole::ON_ACCENT, 0xffffff},
    {ColorRole::FOCUS_RING, 0x60a5fa},
    {ColorRole::FIELD, 0x16181b},
    {ColorRole::FIELD_DISABLED, 0x26292e},
    {ColorRole::BORDER, 0x41464f},
    {ColorRole::SELECTION, 0x2563eb},
    {ColorRole::SELECTION_INACTIVE, 0x41464f},
    {ColorRole::ON_SELECTION, 0xffffff},
    {ColorRole::PREEDIT_UNDERLINE, 0xcbd5e1},
    {ColorRole::SCROLL_TRACK, 0x26292e},
    {ColorRole::SCROLL_THUMB, 0x4b515b},
    {ColorRole::SCROLL_THUMB_ACTIVE, 0x6b7280},
    {ColorRole::TOOLTIP, 0xe5e7eb},
    {ColorRole::ON_TOOLTIP, 0x111827},
};

void fill(Theme& t, const RoleColor (&colors)[COLOR_ROLE_COUNT])
{
    for (int i = 0; i < COLOR_ROLE_COUNT; ++i)
    {
        t[colors[i].role] = Color::rgb(colors[i].rgb);
    }
}

}  // namespace

Theme::Theme() : dark_colors(false), radius(6), field_radius(4), field_padding(8)
{
    fill(*this, LIGHT_COLORS);
}

Theme Theme::light() { return Theme(); }

Theme Theme::dark()
{
    Theme t;
    fill(t, DARK_COLORS);
    t.dark_colors = true;
    return t;
}

bool Theme::operator==(const Theme& o) const
{
    for (int i = 0; i < COLOR_ROLE_COUNT; ++i)
    {
        if (colors_[i] != o.colors_[i])
        {
            return false;
        }
    }
    return dark_colors == o.dark_colors && radius == o.radius && field_radius == o.field_radius &&
           field_padding == o.field_padding;
}

Color ThemeColor::resolve() const { return is_role_ ? detail::current_theme()[role_] : color_; }

namespace detail
{

const Theme& current_theme()
{
    if (Application* app = Application::instance())
    {
        return app->impl()->current_theme;
    }
    static const Theme fallback;
    return fallback;
}

}  // namespace detail
}  // namespace tokotachi
