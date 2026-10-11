// 生成したファイル(tools/canvas/table.mts がカタログから作る)。手で変えない。
#include "../table.hpp"
#include "../values.hpp"

namespace ttc
{

// ---- 列挙型 ----

bool from_json(const Json& j, tokotachi::Align& out)
{
    if (!j.is_string())
    {
        return false;
    }
    const std::string& s = j.as_string();
    if (s == "START")
    {
        out = tokotachi::Align::START;
        return true;
    }
    if (s == "CENTER")
    {
        out = tokotachi::Align::CENTER;
        return true;
    }
    if (s == "END")
    {
        out = tokotachi::Align::END;
        return true;
    }
    if (s == "FILL")
    {
        out = tokotachi::Align::FILL;
        return true;
    }
    return false;
}

Json to_json(tokotachi::Align v)
{
    switch (v)
    {
    case tokotachi::Align::START:
        return Json(std::string("START"));
    case tokotachi::Align::CENTER:
        return Json(std::string("CENTER"));
    case tokotachi::Align::END:
        return Json(std::string("END"));
    case tokotachi::Align::FILL:
        return Json(std::string("FILL"));
    }
    return Json();
}

bool from_json(const Json& j, tokotachi::Anchor& out)
{
    if (!j.is_string())
    {
        return false;
    }
    const std::string& s = j.as_string();
    if (s == "TOP_LEFT")
    {
        out = tokotachi::Anchor::TOP_LEFT;
        return true;
    }
    if (s == "TOP")
    {
        out = tokotachi::Anchor::TOP;
        return true;
    }
    if (s == "TOP_RIGHT")
    {
        out = tokotachi::Anchor::TOP_RIGHT;
        return true;
    }
    if (s == "LEFT")
    {
        out = tokotachi::Anchor::LEFT;
        return true;
    }
    if (s == "CENTER")
    {
        out = tokotachi::Anchor::CENTER;
        return true;
    }
    if (s == "RIGHT")
    {
        out = tokotachi::Anchor::RIGHT;
        return true;
    }
    if (s == "BOTTOM_LEFT")
    {
        out = tokotachi::Anchor::BOTTOM_LEFT;
        return true;
    }
    if (s == "BOTTOM")
    {
        out = tokotachi::Anchor::BOTTOM;
        return true;
    }
    if (s == "BOTTOM_RIGHT")
    {
        out = tokotachi::Anchor::BOTTOM_RIGHT;
        return true;
    }
    return false;
}

Json to_json(tokotachi::Anchor v)
{
    switch (v)
    {
    case tokotachi::Anchor::TOP_LEFT:
        return Json(std::string("TOP_LEFT"));
    case tokotachi::Anchor::TOP:
        return Json(std::string("TOP"));
    case tokotachi::Anchor::TOP_RIGHT:
        return Json(std::string("TOP_RIGHT"));
    case tokotachi::Anchor::LEFT:
        return Json(std::string("LEFT"));
    case tokotachi::Anchor::CENTER:
        return Json(std::string("CENTER"));
    case tokotachi::Anchor::RIGHT:
        return Json(std::string("RIGHT"));
    case tokotachi::Anchor::BOTTOM_LEFT:
        return Json(std::string("BOTTOM_LEFT"));
    case tokotachi::Anchor::BOTTOM:
        return Json(std::string("BOTTOM"));
    case tokotachi::Anchor::BOTTOM_RIGHT:
        return Json(std::string("BOTTOM_RIGHT"));
    }
    return Json();
}

bool from_json(const Json& j, tokotachi::ButtonStyle& out)
{
    if (!j.is_string())
    {
        return false;
    }
    const std::string& s = j.as_string();
    if (s == "PRIMARY")
    {
        out = tokotachi::ButtonStyle::PRIMARY;
        return true;
    }
    if (s == "SECONDARY")
    {
        out = tokotachi::ButtonStyle::SECONDARY;
        return true;
    }
    return false;
}

Json to_json(tokotachi::ButtonStyle v)
{
    switch (v)
    {
    case tokotachi::ButtonStyle::PRIMARY:
        return Json(std::string("PRIMARY"));
    case tokotachi::ButtonStyle::SECONDARY:
        return Json(std::string("SECONDARY"));
    }
    return Json();
}

bool from_json(const Json& j, tokotachi::ColorRole& out)
{
    if (!j.is_string())
    {
        return false;
    }
    const std::string& s = j.as_string();
    if (s == "WINDOW")
    {
        out = tokotachi::ColorRole::WINDOW;
        return true;
    }
    if (s == "POPUP")
    {
        out = tokotachi::ColorRole::POPUP;
        return true;
    }
    if (s == "TEXT")
    {
        out = tokotachi::ColorRole::TEXT;
        return true;
    }
    if (s == "TEXT_SECONDARY")
    {
        out = tokotachi::ColorRole::TEXT_SECONDARY;
        return true;
    }
    if (s == "TEXT_DISABLED")
    {
        out = tokotachi::ColorRole::TEXT_DISABLED;
        return true;
    }
    if (s == "PLACEHOLDER")
    {
        out = tokotachi::ColorRole::PLACEHOLDER;
        return true;
    }
    if (s == "ACCENT")
    {
        out = tokotachi::ColorRole::ACCENT;
        return true;
    }
    if (s == "ACCENT_HOVER")
    {
        out = tokotachi::ColorRole::ACCENT_HOVER;
        return true;
    }
    if (s == "ACCENT_PRESSED")
    {
        out = tokotachi::ColorRole::ACCENT_PRESSED;
        return true;
    }
    if (s == "ACCENT_DISABLED")
    {
        out = tokotachi::ColorRole::ACCENT_DISABLED;
        return true;
    }
    if (s == "ON_ACCENT")
    {
        out = tokotachi::ColorRole::ON_ACCENT;
        return true;
    }
    if (s == "FOCUS_RING")
    {
        out = tokotachi::ColorRole::FOCUS_RING;
        return true;
    }
    if (s == "FIELD")
    {
        out = tokotachi::ColorRole::FIELD;
        return true;
    }
    if (s == "FIELD_DISABLED")
    {
        out = tokotachi::ColorRole::FIELD_DISABLED;
        return true;
    }
    if (s == "BORDER")
    {
        out = tokotachi::ColorRole::BORDER;
        return true;
    }
    if (s == "SELECTION")
    {
        out = tokotachi::ColorRole::SELECTION;
        return true;
    }
    if (s == "SELECTION_INACTIVE")
    {
        out = tokotachi::ColorRole::SELECTION_INACTIVE;
        return true;
    }
    if (s == "ON_SELECTION")
    {
        out = tokotachi::ColorRole::ON_SELECTION;
        return true;
    }
    if (s == "PREEDIT_UNDERLINE")
    {
        out = tokotachi::ColorRole::PREEDIT_UNDERLINE;
        return true;
    }
    if (s == "SCROLL_TRACK")
    {
        out = tokotachi::ColorRole::SCROLL_TRACK;
        return true;
    }
    if (s == "SCROLL_THUMB")
    {
        out = tokotachi::ColorRole::SCROLL_THUMB;
        return true;
    }
    if (s == "SCROLL_THUMB_ACTIVE")
    {
        out = tokotachi::ColorRole::SCROLL_THUMB_ACTIVE;
        return true;
    }
    if (s == "TOOLTIP")
    {
        out = tokotachi::ColorRole::TOOLTIP;
        return true;
    }
    if (s == "ON_TOOLTIP")
    {
        out = tokotachi::ColorRole::ON_TOOLTIP;
        return true;
    }
    return false;
}

Json to_json(tokotachi::ColorRole v)
{
    switch (v)
    {
    case tokotachi::ColorRole::WINDOW:
        return Json(std::string("WINDOW"));
    case tokotachi::ColorRole::POPUP:
        return Json(std::string("POPUP"));
    case tokotachi::ColorRole::TEXT:
        return Json(std::string("TEXT"));
    case tokotachi::ColorRole::TEXT_SECONDARY:
        return Json(std::string("TEXT_SECONDARY"));
    case tokotachi::ColorRole::TEXT_DISABLED:
        return Json(std::string("TEXT_DISABLED"));
    case tokotachi::ColorRole::PLACEHOLDER:
        return Json(std::string("PLACEHOLDER"));
    case tokotachi::ColorRole::ACCENT:
        return Json(std::string("ACCENT"));
    case tokotachi::ColorRole::ACCENT_HOVER:
        return Json(std::string("ACCENT_HOVER"));
    case tokotachi::ColorRole::ACCENT_PRESSED:
        return Json(std::string("ACCENT_PRESSED"));
    case tokotachi::ColorRole::ACCENT_DISABLED:
        return Json(std::string("ACCENT_DISABLED"));
    case tokotachi::ColorRole::ON_ACCENT:
        return Json(std::string("ON_ACCENT"));
    case tokotachi::ColorRole::FOCUS_RING:
        return Json(std::string("FOCUS_RING"));
    case tokotachi::ColorRole::FIELD:
        return Json(std::string("FIELD"));
    case tokotachi::ColorRole::FIELD_DISABLED:
        return Json(std::string("FIELD_DISABLED"));
    case tokotachi::ColorRole::BORDER:
        return Json(std::string("BORDER"));
    case tokotachi::ColorRole::SELECTION:
        return Json(std::string("SELECTION"));
    case tokotachi::ColorRole::SELECTION_INACTIVE:
        return Json(std::string("SELECTION_INACTIVE"));
    case tokotachi::ColorRole::ON_SELECTION:
        return Json(std::string("ON_SELECTION"));
    case tokotachi::ColorRole::PREEDIT_UNDERLINE:
        return Json(std::string("PREEDIT_UNDERLINE"));
    case tokotachi::ColorRole::SCROLL_TRACK:
        return Json(std::string("SCROLL_TRACK"));
    case tokotachi::ColorRole::SCROLL_THUMB:
        return Json(std::string("SCROLL_THUMB"));
    case tokotachi::ColorRole::SCROLL_THUMB_ACTIVE:
        return Json(std::string("SCROLL_THUMB_ACTIVE"));
    case tokotachi::ColorRole::TOOLTIP:
        return Json(std::string("TOOLTIP"));
    case tokotachi::ColorRole::ON_TOOLTIP:
        return Json(std::string("ON_TOOLTIP"));
    }
    return Json();
}

bool from_json(const Json& j, tokotachi::DialogButton& out)
{
    if (!j.is_string())
    {
        return false;
    }
    const std::string& s = j.as_string();
    if (s == "NONE")
    {
        out = tokotachi::DialogButton::NONE;
        return true;
    }
    if (s == "OK")
    {
        out = tokotachi::DialogButton::OK;
        return true;
    }
    if (s == "YES")
    {
        out = tokotachi::DialogButton::YES;
        return true;
    }
    if (s == "SAVE")
    {
        out = tokotachi::DialogButton::SAVE;
        return true;
    }
    if (s == "NO")
    {
        out = tokotachi::DialogButton::NO;
        return true;
    }
    if (s == "DISCARD")
    {
        out = tokotachi::DialogButton::DISCARD;
        return true;
    }
    if (s == "CANCEL")
    {
        out = tokotachi::DialogButton::CANCEL;
        return true;
    }
    if (s == "CLOSE")
    {
        out = tokotachi::DialogButton::CLOSE;
        return true;
    }
    return false;
}

Json to_json(tokotachi::DialogButton v)
{
    switch (v)
    {
    case tokotachi::DialogButton::NONE:
        return Json(std::string("NONE"));
    case tokotachi::DialogButton::OK:
        return Json(std::string("OK"));
    case tokotachi::DialogButton::YES:
        return Json(std::string("YES"));
    case tokotachi::DialogButton::SAVE:
        return Json(std::string("SAVE"));
    case tokotachi::DialogButton::NO:
        return Json(std::string("NO"));
    case tokotachi::DialogButton::DISCARD:
        return Json(std::string("DISCARD"));
    case tokotachi::DialogButton::CANCEL:
        return Json(std::string("CANCEL"));
    case tokotachi::DialogButton::CLOSE:
        return Json(std::string("CLOSE"));
    }
    return Json();
}

bool from_json(const Json& j, tokotachi::FocusPolicy& out)
{
    if (!j.is_string())
    {
        return false;
    }
    const std::string& s = j.as_string();
    if (s == "NONE")
    {
        out = tokotachi::FocusPolicy::NONE;
        return true;
    }
    if (s == "TAB")
    {
        out = tokotachi::FocusPolicy::TAB;
        return true;
    }
    if (s == "CLICK")
    {
        out = tokotachi::FocusPolicy::CLICK;
        return true;
    }
    if (s == "TAB_AND_CLICK")
    {
        out = tokotachi::FocusPolicy::TAB_AND_CLICK;
        return true;
    }
    return false;
}

Json to_json(tokotachi::FocusPolicy v)
{
    switch (v)
    {
    case tokotachi::FocusPolicy::NONE:
        return Json(std::string("NONE"));
    case tokotachi::FocusPolicy::TAB:
        return Json(std::string("TAB"));
    case tokotachi::FocusPolicy::CLICK:
        return Json(std::string("CLICK"));
    case tokotachi::FocusPolicy::TAB_AND_CLICK:
        return Json(std::string("TAB_AND_CLICK"));
    }
    return Json();
}

bool from_json(const Json& j, tokotachi::ImageFit& out)
{
    if (!j.is_string())
    {
        return false;
    }
    const std::string& s = j.as_string();
    if (s == "NONE")
    {
        out = tokotachi::ImageFit::NONE;
        return true;
    }
    if (s == "CONTAIN")
    {
        out = tokotachi::ImageFit::CONTAIN;
        return true;
    }
    if (s == "COVER")
    {
        out = tokotachi::ImageFit::COVER;
        return true;
    }
    if (s == "STRETCH")
    {
        out = tokotachi::ImageFit::STRETCH;
        return true;
    }
    return false;
}

Json to_json(tokotachi::ImageFit v)
{
    switch (v)
    {
    case tokotachi::ImageFit::NONE:
        return Json(std::string("NONE"));
    case tokotachi::ImageFit::CONTAIN:
        return Json(std::string("CONTAIN"));
    case tokotachi::ImageFit::COVER:
        return Json(std::string("COVER"));
    case tokotachi::ImageFit::STRETCH:
        return Json(std::string("STRETCH"));
    }
    return Json();
}

bool from_json(const Json& j, tokotachi::MouseCursor& out)
{
    if (!j.is_string())
    {
        return false;
    }
    const std::string& s = j.as_string();
    if (s == "ARROW")
    {
        out = tokotachi::MouseCursor::ARROW;
        return true;
    }
    if (s == "IBEAM")
    {
        out = tokotachi::MouseCursor::IBEAM;
        return true;
    }
    if (s == "POINTING_HAND")
    {
        out = tokotachi::MouseCursor::POINTING_HAND;
        return true;
    }
    if (s == "WAIT")
    {
        out = tokotachi::MouseCursor::WAIT;
        return true;
    }
    if (s == "CROSSHAIR")
    {
        out = tokotachi::MouseCursor::CROSSHAIR;
        return true;
    }
    if (s == "SIZE_HORIZONTAL")
    {
        out = tokotachi::MouseCursor::SIZE_HORIZONTAL;
        return true;
    }
    if (s == "SIZE_VERTICAL")
    {
        out = tokotachi::MouseCursor::SIZE_VERTICAL;
        return true;
    }
    return false;
}

Json to_json(tokotachi::MouseCursor v)
{
    switch (v)
    {
    case tokotachi::MouseCursor::ARROW:
        return Json(std::string("ARROW"));
    case tokotachi::MouseCursor::IBEAM:
        return Json(std::string("IBEAM"));
    case tokotachi::MouseCursor::POINTING_HAND:
        return Json(std::string("POINTING_HAND"));
    case tokotachi::MouseCursor::WAIT:
        return Json(std::string("WAIT"));
    case tokotachi::MouseCursor::CROSSHAIR:
        return Json(std::string("CROSSHAIR"));
    case tokotachi::MouseCursor::SIZE_HORIZONTAL:
        return Json(std::string("SIZE_HORIZONTAL"));
    case tokotachi::MouseCursor::SIZE_VERTICAL:
        return Json(std::string("SIZE_VERTICAL"));
    }
    return Json();
}

bool from_json(const Json& j, tokotachi::Orientation& out)
{
    if (!j.is_string())
    {
        return false;
    }
    const std::string& s = j.as_string();
    if (s == "HORIZONTAL")
    {
        out = tokotachi::Orientation::HORIZONTAL;
        return true;
    }
    if (s == "VERTICAL")
    {
        out = tokotachi::Orientation::VERTICAL;
        return true;
    }
    return false;
}

Json to_json(tokotachi::Orientation v)
{
    switch (v)
    {
    case tokotachi::Orientation::HORIZONTAL:
        return Json(std::string("HORIZONTAL"));
    case tokotachi::Orientation::VERTICAL:
        return Json(std::string("VERTICAL"));
    }
    return Json();
}

bool from_json(const Json& j, tokotachi::SelectionMode& out)
{
    if (!j.is_string())
    {
        return false;
    }
    const std::string& s = j.as_string();
    if (s == "NONE")
    {
        out = tokotachi::SelectionMode::NONE;
        return true;
    }
    if (s == "SINGLE")
    {
        out = tokotachi::SelectionMode::SINGLE;
        return true;
    }
    if (s == "MULTI")
    {
        out = tokotachi::SelectionMode::MULTI;
        return true;
    }
    return false;
}

Json to_json(tokotachi::SelectionMode v)
{
    switch (v)
    {
    case tokotachi::SelectionMode::NONE:
        return Json(std::string("NONE"));
    case tokotachi::SelectionMode::SINGLE:
        return Json(std::string("SINGLE"));
    case tokotachi::SelectionMode::MULTI:
        return Json(std::string("MULTI"));
    }
    return Json();
}

bool from_json(const Json& j, tokotachi::SortOrder& out)
{
    if (!j.is_string())
    {
        return false;
    }
    const std::string& s = j.as_string();
    if (s == "ASCENDING")
    {
        out = tokotachi::SortOrder::ASCENDING;
        return true;
    }
    if (s == "DESCENDING")
    {
        out = tokotachi::SortOrder::DESCENDING;
        return true;
    }
    return false;
}

Json to_json(tokotachi::SortOrder v)
{
    switch (v)
    {
    case tokotachi::SortOrder::ASCENDING:
        return Json(std::string("ASCENDING"));
    case tokotachi::SortOrder::DESCENDING:
        return Json(std::string("DESCENDING"));
    }
    return Json();
}

bool from_json(const Json& j, tokotachi::ToolButtonStyle& out)
{
    if (!j.is_string())
    {
        return false;
    }
    const std::string& s = j.as_string();
    if (s == "ICON_ONLY")
    {
        out = tokotachi::ToolButtonStyle::ICON_ONLY;
        return true;
    }
    if (s == "TEXT_BESIDE_ICON")
    {
        out = tokotachi::ToolButtonStyle::TEXT_BESIDE_ICON;
        return true;
    }
    if (s == "TEXT_UNDER_ICON")
    {
        out = tokotachi::ToolButtonStyle::TEXT_UNDER_ICON;
        return true;
    }
    return false;
}

Json to_json(tokotachi::ToolButtonStyle v)
{
    switch (v)
    {
    case tokotachi::ToolButtonStyle::ICON_ONLY:
        return Json(std::string("ICON_ONLY"));
    case tokotachi::ToolButtonStyle::TEXT_BESIDE_ICON:
        return Json(std::string("TEXT_BESIDE_ICON"));
    case tokotachi::ToolButtonStyle::TEXT_UNDER_ICON:
        return Json(std::string("TEXT_UNDER_ICON"));
    }
    return Json();
}

namespace
{

bool set_Window(tokotachi::Window* o, const std::string& p, const Json& v)
{
    if (p == "x")
    {
        float t;
        if (!from_json(v, t))
        {
            return false;
        }
        o->x = t;
        return true;
    }
    if (p == "y")
    {
        float t;
        if (!from_json(v, t))
        {
            return false;
        }
        o->y = t;
        return true;
    }
    if (p == "width")
    {
        float t;
        if (!from_json(v, t))
        {
            return false;
        }
        o->width = t;
        return true;
    }
    if (p == "height")
    {
        float t;
        if (!from_json(v, t))
        {
            return false;
        }
        o->height = t;
        return true;
    }
    if (p == "minimum_size")
    {
        tokotachi::Size t;
        if (!from_json(v, t))
        {
            return false;
        }
        o->minimum_size = t;
        return true;
    }
    if (p == "maximum_size")
    {
        tokotachi::Size t;
        if (!from_json(v, t))
        {
            return false;
        }
        o->maximum_size = t;
        return true;
    }
    if (p == "visible")
    {
        bool t;
        if (!from_json(v, t))
        {
            return false;
        }
        o->visible = t;
        return true;
    }
    if (p == "enabled")
    {
        bool t;
        if (!from_json(v, t))
        {
            return false;
        }
        o->enabled = t;
        return true;
    }
    if (p == "focus_policy")
    {
        tokotachi::FocusPolicy t;
        if (!from_json(v, t))
        {
            return false;
        }
        o->focus_policy = t;
        return true;
    }
    if (p == "cursor")
    {
        tokotachi::MouseCursor t;
        if (!from_json(v, t))
        {
            return false;
        }
        o->cursor = t;
        return true;
    }
    if (p == "tooltip")
    {
        std::string t;
        if (!from_json(v, t))
        {
            return false;
        }
        o->tooltip = t;
        return true;
    }
    if (p == "theme_override")
    {
        tokotachi::ThemeOverride t;
        if (!from_json(v, t))
        {
            return false;
        }
        o->theme_override = t;
        return true;
    }
    if (p == "accepts_drops")
    {
        bool t;
        if (!from_json(v, t))
        {
            return false;
        }
        o->accepts_drops = t;
        return true;
    }
    if (p == "title")
    {
        std::string t;
        if (!from_json(v, t))
        {
            return false;
        }
        o->title = t;
        return true;
    }
    if (p == "background")
    {
        tokotachi::ThemeColor t;
        if (!from_json(v, t))
        {
            return false;
        }
        o->background = t;
        return true;
    }
    return false;
}

bool get_Window(tokotachi::Window* o, const std::string& p, Json& out)
{
    if (p == "x")
    {
        out = to_json(o->x.get());
        return true;
    }
    if (p == "y")
    {
        out = to_json(o->y.get());
        return true;
    }
    if (p == "width")
    {
        out = to_json(o->width.get());
        return true;
    }
    if (p == "height")
    {
        out = to_json(o->height.get());
        return true;
    }
    if (p == "minimum_size")
    {
        out = to_json(o->minimum_size.get());
        return true;
    }
    if (p == "maximum_size")
    {
        out = to_json(o->maximum_size.get());
        return true;
    }
    if (p == "visible")
    {
        out = to_json(o->visible.get());
        return true;
    }
    if (p == "enabled")
    {
        out = to_json(o->enabled.get());
        return true;
    }
    if (p == "focus_policy")
    {
        out = to_json(o->focus_policy.get());
        return true;
    }
    if (p == "cursor")
    {
        out = to_json(o->cursor.get());
        return true;
    }
    if (p == "tooltip")
    {
        out = to_json(o->tooltip.get());
        return true;
    }
    if (p == "theme_override")
    {
        out = to_json(o->theme_override.get());
        return true;
    }
    if (p == "accepts_drops")
    {
        out = to_json(o->accepts_drops.get());
        return true;
    }
    if (p == "title")
    {
        out = to_json(o->title.get());
        return true;
    }
    if (p == "background")
    {
        out = to_json(o->background.get());
        return true;
    }
    return false;
}

bool set_Dialog(tokotachi::Dialog* o, const std::string& p, const Json& v)
{
    if (p == "x")
    {
        float t;
        if (!from_json(v, t))
        {
            return false;
        }
        o->x = t;
        return true;
    }
    if (p == "y")
    {
        float t;
        if (!from_json(v, t))
        {
            return false;
        }
        o->y = t;
        return true;
    }
    if (p == "width")
    {
        float t;
        if (!from_json(v, t))
        {
            return false;
        }
        o->width = t;
        return true;
    }
    if (p == "height")
    {
        float t;
        if (!from_json(v, t))
        {
            return false;
        }
        o->height = t;
        return true;
    }
    if (p == "minimum_size")
    {
        tokotachi::Size t;
        if (!from_json(v, t))
        {
            return false;
        }
        o->minimum_size = t;
        return true;
    }
    if (p == "maximum_size")
    {
        tokotachi::Size t;
        if (!from_json(v, t))
        {
            return false;
        }
        o->maximum_size = t;
        return true;
    }
    if (p == "visible")
    {
        bool t;
        if (!from_json(v, t))
        {
            return false;
        }
        o->visible = t;
        return true;
    }
    if (p == "enabled")
    {
        bool t;
        if (!from_json(v, t))
        {
            return false;
        }
        o->enabled = t;
        return true;
    }
    if (p == "focus_policy")
    {
        tokotachi::FocusPolicy t;
        if (!from_json(v, t))
        {
            return false;
        }
        o->focus_policy = t;
        return true;
    }
    if (p == "cursor")
    {
        tokotachi::MouseCursor t;
        if (!from_json(v, t))
        {
            return false;
        }
        o->cursor = t;
        return true;
    }
    if (p == "tooltip")
    {
        std::string t;
        if (!from_json(v, t))
        {
            return false;
        }
        o->tooltip = t;
        return true;
    }
    if (p == "theme_override")
    {
        tokotachi::ThemeOverride t;
        if (!from_json(v, t))
        {
            return false;
        }
        o->theme_override = t;
        return true;
    }
    if (p == "accepts_drops")
    {
        bool t;
        if (!from_json(v, t))
        {
            return false;
        }
        o->accepts_drops = t;
        return true;
    }
    if (p == "title")
    {
        std::string t;
        if (!from_json(v, t))
        {
            return false;
        }
        o->title = t;
        return true;
    }
    if (p == "background")
    {
        tokotachi::ThemeColor t;
        if (!from_json(v, t))
        {
            return false;
        }
        o->background = t;
        return true;
    }
    return false;
}

bool get_Dialog(tokotachi::Dialog* o, const std::string& p, Json& out)
{
    if (p == "x")
    {
        out = to_json(o->x.get());
        return true;
    }
    if (p == "y")
    {
        out = to_json(o->y.get());
        return true;
    }
    if (p == "width")
    {
        out = to_json(o->width.get());
        return true;
    }
    if (p == "height")
    {
        out = to_json(o->height.get());
        return true;
    }
    if (p == "minimum_size")
    {
        out = to_json(o->minimum_size.get());
        return true;
    }
    if (p == "maximum_size")
    {
        out = to_json(o->maximum_size.get());
        return true;
    }
    if (p == "visible")
    {
        out = to_json(o->visible.get());
        return true;
    }
    if (p == "enabled")
    {
        out = to_json(o->enabled.get());
        return true;
    }
    if (p == "focus_policy")
    {
        out = to_json(o->focus_policy.get());
        return true;
    }
    if (p == "cursor")
    {
        out = to_json(o->cursor.get());
        return true;
    }
    if (p == "tooltip")
    {
        out = to_json(o->tooltip.get());
        return true;
    }
    if (p == "theme_override")
    {
        out = to_json(o->theme_override.get());
        return true;
    }
    if (p == "accepts_drops")
    {
        out = to_json(o->accepts_drops.get());
        return true;
    }
    if (p == "title")
    {
        out = to_json(o->title.get());
        return true;
    }
    if (p == "background")
    {
        out = to_json(o->background.get());
        return true;
    }
    return false;
}

bool set_Button(tokotachi::Button* o, const std::string& p, const Json& v)
{
    if (p == "x")
    {
        float t;
        if (!from_json(v, t))
        {
            return false;
        }
        o->x = t;
        return true;
    }
    if (p == "y")
    {
        float t;
        if (!from_json(v, t))
        {
            return false;
        }
        o->y = t;
        return true;
    }
    if (p == "width")
    {
        float t;
        if (!from_json(v, t))
        {
            return false;
        }
        o->width = t;
        return true;
    }
    if (p == "height")
    {
        float t;
        if (!from_json(v, t))
        {
            return false;
        }
        o->height = t;
        return true;
    }
    if (p == "minimum_size")
    {
        tokotachi::Size t;
        if (!from_json(v, t))
        {
            return false;
        }
        o->minimum_size = t;
        return true;
    }
    if (p == "maximum_size")
    {
        tokotachi::Size t;
        if (!from_json(v, t))
        {
            return false;
        }
        o->maximum_size = t;
        return true;
    }
    if (p == "visible")
    {
        bool t;
        if (!from_json(v, t))
        {
            return false;
        }
        o->visible = t;
        return true;
    }
    if (p == "enabled")
    {
        bool t;
        if (!from_json(v, t))
        {
            return false;
        }
        o->enabled = t;
        return true;
    }
    if (p == "focus_policy")
    {
        tokotachi::FocusPolicy t;
        if (!from_json(v, t))
        {
            return false;
        }
        o->focus_policy = t;
        return true;
    }
    if (p == "cursor")
    {
        tokotachi::MouseCursor t;
        if (!from_json(v, t))
        {
            return false;
        }
        o->cursor = t;
        return true;
    }
    if (p == "tooltip")
    {
        std::string t;
        if (!from_json(v, t))
        {
            return false;
        }
        o->tooltip = t;
        return true;
    }
    if (p == "theme_override")
    {
        tokotachi::ThemeOverride t;
        if (!from_json(v, t))
        {
            return false;
        }
        o->theme_override = t;
        return true;
    }
    if (p == "accepts_drops")
    {
        bool t;
        if (!from_json(v, t))
        {
            return false;
        }
        o->accepts_drops = t;
        return true;
    }
    if (p == "text")
    {
        std::string t;
        if (!from_json(v, t))
        {
            return false;
        }
        o->text = t;
        return true;
    }
    if (p == "font")
    {
        tokotachi::Font t;
        if (!from_json(v, t))
        {
            return false;
        }
        o->font = t;
        return true;
    }
    if (p == "style")
    {
        tokotachi::ButtonStyle t;
        if (!from_json(v, t))
        {
            return false;
        }
        o->style = t;
        return true;
    }
    if (p == "icon")
    {
        tokotachi::Image t;
        if (!from_json(v, t))
        {
            return false;
        }
        o->icon = t;
        return true;
    }
    if (p == "icon_size")
    {
        float t;
        if (!from_json(v, t))
        {
            return false;
        }
        o->icon_size = t;
        return true;
    }
    return false;
}

bool get_Button(tokotachi::Button* o, const std::string& p, Json& out)
{
    if (p == "x")
    {
        out = to_json(o->x.get());
        return true;
    }
    if (p == "y")
    {
        out = to_json(o->y.get());
        return true;
    }
    if (p == "width")
    {
        out = to_json(o->width.get());
        return true;
    }
    if (p == "height")
    {
        out = to_json(o->height.get());
        return true;
    }
    if (p == "minimum_size")
    {
        out = to_json(o->minimum_size.get());
        return true;
    }
    if (p == "maximum_size")
    {
        out = to_json(o->maximum_size.get());
        return true;
    }
    if (p == "visible")
    {
        out = to_json(o->visible.get());
        return true;
    }
    if (p == "enabled")
    {
        out = to_json(o->enabled.get());
        return true;
    }
    if (p == "focus_policy")
    {
        out = to_json(o->focus_policy.get());
        return true;
    }
    if (p == "cursor")
    {
        out = to_json(o->cursor.get());
        return true;
    }
    if (p == "tooltip")
    {
        out = to_json(o->tooltip.get());
        return true;
    }
    if (p == "theme_override")
    {
        out = to_json(o->theme_override.get());
        return true;
    }
    if (p == "accepts_drops")
    {
        out = to_json(o->accepts_drops.get());
        return true;
    }
    if (p == "text")
    {
        out = to_json(o->text.get());
        return true;
    }
    if (p == "font")
    {
        out = to_json(o->font.get());
        return true;
    }
    if (p == "style")
    {
        out = to_json(o->style.get());
        return true;
    }
    if (p == "icon")
    {
        out = to_json(o->icon.get());
        return true;
    }
    if (p == "icon_size")
    {
        out = to_json(o->icon_size.get());
        return true;
    }
    return false;
}

bool set_CheckBox(tokotachi::CheckBox* o, const std::string& p, const Json& v)
{
    if (p == "x")
    {
        float t;
        if (!from_json(v, t))
        {
            return false;
        }
        o->x = t;
        return true;
    }
    if (p == "y")
    {
        float t;
        if (!from_json(v, t))
        {
            return false;
        }
        o->y = t;
        return true;
    }
    if (p == "width")
    {
        float t;
        if (!from_json(v, t))
        {
            return false;
        }
        o->width = t;
        return true;
    }
    if (p == "height")
    {
        float t;
        if (!from_json(v, t))
        {
            return false;
        }
        o->height = t;
        return true;
    }
    if (p == "minimum_size")
    {
        tokotachi::Size t;
        if (!from_json(v, t))
        {
            return false;
        }
        o->minimum_size = t;
        return true;
    }
    if (p == "maximum_size")
    {
        tokotachi::Size t;
        if (!from_json(v, t))
        {
            return false;
        }
        o->maximum_size = t;
        return true;
    }
    if (p == "visible")
    {
        bool t;
        if (!from_json(v, t))
        {
            return false;
        }
        o->visible = t;
        return true;
    }
    if (p == "enabled")
    {
        bool t;
        if (!from_json(v, t))
        {
            return false;
        }
        o->enabled = t;
        return true;
    }
    if (p == "focus_policy")
    {
        tokotachi::FocusPolicy t;
        if (!from_json(v, t))
        {
            return false;
        }
        o->focus_policy = t;
        return true;
    }
    if (p == "cursor")
    {
        tokotachi::MouseCursor t;
        if (!from_json(v, t))
        {
            return false;
        }
        o->cursor = t;
        return true;
    }
    if (p == "tooltip")
    {
        std::string t;
        if (!from_json(v, t))
        {
            return false;
        }
        o->tooltip = t;
        return true;
    }
    if (p == "theme_override")
    {
        tokotachi::ThemeOverride t;
        if (!from_json(v, t))
        {
            return false;
        }
        o->theme_override = t;
        return true;
    }
    if (p == "accepts_drops")
    {
        bool t;
        if (!from_json(v, t))
        {
            return false;
        }
        o->accepts_drops = t;
        return true;
    }
    if (p == "text")
    {
        std::string t;
        if (!from_json(v, t))
        {
            return false;
        }
        o->text = t;
        return true;
    }
    if (p == "font")
    {
        tokotachi::Font t;
        if (!from_json(v, t))
        {
            return false;
        }
        o->font = t;
        return true;
    }
    if (p == "checked")
    {
        bool t;
        if (!from_json(v, t))
        {
            return false;
        }
        o->checked = t;
        return true;
    }
    return false;
}

bool get_CheckBox(tokotachi::CheckBox* o, const std::string& p, Json& out)
{
    if (p == "x")
    {
        out = to_json(o->x.get());
        return true;
    }
    if (p == "y")
    {
        out = to_json(o->y.get());
        return true;
    }
    if (p == "width")
    {
        out = to_json(o->width.get());
        return true;
    }
    if (p == "height")
    {
        out = to_json(o->height.get());
        return true;
    }
    if (p == "minimum_size")
    {
        out = to_json(o->minimum_size.get());
        return true;
    }
    if (p == "maximum_size")
    {
        out = to_json(o->maximum_size.get());
        return true;
    }
    if (p == "visible")
    {
        out = to_json(o->visible.get());
        return true;
    }
    if (p == "enabled")
    {
        out = to_json(o->enabled.get());
        return true;
    }
    if (p == "focus_policy")
    {
        out = to_json(o->focus_policy.get());
        return true;
    }
    if (p == "cursor")
    {
        out = to_json(o->cursor.get());
        return true;
    }
    if (p == "tooltip")
    {
        out = to_json(o->tooltip.get());
        return true;
    }
    if (p == "theme_override")
    {
        out = to_json(o->theme_override.get());
        return true;
    }
    if (p == "accepts_drops")
    {
        out = to_json(o->accepts_drops.get());
        return true;
    }
    if (p == "text")
    {
        out = to_json(o->text.get());
        return true;
    }
    if (p == "font")
    {
        out = to_json(o->font.get());
        return true;
    }
    if (p == "checked")
    {
        out = to_json(o->checked.get());
        return true;
    }
    return false;
}

bool set_RadioButton(tokotachi::RadioButton* o, const std::string& p, const Json& v)
{
    if (p == "x")
    {
        float t;
        if (!from_json(v, t))
        {
            return false;
        }
        o->x = t;
        return true;
    }
    if (p == "y")
    {
        float t;
        if (!from_json(v, t))
        {
            return false;
        }
        o->y = t;
        return true;
    }
    if (p == "width")
    {
        float t;
        if (!from_json(v, t))
        {
            return false;
        }
        o->width = t;
        return true;
    }
    if (p == "height")
    {
        float t;
        if (!from_json(v, t))
        {
            return false;
        }
        o->height = t;
        return true;
    }
    if (p == "minimum_size")
    {
        tokotachi::Size t;
        if (!from_json(v, t))
        {
            return false;
        }
        o->minimum_size = t;
        return true;
    }
    if (p == "maximum_size")
    {
        tokotachi::Size t;
        if (!from_json(v, t))
        {
            return false;
        }
        o->maximum_size = t;
        return true;
    }
    if (p == "visible")
    {
        bool t;
        if (!from_json(v, t))
        {
            return false;
        }
        o->visible = t;
        return true;
    }
    if (p == "enabled")
    {
        bool t;
        if (!from_json(v, t))
        {
            return false;
        }
        o->enabled = t;
        return true;
    }
    if (p == "focus_policy")
    {
        tokotachi::FocusPolicy t;
        if (!from_json(v, t))
        {
            return false;
        }
        o->focus_policy = t;
        return true;
    }
    if (p == "cursor")
    {
        tokotachi::MouseCursor t;
        if (!from_json(v, t))
        {
            return false;
        }
        o->cursor = t;
        return true;
    }
    if (p == "tooltip")
    {
        std::string t;
        if (!from_json(v, t))
        {
            return false;
        }
        o->tooltip = t;
        return true;
    }
    if (p == "theme_override")
    {
        tokotachi::ThemeOverride t;
        if (!from_json(v, t))
        {
            return false;
        }
        o->theme_override = t;
        return true;
    }
    if (p == "accepts_drops")
    {
        bool t;
        if (!from_json(v, t))
        {
            return false;
        }
        o->accepts_drops = t;
        return true;
    }
    if (p == "text")
    {
        std::string t;
        if (!from_json(v, t))
        {
            return false;
        }
        o->text = t;
        return true;
    }
    if (p == "font")
    {
        tokotachi::Font t;
        if (!from_json(v, t))
        {
            return false;
        }
        o->font = t;
        return true;
    }
    if (p == "value")
    {
        int t;
        if (!from_json(v, t))
        {
            return false;
        }
        o->value = t;
        return true;
    }
    if (p == "group_value")
    {
        int t;
        if (!from_json(v, t))
        {
            return false;
        }
        o->group_value = t;
        return true;
    }
    return false;
}

bool get_RadioButton(tokotachi::RadioButton* o, const std::string& p, Json& out)
{
    if (p == "x")
    {
        out = to_json(o->x.get());
        return true;
    }
    if (p == "y")
    {
        out = to_json(o->y.get());
        return true;
    }
    if (p == "width")
    {
        out = to_json(o->width.get());
        return true;
    }
    if (p == "height")
    {
        out = to_json(o->height.get());
        return true;
    }
    if (p == "minimum_size")
    {
        out = to_json(o->minimum_size.get());
        return true;
    }
    if (p == "maximum_size")
    {
        out = to_json(o->maximum_size.get());
        return true;
    }
    if (p == "visible")
    {
        out = to_json(o->visible.get());
        return true;
    }
    if (p == "enabled")
    {
        out = to_json(o->enabled.get());
        return true;
    }
    if (p == "focus_policy")
    {
        out = to_json(o->focus_policy.get());
        return true;
    }
    if (p == "cursor")
    {
        out = to_json(o->cursor.get());
        return true;
    }
    if (p == "tooltip")
    {
        out = to_json(o->tooltip.get());
        return true;
    }
    if (p == "theme_override")
    {
        out = to_json(o->theme_override.get());
        return true;
    }
    if (p == "accepts_drops")
    {
        out = to_json(o->accepts_drops.get());
        return true;
    }
    if (p == "text")
    {
        out = to_json(o->text.get());
        return true;
    }
    if (p == "font")
    {
        out = to_json(o->font.get());
        return true;
    }
    if (p == "value")
    {
        out = to_json(o->value.get());
        return true;
    }
    if (p == "group_value")
    {
        out = to_json(o->group_value.get());
        return true;
    }
    return false;
}

bool set_LineEdit(tokotachi::LineEdit* o, const std::string& p, const Json& v)
{
    if (p == "x")
    {
        float t;
        if (!from_json(v, t))
        {
            return false;
        }
        o->x = t;
        return true;
    }
    if (p == "y")
    {
        float t;
        if (!from_json(v, t))
        {
            return false;
        }
        o->y = t;
        return true;
    }
    if (p == "width")
    {
        float t;
        if (!from_json(v, t))
        {
            return false;
        }
        o->width = t;
        return true;
    }
    if (p == "height")
    {
        float t;
        if (!from_json(v, t))
        {
            return false;
        }
        o->height = t;
        return true;
    }
    if (p == "minimum_size")
    {
        tokotachi::Size t;
        if (!from_json(v, t))
        {
            return false;
        }
        o->minimum_size = t;
        return true;
    }
    if (p == "maximum_size")
    {
        tokotachi::Size t;
        if (!from_json(v, t))
        {
            return false;
        }
        o->maximum_size = t;
        return true;
    }
    if (p == "visible")
    {
        bool t;
        if (!from_json(v, t))
        {
            return false;
        }
        o->visible = t;
        return true;
    }
    if (p == "enabled")
    {
        bool t;
        if (!from_json(v, t))
        {
            return false;
        }
        o->enabled = t;
        return true;
    }
    if (p == "focus_policy")
    {
        tokotachi::FocusPolicy t;
        if (!from_json(v, t))
        {
            return false;
        }
        o->focus_policy = t;
        return true;
    }
    if (p == "cursor")
    {
        tokotachi::MouseCursor t;
        if (!from_json(v, t))
        {
            return false;
        }
        o->cursor = t;
        return true;
    }
    if (p == "tooltip")
    {
        std::string t;
        if (!from_json(v, t))
        {
            return false;
        }
        o->tooltip = t;
        return true;
    }
    if (p == "theme_override")
    {
        tokotachi::ThemeOverride t;
        if (!from_json(v, t))
        {
            return false;
        }
        o->theme_override = t;
        return true;
    }
    if (p == "accepts_drops")
    {
        bool t;
        if (!from_json(v, t))
        {
            return false;
        }
        o->accepts_drops = t;
        return true;
    }
    if (p == "text")
    {
        std::string t;
        if (!from_json(v, t))
        {
            return false;
        }
        o->text = t;
        return true;
    }
    if (p == "placeholder_text")
    {
        std::string t;
        if (!from_json(v, t))
        {
            return false;
        }
        o->placeholder_text = t;
        return true;
    }
    if (p == "font")
    {
        tokotachi::Font t;
        if (!from_json(v, t))
        {
            return false;
        }
        o->font = t;
        return true;
    }
    return false;
}

bool get_LineEdit(tokotachi::LineEdit* o, const std::string& p, Json& out)
{
    if (p == "x")
    {
        out = to_json(o->x.get());
        return true;
    }
    if (p == "y")
    {
        out = to_json(o->y.get());
        return true;
    }
    if (p == "width")
    {
        out = to_json(o->width.get());
        return true;
    }
    if (p == "height")
    {
        out = to_json(o->height.get());
        return true;
    }
    if (p == "minimum_size")
    {
        out = to_json(o->minimum_size.get());
        return true;
    }
    if (p == "maximum_size")
    {
        out = to_json(o->maximum_size.get());
        return true;
    }
    if (p == "visible")
    {
        out = to_json(o->visible.get());
        return true;
    }
    if (p == "enabled")
    {
        out = to_json(o->enabled.get());
        return true;
    }
    if (p == "focus_policy")
    {
        out = to_json(o->focus_policy.get());
        return true;
    }
    if (p == "cursor")
    {
        out = to_json(o->cursor.get());
        return true;
    }
    if (p == "tooltip")
    {
        out = to_json(o->tooltip.get());
        return true;
    }
    if (p == "theme_override")
    {
        out = to_json(o->theme_override.get());
        return true;
    }
    if (p == "accepts_drops")
    {
        out = to_json(o->accepts_drops.get());
        return true;
    }
    if (p == "text")
    {
        out = to_json(o->text.get());
        return true;
    }
    if (p == "placeholder_text")
    {
        out = to_json(o->placeholder_text.get());
        return true;
    }
    if (p == "font")
    {
        out = to_json(o->font.get());
        return true;
    }
    return false;
}

bool set_TextEdit(tokotachi::TextEdit* o, const std::string& p, const Json& v)
{
    if (p == "x")
    {
        float t;
        if (!from_json(v, t))
        {
            return false;
        }
        o->x = t;
        return true;
    }
    if (p == "y")
    {
        float t;
        if (!from_json(v, t))
        {
            return false;
        }
        o->y = t;
        return true;
    }
    if (p == "width")
    {
        float t;
        if (!from_json(v, t))
        {
            return false;
        }
        o->width = t;
        return true;
    }
    if (p == "height")
    {
        float t;
        if (!from_json(v, t))
        {
            return false;
        }
        o->height = t;
        return true;
    }
    if (p == "minimum_size")
    {
        tokotachi::Size t;
        if (!from_json(v, t))
        {
            return false;
        }
        o->minimum_size = t;
        return true;
    }
    if (p == "maximum_size")
    {
        tokotachi::Size t;
        if (!from_json(v, t))
        {
            return false;
        }
        o->maximum_size = t;
        return true;
    }
    if (p == "visible")
    {
        bool t;
        if (!from_json(v, t))
        {
            return false;
        }
        o->visible = t;
        return true;
    }
    if (p == "enabled")
    {
        bool t;
        if (!from_json(v, t))
        {
            return false;
        }
        o->enabled = t;
        return true;
    }
    if (p == "focus_policy")
    {
        tokotachi::FocusPolicy t;
        if (!from_json(v, t))
        {
            return false;
        }
        o->focus_policy = t;
        return true;
    }
    if (p == "cursor")
    {
        tokotachi::MouseCursor t;
        if (!from_json(v, t))
        {
            return false;
        }
        o->cursor = t;
        return true;
    }
    if (p == "tooltip")
    {
        std::string t;
        if (!from_json(v, t))
        {
            return false;
        }
        o->tooltip = t;
        return true;
    }
    if (p == "theme_override")
    {
        tokotachi::ThemeOverride t;
        if (!from_json(v, t))
        {
            return false;
        }
        o->theme_override = t;
        return true;
    }
    if (p == "accepts_drops")
    {
        bool t;
        if (!from_json(v, t))
        {
            return false;
        }
        o->accepts_drops = t;
        return true;
    }
    if (p == "text")
    {
        std::string t;
        if (!from_json(v, t))
        {
            return false;
        }
        o->text = t;
        return true;
    }
    if (p == "placeholder_text")
    {
        std::string t;
        if (!from_json(v, t))
        {
            return false;
        }
        o->placeholder_text = t;
        return true;
    }
    if (p == "font")
    {
        tokotachi::Font t;
        if (!from_json(v, t))
        {
            return false;
        }
        o->font = t;
        return true;
    }
    if (p == "word_wrap")
    {
        bool t;
        if (!from_json(v, t))
        {
            return false;
        }
        o->word_wrap = t;
        return true;
    }
    return false;
}

bool get_TextEdit(tokotachi::TextEdit* o, const std::string& p, Json& out)
{
    if (p == "x")
    {
        out = to_json(o->x.get());
        return true;
    }
    if (p == "y")
    {
        out = to_json(o->y.get());
        return true;
    }
    if (p == "width")
    {
        out = to_json(o->width.get());
        return true;
    }
    if (p == "height")
    {
        out = to_json(o->height.get());
        return true;
    }
    if (p == "minimum_size")
    {
        out = to_json(o->minimum_size.get());
        return true;
    }
    if (p == "maximum_size")
    {
        out = to_json(o->maximum_size.get());
        return true;
    }
    if (p == "visible")
    {
        out = to_json(o->visible.get());
        return true;
    }
    if (p == "enabled")
    {
        out = to_json(o->enabled.get());
        return true;
    }
    if (p == "focus_policy")
    {
        out = to_json(o->focus_policy.get());
        return true;
    }
    if (p == "cursor")
    {
        out = to_json(o->cursor.get());
        return true;
    }
    if (p == "tooltip")
    {
        out = to_json(o->tooltip.get());
        return true;
    }
    if (p == "theme_override")
    {
        out = to_json(o->theme_override.get());
        return true;
    }
    if (p == "accepts_drops")
    {
        out = to_json(o->accepts_drops.get());
        return true;
    }
    if (p == "text")
    {
        out = to_json(o->text.get());
        return true;
    }
    if (p == "placeholder_text")
    {
        out = to_json(o->placeholder_text.get());
        return true;
    }
    if (p == "font")
    {
        out = to_json(o->font.get());
        return true;
    }
    if (p == "word_wrap")
    {
        out = to_json(o->word_wrap.get());
        return true;
    }
    return false;
}

bool set_SpinBox(tokotachi::SpinBox* o, const std::string& p, const Json& v)
{
    if (p == "x")
    {
        float t;
        if (!from_json(v, t))
        {
            return false;
        }
        o->x = t;
        return true;
    }
    if (p == "y")
    {
        float t;
        if (!from_json(v, t))
        {
            return false;
        }
        o->y = t;
        return true;
    }
    if (p == "width")
    {
        float t;
        if (!from_json(v, t))
        {
            return false;
        }
        o->width = t;
        return true;
    }
    if (p == "height")
    {
        float t;
        if (!from_json(v, t))
        {
            return false;
        }
        o->height = t;
        return true;
    }
    if (p == "minimum_size")
    {
        tokotachi::Size t;
        if (!from_json(v, t))
        {
            return false;
        }
        o->minimum_size = t;
        return true;
    }
    if (p == "maximum_size")
    {
        tokotachi::Size t;
        if (!from_json(v, t))
        {
            return false;
        }
        o->maximum_size = t;
        return true;
    }
    if (p == "visible")
    {
        bool t;
        if (!from_json(v, t))
        {
            return false;
        }
        o->visible = t;
        return true;
    }
    if (p == "enabled")
    {
        bool t;
        if (!from_json(v, t))
        {
            return false;
        }
        o->enabled = t;
        return true;
    }
    if (p == "focus_policy")
    {
        tokotachi::FocusPolicy t;
        if (!from_json(v, t))
        {
            return false;
        }
        o->focus_policy = t;
        return true;
    }
    if (p == "cursor")
    {
        tokotachi::MouseCursor t;
        if (!from_json(v, t))
        {
            return false;
        }
        o->cursor = t;
        return true;
    }
    if (p == "tooltip")
    {
        std::string t;
        if (!from_json(v, t))
        {
            return false;
        }
        o->tooltip = t;
        return true;
    }
    if (p == "theme_override")
    {
        tokotachi::ThemeOverride t;
        if (!from_json(v, t))
        {
            return false;
        }
        o->theme_override = t;
        return true;
    }
    if (p == "accepts_drops")
    {
        bool t;
        if (!from_json(v, t))
        {
            return false;
        }
        o->accepts_drops = t;
        return true;
    }
    if (p == "minimum")
    {
        double t;
        if (!from_json(v, t))
        {
            return false;
        }
        o->minimum = t;
        return true;
    }
    if (p == "maximum")
    {
        double t;
        if (!from_json(v, t))
        {
            return false;
        }
        o->maximum = t;
        return true;
    }
    if (p == "value")
    {
        double t;
        if (!from_json(v, t))
        {
            return false;
        }
        o->value = t;
        return true;
    }
    if (p == "single_step")
    {
        double t;
        if (!from_json(v, t))
        {
            return false;
        }
        o->single_step = t;
        return true;
    }
    if (p == "page_step")
    {
        double t;
        if (!from_json(v, t))
        {
            return false;
        }
        o->page_step = t;
        return true;
    }
    if (p == "decimals")
    {
        int t;
        if (!from_json(v, t))
        {
            return false;
        }
        o->decimals = t;
        return true;
    }
    return false;
}

bool get_SpinBox(tokotachi::SpinBox* o, const std::string& p, Json& out)
{
    if (p == "x")
    {
        out = to_json(o->x.get());
        return true;
    }
    if (p == "y")
    {
        out = to_json(o->y.get());
        return true;
    }
    if (p == "width")
    {
        out = to_json(o->width.get());
        return true;
    }
    if (p == "height")
    {
        out = to_json(o->height.get());
        return true;
    }
    if (p == "minimum_size")
    {
        out = to_json(o->minimum_size.get());
        return true;
    }
    if (p == "maximum_size")
    {
        out = to_json(o->maximum_size.get());
        return true;
    }
    if (p == "visible")
    {
        out = to_json(o->visible.get());
        return true;
    }
    if (p == "enabled")
    {
        out = to_json(o->enabled.get());
        return true;
    }
    if (p == "focus_policy")
    {
        out = to_json(o->focus_policy.get());
        return true;
    }
    if (p == "cursor")
    {
        out = to_json(o->cursor.get());
        return true;
    }
    if (p == "tooltip")
    {
        out = to_json(o->tooltip.get());
        return true;
    }
    if (p == "theme_override")
    {
        out = to_json(o->theme_override.get());
        return true;
    }
    if (p == "accepts_drops")
    {
        out = to_json(o->accepts_drops.get());
        return true;
    }
    if (p == "minimum")
    {
        out = to_json(o->minimum.get());
        return true;
    }
    if (p == "maximum")
    {
        out = to_json(o->maximum.get());
        return true;
    }
    if (p == "value")
    {
        out = to_json(o->value.get());
        return true;
    }
    if (p == "single_step")
    {
        out = to_json(o->single_step.get());
        return true;
    }
    if (p == "page_step")
    {
        out = to_json(o->page_step.get());
        return true;
    }
    if (p == "decimals")
    {
        out = to_json(o->decimals.get());
        return true;
    }
    return false;
}

bool set_Slider(tokotachi::Slider* o, const std::string& p, const Json& v)
{
    if (p == "x")
    {
        float t;
        if (!from_json(v, t))
        {
            return false;
        }
        o->x = t;
        return true;
    }
    if (p == "y")
    {
        float t;
        if (!from_json(v, t))
        {
            return false;
        }
        o->y = t;
        return true;
    }
    if (p == "width")
    {
        float t;
        if (!from_json(v, t))
        {
            return false;
        }
        o->width = t;
        return true;
    }
    if (p == "height")
    {
        float t;
        if (!from_json(v, t))
        {
            return false;
        }
        o->height = t;
        return true;
    }
    if (p == "minimum_size")
    {
        tokotachi::Size t;
        if (!from_json(v, t))
        {
            return false;
        }
        o->minimum_size = t;
        return true;
    }
    if (p == "maximum_size")
    {
        tokotachi::Size t;
        if (!from_json(v, t))
        {
            return false;
        }
        o->maximum_size = t;
        return true;
    }
    if (p == "visible")
    {
        bool t;
        if (!from_json(v, t))
        {
            return false;
        }
        o->visible = t;
        return true;
    }
    if (p == "enabled")
    {
        bool t;
        if (!from_json(v, t))
        {
            return false;
        }
        o->enabled = t;
        return true;
    }
    if (p == "focus_policy")
    {
        tokotachi::FocusPolicy t;
        if (!from_json(v, t))
        {
            return false;
        }
        o->focus_policy = t;
        return true;
    }
    if (p == "cursor")
    {
        tokotachi::MouseCursor t;
        if (!from_json(v, t))
        {
            return false;
        }
        o->cursor = t;
        return true;
    }
    if (p == "tooltip")
    {
        std::string t;
        if (!from_json(v, t))
        {
            return false;
        }
        o->tooltip = t;
        return true;
    }
    if (p == "theme_override")
    {
        tokotachi::ThemeOverride t;
        if (!from_json(v, t))
        {
            return false;
        }
        o->theme_override = t;
        return true;
    }
    if (p == "accepts_drops")
    {
        bool t;
        if (!from_json(v, t))
        {
            return false;
        }
        o->accepts_drops = t;
        return true;
    }
    if (p == "minimum")
    {
        double t;
        if (!from_json(v, t))
        {
            return false;
        }
        o->minimum = t;
        return true;
    }
    if (p == "maximum")
    {
        double t;
        if (!from_json(v, t))
        {
            return false;
        }
        o->maximum = t;
        return true;
    }
    if (p == "value")
    {
        double t;
        if (!from_json(v, t))
        {
            return false;
        }
        o->value = t;
        return true;
    }
    if (p == "single_step")
    {
        double t;
        if (!from_json(v, t))
        {
            return false;
        }
        o->single_step = t;
        return true;
    }
    if (p == "page_step")
    {
        double t;
        if (!from_json(v, t))
        {
            return false;
        }
        o->page_step = t;
        return true;
    }
    if (p == "orientation")
    {
        tokotachi::Orientation t;
        if (!from_json(v, t))
        {
            return false;
        }
        o->orientation = t;
        return true;
    }
    return false;
}

bool get_Slider(tokotachi::Slider* o, const std::string& p, Json& out)
{
    if (p == "x")
    {
        out = to_json(o->x.get());
        return true;
    }
    if (p == "y")
    {
        out = to_json(o->y.get());
        return true;
    }
    if (p == "width")
    {
        out = to_json(o->width.get());
        return true;
    }
    if (p == "height")
    {
        out = to_json(o->height.get());
        return true;
    }
    if (p == "minimum_size")
    {
        out = to_json(o->minimum_size.get());
        return true;
    }
    if (p == "maximum_size")
    {
        out = to_json(o->maximum_size.get());
        return true;
    }
    if (p == "visible")
    {
        out = to_json(o->visible.get());
        return true;
    }
    if (p == "enabled")
    {
        out = to_json(o->enabled.get());
        return true;
    }
    if (p == "focus_policy")
    {
        out = to_json(o->focus_policy.get());
        return true;
    }
    if (p == "cursor")
    {
        out = to_json(o->cursor.get());
        return true;
    }
    if (p == "tooltip")
    {
        out = to_json(o->tooltip.get());
        return true;
    }
    if (p == "theme_override")
    {
        out = to_json(o->theme_override.get());
        return true;
    }
    if (p == "accepts_drops")
    {
        out = to_json(o->accepts_drops.get());
        return true;
    }
    if (p == "minimum")
    {
        out = to_json(o->minimum.get());
        return true;
    }
    if (p == "maximum")
    {
        out = to_json(o->maximum.get());
        return true;
    }
    if (p == "value")
    {
        out = to_json(o->value.get());
        return true;
    }
    if (p == "single_step")
    {
        out = to_json(o->single_step.get());
        return true;
    }
    if (p == "page_step")
    {
        out = to_json(o->page_step.get());
        return true;
    }
    if (p == "orientation")
    {
        out = to_json(o->orientation.get());
        return true;
    }
    return false;
}

bool set_ComboBox(tokotachi::ComboBox* o, const std::string& p, const Json& v)
{
    if (p == "x")
    {
        float t;
        if (!from_json(v, t))
        {
            return false;
        }
        o->x = t;
        return true;
    }
    if (p == "y")
    {
        float t;
        if (!from_json(v, t))
        {
            return false;
        }
        o->y = t;
        return true;
    }
    if (p == "width")
    {
        float t;
        if (!from_json(v, t))
        {
            return false;
        }
        o->width = t;
        return true;
    }
    if (p == "height")
    {
        float t;
        if (!from_json(v, t))
        {
            return false;
        }
        o->height = t;
        return true;
    }
    if (p == "minimum_size")
    {
        tokotachi::Size t;
        if (!from_json(v, t))
        {
            return false;
        }
        o->minimum_size = t;
        return true;
    }
    if (p == "maximum_size")
    {
        tokotachi::Size t;
        if (!from_json(v, t))
        {
            return false;
        }
        o->maximum_size = t;
        return true;
    }
    if (p == "visible")
    {
        bool t;
        if (!from_json(v, t))
        {
            return false;
        }
        o->visible = t;
        return true;
    }
    if (p == "enabled")
    {
        bool t;
        if (!from_json(v, t))
        {
            return false;
        }
        o->enabled = t;
        return true;
    }
    if (p == "focus_policy")
    {
        tokotachi::FocusPolicy t;
        if (!from_json(v, t))
        {
            return false;
        }
        o->focus_policy = t;
        return true;
    }
    if (p == "cursor")
    {
        tokotachi::MouseCursor t;
        if (!from_json(v, t))
        {
            return false;
        }
        o->cursor = t;
        return true;
    }
    if (p == "tooltip")
    {
        std::string t;
        if (!from_json(v, t))
        {
            return false;
        }
        o->tooltip = t;
        return true;
    }
    if (p == "theme_override")
    {
        tokotachi::ThemeOverride t;
        if (!from_json(v, t))
        {
            return false;
        }
        o->theme_override = t;
        return true;
    }
    if (p == "accepts_drops")
    {
        bool t;
        if (!from_json(v, t))
        {
            return false;
        }
        o->accepts_drops = t;
        return true;
    }
    if (p == "items")
    {
        std::vector<std::string> t;
        if (!from_json(v, t))
        {
            return false;
        }
        o->items = t;
        return true;
    }
    if (p == "current_index")
    {
        int t;
        if (!from_json(v, t))
        {
            return false;
        }
        o->current_index = t;
        return true;
    }
    if (p == "placeholder_text")
    {
        std::string t;
        if (!from_json(v, t))
        {
            return false;
        }
        o->placeholder_text = t;
        return true;
    }
    if (p == "font")
    {
        tokotachi::Font t;
        if (!from_json(v, t))
        {
            return false;
        }
        o->font = t;
        return true;
    }
    return false;
}

bool get_ComboBox(tokotachi::ComboBox* o, const std::string& p, Json& out)
{
    if (p == "x")
    {
        out = to_json(o->x.get());
        return true;
    }
    if (p == "y")
    {
        out = to_json(o->y.get());
        return true;
    }
    if (p == "width")
    {
        out = to_json(o->width.get());
        return true;
    }
    if (p == "height")
    {
        out = to_json(o->height.get());
        return true;
    }
    if (p == "minimum_size")
    {
        out = to_json(o->minimum_size.get());
        return true;
    }
    if (p == "maximum_size")
    {
        out = to_json(o->maximum_size.get());
        return true;
    }
    if (p == "visible")
    {
        out = to_json(o->visible.get());
        return true;
    }
    if (p == "enabled")
    {
        out = to_json(o->enabled.get());
        return true;
    }
    if (p == "focus_policy")
    {
        out = to_json(o->focus_policy.get());
        return true;
    }
    if (p == "cursor")
    {
        out = to_json(o->cursor.get());
        return true;
    }
    if (p == "tooltip")
    {
        out = to_json(o->tooltip.get());
        return true;
    }
    if (p == "theme_override")
    {
        out = to_json(o->theme_override.get());
        return true;
    }
    if (p == "accepts_drops")
    {
        out = to_json(o->accepts_drops.get());
        return true;
    }
    if (p == "items")
    {
        out = to_json(o->items.get());
        return true;
    }
    if (p == "current_index")
    {
        out = to_json(o->current_index.get());
        return true;
    }
    if (p == "placeholder_text")
    {
        out = to_json(o->placeholder_text.get());
        return true;
    }
    if (p == "font")
    {
        out = to_json(o->font.get());
        return true;
    }
    return false;
}

bool set_Label(tokotachi::Label* o, const std::string& p, const Json& v)
{
    if (p == "x")
    {
        float t;
        if (!from_json(v, t))
        {
            return false;
        }
        o->x = t;
        return true;
    }
    if (p == "y")
    {
        float t;
        if (!from_json(v, t))
        {
            return false;
        }
        o->y = t;
        return true;
    }
    if (p == "width")
    {
        float t;
        if (!from_json(v, t))
        {
            return false;
        }
        o->width = t;
        return true;
    }
    if (p == "height")
    {
        float t;
        if (!from_json(v, t))
        {
            return false;
        }
        o->height = t;
        return true;
    }
    if (p == "minimum_size")
    {
        tokotachi::Size t;
        if (!from_json(v, t))
        {
            return false;
        }
        o->minimum_size = t;
        return true;
    }
    if (p == "maximum_size")
    {
        tokotachi::Size t;
        if (!from_json(v, t))
        {
            return false;
        }
        o->maximum_size = t;
        return true;
    }
    if (p == "visible")
    {
        bool t;
        if (!from_json(v, t))
        {
            return false;
        }
        o->visible = t;
        return true;
    }
    if (p == "enabled")
    {
        bool t;
        if (!from_json(v, t))
        {
            return false;
        }
        o->enabled = t;
        return true;
    }
    if (p == "focus_policy")
    {
        tokotachi::FocusPolicy t;
        if (!from_json(v, t))
        {
            return false;
        }
        o->focus_policy = t;
        return true;
    }
    if (p == "cursor")
    {
        tokotachi::MouseCursor t;
        if (!from_json(v, t))
        {
            return false;
        }
        o->cursor = t;
        return true;
    }
    if (p == "tooltip")
    {
        std::string t;
        if (!from_json(v, t))
        {
            return false;
        }
        o->tooltip = t;
        return true;
    }
    if (p == "theme_override")
    {
        tokotachi::ThemeOverride t;
        if (!from_json(v, t))
        {
            return false;
        }
        o->theme_override = t;
        return true;
    }
    if (p == "accepts_drops")
    {
        bool t;
        if (!from_json(v, t))
        {
            return false;
        }
        o->accepts_drops = t;
        return true;
    }
    if (p == "text")
    {
        std::string t;
        if (!from_json(v, t))
        {
            return false;
        }
        o->text = t;
        return true;
    }
    if (p == "font")
    {
        tokotachi::Font t;
        if (!from_json(v, t))
        {
            return false;
        }
        o->font = t;
        return true;
    }
    if (p == "color")
    {
        tokotachi::ThemeColor t;
        if (!from_json(v, t))
        {
            return false;
        }
        o->color = t;
        return true;
    }
    if (p == "alignment")
    {
        tokotachi::Align t;
        if (!from_json(v, t))
        {
            return false;
        }
        o->alignment = t;
        return true;
    }
    if (p == "word_wrap")
    {
        bool t;
        if (!from_json(v, t))
        {
            return false;
        }
        o->word_wrap = t;
        return true;
    }
    if (p == "icon")
    {
        tokotachi::Image t;
        if (!from_json(v, t))
        {
            return false;
        }
        o->icon = t;
        return true;
    }
    if (p == "icon_size")
    {
        float t;
        if (!from_json(v, t))
        {
            return false;
        }
        o->icon_size = t;
        return true;
    }
    return false;
}

bool get_Label(tokotachi::Label* o, const std::string& p, Json& out)
{
    if (p == "x")
    {
        out = to_json(o->x.get());
        return true;
    }
    if (p == "y")
    {
        out = to_json(o->y.get());
        return true;
    }
    if (p == "width")
    {
        out = to_json(o->width.get());
        return true;
    }
    if (p == "height")
    {
        out = to_json(o->height.get());
        return true;
    }
    if (p == "minimum_size")
    {
        out = to_json(o->minimum_size.get());
        return true;
    }
    if (p == "maximum_size")
    {
        out = to_json(o->maximum_size.get());
        return true;
    }
    if (p == "visible")
    {
        out = to_json(o->visible.get());
        return true;
    }
    if (p == "enabled")
    {
        out = to_json(o->enabled.get());
        return true;
    }
    if (p == "focus_policy")
    {
        out = to_json(o->focus_policy.get());
        return true;
    }
    if (p == "cursor")
    {
        out = to_json(o->cursor.get());
        return true;
    }
    if (p == "tooltip")
    {
        out = to_json(o->tooltip.get());
        return true;
    }
    if (p == "theme_override")
    {
        out = to_json(o->theme_override.get());
        return true;
    }
    if (p == "accepts_drops")
    {
        out = to_json(o->accepts_drops.get());
        return true;
    }
    if (p == "text")
    {
        out = to_json(o->text.get());
        return true;
    }
    if (p == "font")
    {
        out = to_json(o->font.get());
        return true;
    }
    if (p == "color")
    {
        out = to_json(o->color.get());
        return true;
    }
    if (p == "alignment")
    {
        out = to_json(o->alignment.get());
        return true;
    }
    if (p == "word_wrap")
    {
        out = to_json(o->word_wrap.get());
        return true;
    }
    if (p == "icon")
    {
        out = to_json(o->icon.get());
        return true;
    }
    if (p == "icon_size")
    {
        out = to_json(o->icon_size.get());
        return true;
    }
    return false;
}

bool set_ProgressBar(tokotachi::ProgressBar* o, const std::string& p, const Json& v)
{
    if (p == "x")
    {
        float t;
        if (!from_json(v, t))
        {
            return false;
        }
        o->x = t;
        return true;
    }
    if (p == "y")
    {
        float t;
        if (!from_json(v, t))
        {
            return false;
        }
        o->y = t;
        return true;
    }
    if (p == "width")
    {
        float t;
        if (!from_json(v, t))
        {
            return false;
        }
        o->width = t;
        return true;
    }
    if (p == "height")
    {
        float t;
        if (!from_json(v, t))
        {
            return false;
        }
        o->height = t;
        return true;
    }
    if (p == "minimum_size")
    {
        tokotachi::Size t;
        if (!from_json(v, t))
        {
            return false;
        }
        o->minimum_size = t;
        return true;
    }
    if (p == "maximum_size")
    {
        tokotachi::Size t;
        if (!from_json(v, t))
        {
            return false;
        }
        o->maximum_size = t;
        return true;
    }
    if (p == "visible")
    {
        bool t;
        if (!from_json(v, t))
        {
            return false;
        }
        o->visible = t;
        return true;
    }
    if (p == "enabled")
    {
        bool t;
        if (!from_json(v, t))
        {
            return false;
        }
        o->enabled = t;
        return true;
    }
    if (p == "focus_policy")
    {
        tokotachi::FocusPolicy t;
        if (!from_json(v, t))
        {
            return false;
        }
        o->focus_policy = t;
        return true;
    }
    if (p == "cursor")
    {
        tokotachi::MouseCursor t;
        if (!from_json(v, t))
        {
            return false;
        }
        o->cursor = t;
        return true;
    }
    if (p == "tooltip")
    {
        std::string t;
        if (!from_json(v, t))
        {
            return false;
        }
        o->tooltip = t;
        return true;
    }
    if (p == "theme_override")
    {
        tokotachi::ThemeOverride t;
        if (!from_json(v, t))
        {
            return false;
        }
        o->theme_override = t;
        return true;
    }
    if (p == "accepts_drops")
    {
        bool t;
        if (!from_json(v, t))
        {
            return false;
        }
        o->accepts_drops = t;
        return true;
    }
    if (p == "minimum")
    {
        double t;
        if (!from_json(v, t))
        {
            return false;
        }
        o->minimum = t;
        return true;
    }
    if (p == "maximum")
    {
        double t;
        if (!from_json(v, t))
        {
            return false;
        }
        o->maximum = t;
        return true;
    }
    if (p == "value")
    {
        double t;
        if (!from_json(v, t))
        {
            return false;
        }
        o->value = t;
        return true;
    }
    if (p == "text_visible")
    {
        bool t;
        if (!from_json(v, t))
        {
            return false;
        }
        o->text_visible = t;
        return true;
    }
    return false;
}

bool get_ProgressBar(tokotachi::ProgressBar* o, const std::string& p, Json& out)
{
    if (p == "x")
    {
        out = to_json(o->x.get());
        return true;
    }
    if (p == "y")
    {
        out = to_json(o->y.get());
        return true;
    }
    if (p == "width")
    {
        out = to_json(o->width.get());
        return true;
    }
    if (p == "height")
    {
        out = to_json(o->height.get());
        return true;
    }
    if (p == "minimum_size")
    {
        out = to_json(o->minimum_size.get());
        return true;
    }
    if (p == "maximum_size")
    {
        out = to_json(o->maximum_size.get());
        return true;
    }
    if (p == "visible")
    {
        out = to_json(o->visible.get());
        return true;
    }
    if (p == "enabled")
    {
        out = to_json(o->enabled.get());
        return true;
    }
    if (p == "focus_policy")
    {
        out = to_json(o->focus_policy.get());
        return true;
    }
    if (p == "cursor")
    {
        out = to_json(o->cursor.get());
        return true;
    }
    if (p == "tooltip")
    {
        out = to_json(o->tooltip.get());
        return true;
    }
    if (p == "theme_override")
    {
        out = to_json(o->theme_override.get());
        return true;
    }
    if (p == "accepts_drops")
    {
        out = to_json(o->accepts_drops.get());
        return true;
    }
    if (p == "minimum")
    {
        out = to_json(o->minimum.get());
        return true;
    }
    if (p == "maximum")
    {
        out = to_json(o->maximum.get());
        return true;
    }
    if (p == "value")
    {
        out = to_json(o->value.get());
        return true;
    }
    if (p == "text_visible")
    {
        out = to_json(o->text_visible.get());
        return true;
    }
    return false;
}

bool set_ImageBox(tokotachi::ImageBox* o, const std::string& p, const Json& v)
{
    if (p == "x")
    {
        float t;
        if (!from_json(v, t))
        {
            return false;
        }
        o->x = t;
        return true;
    }
    if (p == "y")
    {
        float t;
        if (!from_json(v, t))
        {
            return false;
        }
        o->y = t;
        return true;
    }
    if (p == "width")
    {
        float t;
        if (!from_json(v, t))
        {
            return false;
        }
        o->width = t;
        return true;
    }
    if (p == "height")
    {
        float t;
        if (!from_json(v, t))
        {
            return false;
        }
        o->height = t;
        return true;
    }
    if (p == "minimum_size")
    {
        tokotachi::Size t;
        if (!from_json(v, t))
        {
            return false;
        }
        o->minimum_size = t;
        return true;
    }
    if (p == "maximum_size")
    {
        tokotachi::Size t;
        if (!from_json(v, t))
        {
            return false;
        }
        o->maximum_size = t;
        return true;
    }
    if (p == "visible")
    {
        bool t;
        if (!from_json(v, t))
        {
            return false;
        }
        o->visible = t;
        return true;
    }
    if (p == "enabled")
    {
        bool t;
        if (!from_json(v, t))
        {
            return false;
        }
        o->enabled = t;
        return true;
    }
    if (p == "focus_policy")
    {
        tokotachi::FocusPolicy t;
        if (!from_json(v, t))
        {
            return false;
        }
        o->focus_policy = t;
        return true;
    }
    if (p == "cursor")
    {
        tokotachi::MouseCursor t;
        if (!from_json(v, t))
        {
            return false;
        }
        o->cursor = t;
        return true;
    }
    if (p == "tooltip")
    {
        std::string t;
        if (!from_json(v, t))
        {
            return false;
        }
        o->tooltip = t;
        return true;
    }
    if (p == "theme_override")
    {
        tokotachi::ThemeOverride t;
        if (!from_json(v, t))
        {
            return false;
        }
        o->theme_override = t;
        return true;
    }
    if (p == "accepts_drops")
    {
        bool t;
        if (!from_json(v, t))
        {
            return false;
        }
        o->accepts_drops = t;
        return true;
    }
    if (p == "image")
    {
        tokotachi::Image t;
        if (!from_json(v, t))
        {
            return false;
        }
        o->image = t;
        return true;
    }
    if (p == "fit")
    {
        tokotachi::ImageFit t;
        if (!from_json(v, t))
        {
            return false;
        }
        o->fit = t;
        return true;
    }
    return false;
}

bool get_ImageBox(tokotachi::ImageBox* o, const std::string& p, Json& out)
{
    if (p == "x")
    {
        out = to_json(o->x.get());
        return true;
    }
    if (p == "y")
    {
        out = to_json(o->y.get());
        return true;
    }
    if (p == "width")
    {
        out = to_json(o->width.get());
        return true;
    }
    if (p == "height")
    {
        out = to_json(o->height.get());
        return true;
    }
    if (p == "minimum_size")
    {
        out = to_json(o->minimum_size.get());
        return true;
    }
    if (p == "maximum_size")
    {
        out = to_json(o->maximum_size.get());
        return true;
    }
    if (p == "visible")
    {
        out = to_json(o->visible.get());
        return true;
    }
    if (p == "enabled")
    {
        out = to_json(o->enabled.get());
        return true;
    }
    if (p == "focus_policy")
    {
        out = to_json(o->focus_policy.get());
        return true;
    }
    if (p == "cursor")
    {
        out = to_json(o->cursor.get());
        return true;
    }
    if (p == "tooltip")
    {
        out = to_json(o->tooltip.get());
        return true;
    }
    if (p == "theme_override")
    {
        out = to_json(o->theme_override.get());
        return true;
    }
    if (p == "accepts_drops")
    {
        out = to_json(o->accepts_drops.get());
        return true;
    }
    if (p == "image")
    {
        out = to_json(o->image.get());
        return true;
    }
    if (p == "fit")
    {
        out = to_json(o->fit.get());
        return true;
    }
    return false;
}

bool set_ListView(tokotachi::ListView* o, const std::string& p, const Json& v)
{
    if (p == "x")
    {
        float t;
        if (!from_json(v, t))
        {
            return false;
        }
        o->x = t;
        return true;
    }
    if (p == "y")
    {
        float t;
        if (!from_json(v, t))
        {
            return false;
        }
        o->y = t;
        return true;
    }
    if (p == "width")
    {
        float t;
        if (!from_json(v, t))
        {
            return false;
        }
        o->width = t;
        return true;
    }
    if (p == "height")
    {
        float t;
        if (!from_json(v, t))
        {
            return false;
        }
        o->height = t;
        return true;
    }
    if (p == "minimum_size")
    {
        tokotachi::Size t;
        if (!from_json(v, t))
        {
            return false;
        }
        o->minimum_size = t;
        return true;
    }
    if (p == "maximum_size")
    {
        tokotachi::Size t;
        if (!from_json(v, t))
        {
            return false;
        }
        o->maximum_size = t;
        return true;
    }
    if (p == "visible")
    {
        bool t;
        if (!from_json(v, t))
        {
            return false;
        }
        o->visible = t;
        return true;
    }
    if (p == "enabled")
    {
        bool t;
        if (!from_json(v, t))
        {
            return false;
        }
        o->enabled = t;
        return true;
    }
    if (p == "focus_policy")
    {
        tokotachi::FocusPolicy t;
        if (!from_json(v, t))
        {
            return false;
        }
        o->focus_policy = t;
        return true;
    }
    if (p == "cursor")
    {
        tokotachi::MouseCursor t;
        if (!from_json(v, t))
        {
            return false;
        }
        o->cursor = t;
        return true;
    }
    if (p == "tooltip")
    {
        std::string t;
        if (!from_json(v, t))
        {
            return false;
        }
        o->tooltip = t;
        return true;
    }
    if (p == "theme_override")
    {
        tokotachi::ThemeOverride t;
        if (!from_json(v, t))
        {
            return false;
        }
        o->theme_override = t;
        return true;
    }
    if (p == "accepts_drops")
    {
        bool t;
        if (!from_json(v, t))
        {
            return false;
        }
        o->accepts_drops = t;
        return true;
    }
    if (p == "header_visible")
    {
        bool t;
        if (!from_json(v, t))
        {
            return false;
        }
        o->header_visible = t;
        return true;
    }
    if (p == "sorting_enabled")
    {
        bool t;
        if (!from_json(v, t))
        {
            return false;
        }
        o->sorting_enabled = t;
        return true;
    }
    if (p == "sort_column")
    {
        int t;
        if (!from_json(v, t))
        {
            return false;
        }
        o->sort_column = t;
        return true;
    }
    if (p == "sort_order")
    {
        tokotachi::SortOrder t;
        if (!from_json(v, t))
        {
            return false;
        }
        o->sort_order = t;
        return true;
    }
    if (p == "selection_mode")
    {
        tokotachi::SelectionMode t;
        if (!from_json(v, t))
        {
            return false;
        }
        o->selection_mode = t;
        return true;
    }
    if (p == "drag_enabled")
    {
        bool t;
        if (!from_json(v, t))
        {
            return false;
        }
        o->drag_enabled = t;
        return true;
    }
    if (p == "drop_enabled")
    {
        bool t;
        if (!from_json(v, t))
        {
            return false;
        }
        o->drop_enabled = t;
        return true;
    }
    return false;
}

bool get_ListView(tokotachi::ListView* o, const std::string& p, Json& out)
{
    if (p == "x")
    {
        out = to_json(o->x.get());
        return true;
    }
    if (p == "y")
    {
        out = to_json(o->y.get());
        return true;
    }
    if (p == "width")
    {
        out = to_json(o->width.get());
        return true;
    }
    if (p == "height")
    {
        out = to_json(o->height.get());
        return true;
    }
    if (p == "minimum_size")
    {
        out = to_json(o->minimum_size.get());
        return true;
    }
    if (p == "maximum_size")
    {
        out = to_json(o->maximum_size.get());
        return true;
    }
    if (p == "visible")
    {
        out = to_json(o->visible.get());
        return true;
    }
    if (p == "enabled")
    {
        out = to_json(o->enabled.get());
        return true;
    }
    if (p == "focus_policy")
    {
        out = to_json(o->focus_policy.get());
        return true;
    }
    if (p == "cursor")
    {
        out = to_json(o->cursor.get());
        return true;
    }
    if (p == "tooltip")
    {
        out = to_json(o->tooltip.get());
        return true;
    }
    if (p == "theme_override")
    {
        out = to_json(o->theme_override.get());
        return true;
    }
    if (p == "accepts_drops")
    {
        out = to_json(o->accepts_drops.get());
        return true;
    }
    if (p == "header_visible")
    {
        out = to_json(o->header_visible.get());
        return true;
    }
    if (p == "sorting_enabled")
    {
        out = to_json(o->sorting_enabled.get());
        return true;
    }
    if (p == "sort_column")
    {
        out = to_json(o->sort_column.get());
        return true;
    }
    if (p == "sort_order")
    {
        out = to_json(o->sort_order.get());
        return true;
    }
    if (p == "selection_mode")
    {
        out = to_json(o->selection_mode.get());
        return true;
    }
    if (p == "drag_enabled")
    {
        out = to_json(o->drag_enabled.get());
        return true;
    }
    if (p == "drop_enabled")
    {
        out = to_json(o->drop_enabled.get());
        return true;
    }
    return false;
}

bool set_TreeView(tokotachi::TreeView* o, const std::string& p, const Json& v)
{
    if (p == "x")
    {
        float t;
        if (!from_json(v, t))
        {
            return false;
        }
        o->x = t;
        return true;
    }
    if (p == "y")
    {
        float t;
        if (!from_json(v, t))
        {
            return false;
        }
        o->y = t;
        return true;
    }
    if (p == "width")
    {
        float t;
        if (!from_json(v, t))
        {
            return false;
        }
        o->width = t;
        return true;
    }
    if (p == "height")
    {
        float t;
        if (!from_json(v, t))
        {
            return false;
        }
        o->height = t;
        return true;
    }
    if (p == "minimum_size")
    {
        tokotachi::Size t;
        if (!from_json(v, t))
        {
            return false;
        }
        o->minimum_size = t;
        return true;
    }
    if (p == "maximum_size")
    {
        tokotachi::Size t;
        if (!from_json(v, t))
        {
            return false;
        }
        o->maximum_size = t;
        return true;
    }
    if (p == "visible")
    {
        bool t;
        if (!from_json(v, t))
        {
            return false;
        }
        o->visible = t;
        return true;
    }
    if (p == "enabled")
    {
        bool t;
        if (!from_json(v, t))
        {
            return false;
        }
        o->enabled = t;
        return true;
    }
    if (p == "focus_policy")
    {
        tokotachi::FocusPolicy t;
        if (!from_json(v, t))
        {
            return false;
        }
        o->focus_policy = t;
        return true;
    }
    if (p == "cursor")
    {
        tokotachi::MouseCursor t;
        if (!from_json(v, t))
        {
            return false;
        }
        o->cursor = t;
        return true;
    }
    if (p == "tooltip")
    {
        std::string t;
        if (!from_json(v, t))
        {
            return false;
        }
        o->tooltip = t;
        return true;
    }
    if (p == "theme_override")
    {
        tokotachi::ThemeOverride t;
        if (!from_json(v, t))
        {
            return false;
        }
        o->theme_override = t;
        return true;
    }
    if (p == "accepts_drops")
    {
        bool t;
        if (!from_json(v, t))
        {
            return false;
        }
        o->accepts_drops = t;
        return true;
    }
    if (p == "header_visible")
    {
        bool t;
        if (!from_json(v, t))
        {
            return false;
        }
        o->header_visible = t;
        return true;
    }
    if (p == "sorting_enabled")
    {
        bool t;
        if (!from_json(v, t))
        {
            return false;
        }
        o->sorting_enabled = t;
        return true;
    }
    if (p == "sort_column")
    {
        int t;
        if (!from_json(v, t))
        {
            return false;
        }
        o->sort_column = t;
        return true;
    }
    if (p == "sort_order")
    {
        tokotachi::SortOrder t;
        if (!from_json(v, t))
        {
            return false;
        }
        o->sort_order = t;
        return true;
    }
    if (p == "selection_mode")
    {
        tokotachi::SelectionMode t;
        if (!from_json(v, t))
        {
            return false;
        }
        o->selection_mode = t;
        return true;
    }
    if (p == "drag_enabled")
    {
        bool t;
        if (!from_json(v, t))
        {
            return false;
        }
        o->drag_enabled = t;
        return true;
    }
    if (p == "drop_enabled")
    {
        bool t;
        if (!from_json(v, t))
        {
            return false;
        }
        o->drop_enabled = t;
        return true;
    }
    if (p == "indentation")
    {
        float t;
        if (!from_json(v, t))
        {
            return false;
        }
        o->indentation = t;
        return true;
    }
    return false;
}

bool get_TreeView(tokotachi::TreeView* o, const std::string& p, Json& out)
{
    if (p == "x")
    {
        out = to_json(o->x.get());
        return true;
    }
    if (p == "y")
    {
        out = to_json(o->y.get());
        return true;
    }
    if (p == "width")
    {
        out = to_json(o->width.get());
        return true;
    }
    if (p == "height")
    {
        out = to_json(o->height.get());
        return true;
    }
    if (p == "minimum_size")
    {
        out = to_json(o->minimum_size.get());
        return true;
    }
    if (p == "maximum_size")
    {
        out = to_json(o->maximum_size.get());
        return true;
    }
    if (p == "visible")
    {
        out = to_json(o->visible.get());
        return true;
    }
    if (p == "enabled")
    {
        out = to_json(o->enabled.get());
        return true;
    }
    if (p == "focus_policy")
    {
        out = to_json(o->focus_policy.get());
        return true;
    }
    if (p == "cursor")
    {
        out = to_json(o->cursor.get());
        return true;
    }
    if (p == "tooltip")
    {
        out = to_json(o->tooltip.get());
        return true;
    }
    if (p == "theme_override")
    {
        out = to_json(o->theme_override.get());
        return true;
    }
    if (p == "accepts_drops")
    {
        out = to_json(o->accepts_drops.get());
        return true;
    }
    if (p == "header_visible")
    {
        out = to_json(o->header_visible.get());
        return true;
    }
    if (p == "sorting_enabled")
    {
        out = to_json(o->sorting_enabled.get());
        return true;
    }
    if (p == "sort_column")
    {
        out = to_json(o->sort_column.get());
        return true;
    }
    if (p == "sort_order")
    {
        out = to_json(o->sort_order.get());
        return true;
    }
    if (p == "selection_mode")
    {
        out = to_json(o->selection_mode.get());
        return true;
    }
    if (p == "drag_enabled")
    {
        out = to_json(o->drag_enabled.get());
        return true;
    }
    if (p == "drop_enabled")
    {
        out = to_json(o->drop_enabled.get());
        return true;
    }
    if (p == "indentation")
    {
        out = to_json(o->indentation.get());
        return true;
    }
    return false;
}

bool set_Widget(tokotachi::Widget* o, const std::string& p, const Json& v)
{
    if (p == "x")
    {
        float t;
        if (!from_json(v, t))
        {
            return false;
        }
        o->x = t;
        return true;
    }
    if (p == "y")
    {
        float t;
        if (!from_json(v, t))
        {
            return false;
        }
        o->y = t;
        return true;
    }
    if (p == "width")
    {
        float t;
        if (!from_json(v, t))
        {
            return false;
        }
        o->width = t;
        return true;
    }
    if (p == "height")
    {
        float t;
        if (!from_json(v, t))
        {
            return false;
        }
        o->height = t;
        return true;
    }
    if (p == "minimum_size")
    {
        tokotachi::Size t;
        if (!from_json(v, t))
        {
            return false;
        }
        o->minimum_size = t;
        return true;
    }
    if (p == "maximum_size")
    {
        tokotachi::Size t;
        if (!from_json(v, t))
        {
            return false;
        }
        o->maximum_size = t;
        return true;
    }
    if (p == "visible")
    {
        bool t;
        if (!from_json(v, t))
        {
            return false;
        }
        o->visible = t;
        return true;
    }
    if (p == "enabled")
    {
        bool t;
        if (!from_json(v, t))
        {
            return false;
        }
        o->enabled = t;
        return true;
    }
    if (p == "focus_policy")
    {
        tokotachi::FocusPolicy t;
        if (!from_json(v, t))
        {
            return false;
        }
        o->focus_policy = t;
        return true;
    }
    if (p == "cursor")
    {
        tokotachi::MouseCursor t;
        if (!from_json(v, t))
        {
            return false;
        }
        o->cursor = t;
        return true;
    }
    if (p == "tooltip")
    {
        std::string t;
        if (!from_json(v, t))
        {
            return false;
        }
        o->tooltip = t;
        return true;
    }
    if (p == "theme_override")
    {
        tokotachi::ThemeOverride t;
        if (!from_json(v, t))
        {
            return false;
        }
        o->theme_override = t;
        return true;
    }
    if (p == "accepts_drops")
    {
        bool t;
        if (!from_json(v, t))
        {
            return false;
        }
        o->accepts_drops = t;
        return true;
    }
    return false;
}

bool get_Widget(tokotachi::Widget* o, const std::string& p, Json& out)
{
    if (p == "x")
    {
        out = to_json(o->x.get());
        return true;
    }
    if (p == "y")
    {
        out = to_json(o->y.get());
        return true;
    }
    if (p == "width")
    {
        out = to_json(o->width.get());
        return true;
    }
    if (p == "height")
    {
        out = to_json(o->height.get());
        return true;
    }
    if (p == "minimum_size")
    {
        out = to_json(o->minimum_size.get());
        return true;
    }
    if (p == "maximum_size")
    {
        out = to_json(o->maximum_size.get());
        return true;
    }
    if (p == "visible")
    {
        out = to_json(o->visible.get());
        return true;
    }
    if (p == "enabled")
    {
        out = to_json(o->enabled.get());
        return true;
    }
    if (p == "focus_policy")
    {
        out = to_json(o->focus_policy.get());
        return true;
    }
    if (p == "cursor")
    {
        out = to_json(o->cursor.get());
        return true;
    }
    if (p == "tooltip")
    {
        out = to_json(o->tooltip.get());
        return true;
    }
    if (p == "theme_override")
    {
        out = to_json(o->theme_override.get());
        return true;
    }
    if (p == "accepts_drops")
    {
        out = to_json(o->accepts_drops.get());
        return true;
    }
    return false;
}

bool set_TabWidget(tokotachi::TabWidget* o, const std::string& p, const Json& v)
{
    if (p == "x")
    {
        float t;
        if (!from_json(v, t))
        {
            return false;
        }
        o->x = t;
        return true;
    }
    if (p == "y")
    {
        float t;
        if (!from_json(v, t))
        {
            return false;
        }
        o->y = t;
        return true;
    }
    if (p == "width")
    {
        float t;
        if (!from_json(v, t))
        {
            return false;
        }
        o->width = t;
        return true;
    }
    if (p == "height")
    {
        float t;
        if (!from_json(v, t))
        {
            return false;
        }
        o->height = t;
        return true;
    }
    if (p == "minimum_size")
    {
        tokotachi::Size t;
        if (!from_json(v, t))
        {
            return false;
        }
        o->minimum_size = t;
        return true;
    }
    if (p == "maximum_size")
    {
        tokotachi::Size t;
        if (!from_json(v, t))
        {
            return false;
        }
        o->maximum_size = t;
        return true;
    }
    if (p == "visible")
    {
        bool t;
        if (!from_json(v, t))
        {
            return false;
        }
        o->visible = t;
        return true;
    }
    if (p == "enabled")
    {
        bool t;
        if (!from_json(v, t))
        {
            return false;
        }
        o->enabled = t;
        return true;
    }
    if (p == "focus_policy")
    {
        tokotachi::FocusPolicy t;
        if (!from_json(v, t))
        {
            return false;
        }
        o->focus_policy = t;
        return true;
    }
    if (p == "cursor")
    {
        tokotachi::MouseCursor t;
        if (!from_json(v, t))
        {
            return false;
        }
        o->cursor = t;
        return true;
    }
    if (p == "tooltip")
    {
        std::string t;
        if (!from_json(v, t))
        {
            return false;
        }
        o->tooltip = t;
        return true;
    }
    if (p == "theme_override")
    {
        tokotachi::ThemeOverride t;
        if (!from_json(v, t))
        {
            return false;
        }
        o->theme_override = t;
        return true;
    }
    if (p == "accepts_drops")
    {
        bool t;
        if (!from_json(v, t))
        {
            return false;
        }
        o->accepts_drops = t;
        return true;
    }
    if (p == "current_index")
    {
        int t;
        if (!from_json(v, t))
        {
            return false;
        }
        o->current_index = t;
        return true;
    }
    return false;
}

bool get_TabWidget(tokotachi::TabWidget* o, const std::string& p, Json& out)
{
    if (p == "x")
    {
        out = to_json(o->x.get());
        return true;
    }
    if (p == "y")
    {
        out = to_json(o->y.get());
        return true;
    }
    if (p == "width")
    {
        out = to_json(o->width.get());
        return true;
    }
    if (p == "height")
    {
        out = to_json(o->height.get());
        return true;
    }
    if (p == "minimum_size")
    {
        out = to_json(o->minimum_size.get());
        return true;
    }
    if (p == "maximum_size")
    {
        out = to_json(o->maximum_size.get());
        return true;
    }
    if (p == "visible")
    {
        out = to_json(o->visible.get());
        return true;
    }
    if (p == "enabled")
    {
        out = to_json(o->enabled.get());
        return true;
    }
    if (p == "focus_policy")
    {
        out = to_json(o->focus_policy.get());
        return true;
    }
    if (p == "cursor")
    {
        out = to_json(o->cursor.get());
        return true;
    }
    if (p == "tooltip")
    {
        out = to_json(o->tooltip.get());
        return true;
    }
    if (p == "theme_override")
    {
        out = to_json(o->theme_override.get());
        return true;
    }
    if (p == "accepts_drops")
    {
        out = to_json(o->accepts_drops.get());
        return true;
    }
    if (p == "current_index")
    {
        out = to_json(o->current_index.get());
        return true;
    }
    return false;
}

bool set_Splitter(tokotachi::Splitter* o, const std::string& p, const Json& v)
{
    if (p == "x")
    {
        float t;
        if (!from_json(v, t))
        {
            return false;
        }
        o->x = t;
        return true;
    }
    if (p == "y")
    {
        float t;
        if (!from_json(v, t))
        {
            return false;
        }
        o->y = t;
        return true;
    }
    if (p == "width")
    {
        float t;
        if (!from_json(v, t))
        {
            return false;
        }
        o->width = t;
        return true;
    }
    if (p == "height")
    {
        float t;
        if (!from_json(v, t))
        {
            return false;
        }
        o->height = t;
        return true;
    }
    if (p == "minimum_size")
    {
        tokotachi::Size t;
        if (!from_json(v, t))
        {
            return false;
        }
        o->minimum_size = t;
        return true;
    }
    if (p == "maximum_size")
    {
        tokotachi::Size t;
        if (!from_json(v, t))
        {
            return false;
        }
        o->maximum_size = t;
        return true;
    }
    if (p == "visible")
    {
        bool t;
        if (!from_json(v, t))
        {
            return false;
        }
        o->visible = t;
        return true;
    }
    if (p == "enabled")
    {
        bool t;
        if (!from_json(v, t))
        {
            return false;
        }
        o->enabled = t;
        return true;
    }
    if (p == "focus_policy")
    {
        tokotachi::FocusPolicy t;
        if (!from_json(v, t))
        {
            return false;
        }
        o->focus_policy = t;
        return true;
    }
    if (p == "cursor")
    {
        tokotachi::MouseCursor t;
        if (!from_json(v, t))
        {
            return false;
        }
        o->cursor = t;
        return true;
    }
    if (p == "tooltip")
    {
        std::string t;
        if (!from_json(v, t))
        {
            return false;
        }
        o->tooltip = t;
        return true;
    }
    if (p == "theme_override")
    {
        tokotachi::ThemeOverride t;
        if (!from_json(v, t))
        {
            return false;
        }
        o->theme_override = t;
        return true;
    }
    if (p == "accepts_drops")
    {
        bool t;
        if (!from_json(v, t))
        {
            return false;
        }
        o->accepts_drops = t;
        return true;
    }
    if (p == "orientation")
    {
        tokotachi::Orientation t;
        if (!from_json(v, t))
        {
            return false;
        }
        o->orientation = t;
        return true;
    }
    if (p == "sizes")
    {
        std::vector<float> t;
        if (!from_json(v, t))
        {
            return false;
        }
        o->sizes = t;
        return true;
    }
    return false;
}

bool get_Splitter(tokotachi::Splitter* o, const std::string& p, Json& out)
{
    if (p == "x")
    {
        out = to_json(o->x.get());
        return true;
    }
    if (p == "y")
    {
        out = to_json(o->y.get());
        return true;
    }
    if (p == "width")
    {
        out = to_json(o->width.get());
        return true;
    }
    if (p == "height")
    {
        out = to_json(o->height.get());
        return true;
    }
    if (p == "minimum_size")
    {
        out = to_json(o->minimum_size.get());
        return true;
    }
    if (p == "maximum_size")
    {
        out = to_json(o->maximum_size.get());
        return true;
    }
    if (p == "visible")
    {
        out = to_json(o->visible.get());
        return true;
    }
    if (p == "enabled")
    {
        out = to_json(o->enabled.get());
        return true;
    }
    if (p == "focus_policy")
    {
        out = to_json(o->focus_policy.get());
        return true;
    }
    if (p == "cursor")
    {
        out = to_json(o->cursor.get());
        return true;
    }
    if (p == "tooltip")
    {
        out = to_json(o->tooltip.get());
        return true;
    }
    if (p == "theme_override")
    {
        out = to_json(o->theme_override.get());
        return true;
    }
    if (p == "accepts_drops")
    {
        out = to_json(o->accepts_drops.get());
        return true;
    }
    if (p == "orientation")
    {
        out = to_json(o->orientation.get());
        return true;
    }
    if (p == "sizes")
    {
        out = to_json(o->sizes.get());
        return true;
    }
    return false;
}

bool set_ScrollArea(tokotachi::ScrollArea* o, const std::string& p, const Json& v)
{
    if (p == "x")
    {
        float t;
        if (!from_json(v, t))
        {
            return false;
        }
        o->x = t;
        return true;
    }
    if (p == "y")
    {
        float t;
        if (!from_json(v, t))
        {
            return false;
        }
        o->y = t;
        return true;
    }
    if (p == "width")
    {
        float t;
        if (!from_json(v, t))
        {
            return false;
        }
        o->width = t;
        return true;
    }
    if (p == "height")
    {
        float t;
        if (!from_json(v, t))
        {
            return false;
        }
        o->height = t;
        return true;
    }
    if (p == "minimum_size")
    {
        tokotachi::Size t;
        if (!from_json(v, t))
        {
            return false;
        }
        o->minimum_size = t;
        return true;
    }
    if (p == "maximum_size")
    {
        tokotachi::Size t;
        if (!from_json(v, t))
        {
            return false;
        }
        o->maximum_size = t;
        return true;
    }
    if (p == "visible")
    {
        bool t;
        if (!from_json(v, t))
        {
            return false;
        }
        o->visible = t;
        return true;
    }
    if (p == "enabled")
    {
        bool t;
        if (!from_json(v, t))
        {
            return false;
        }
        o->enabled = t;
        return true;
    }
    if (p == "focus_policy")
    {
        tokotachi::FocusPolicy t;
        if (!from_json(v, t))
        {
            return false;
        }
        o->focus_policy = t;
        return true;
    }
    if (p == "cursor")
    {
        tokotachi::MouseCursor t;
        if (!from_json(v, t))
        {
            return false;
        }
        o->cursor = t;
        return true;
    }
    if (p == "tooltip")
    {
        std::string t;
        if (!from_json(v, t))
        {
            return false;
        }
        o->tooltip = t;
        return true;
    }
    if (p == "theme_override")
    {
        tokotachi::ThemeOverride t;
        if (!from_json(v, t))
        {
            return false;
        }
        o->theme_override = t;
        return true;
    }
    if (p == "accepts_drops")
    {
        bool t;
        if (!from_json(v, t))
        {
            return false;
        }
        o->accepts_drops = t;
        return true;
    }
    if (p == "fit_width")
    {
        bool t;
        if (!from_json(v, t))
        {
            return false;
        }
        o->fit_width = t;
        return true;
    }
    return false;
}

bool get_ScrollArea(tokotachi::ScrollArea* o, const std::string& p, Json& out)
{
    if (p == "x")
    {
        out = to_json(o->x.get());
        return true;
    }
    if (p == "y")
    {
        out = to_json(o->y.get());
        return true;
    }
    if (p == "width")
    {
        out = to_json(o->width.get());
        return true;
    }
    if (p == "height")
    {
        out = to_json(o->height.get());
        return true;
    }
    if (p == "minimum_size")
    {
        out = to_json(o->minimum_size.get());
        return true;
    }
    if (p == "maximum_size")
    {
        out = to_json(o->maximum_size.get());
        return true;
    }
    if (p == "visible")
    {
        out = to_json(o->visible.get());
        return true;
    }
    if (p == "enabled")
    {
        out = to_json(o->enabled.get());
        return true;
    }
    if (p == "focus_policy")
    {
        out = to_json(o->focus_policy.get());
        return true;
    }
    if (p == "cursor")
    {
        out = to_json(o->cursor.get());
        return true;
    }
    if (p == "tooltip")
    {
        out = to_json(o->tooltip.get());
        return true;
    }
    if (p == "theme_override")
    {
        out = to_json(o->theme_override.get());
        return true;
    }
    if (p == "accepts_drops")
    {
        out = to_json(o->accepts_drops.get());
        return true;
    }
    if (p == "fit_width")
    {
        out = to_json(o->fit_width.get());
        return true;
    }
    return false;
}

bool set_PaintBox(tokotachi::PaintBox* o, const std::string& p, const Json& v)
{
    if (p == "x")
    {
        float t;
        if (!from_json(v, t))
        {
            return false;
        }
        o->x = t;
        return true;
    }
    if (p == "y")
    {
        float t;
        if (!from_json(v, t))
        {
            return false;
        }
        o->y = t;
        return true;
    }
    if (p == "width")
    {
        float t;
        if (!from_json(v, t))
        {
            return false;
        }
        o->width = t;
        return true;
    }
    if (p == "height")
    {
        float t;
        if (!from_json(v, t))
        {
            return false;
        }
        o->height = t;
        return true;
    }
    if (p == "minimum_size")
    {
        tokotachi::Size t;
        if (!from_json(v, t))
        {
            return false;
        }
        o->minimum_size = t;
        return true;
    }
    if (p == "maximum_size")
    {
        tokotachi::Size t;
        if (!from_json(v, t))
        {
            return false;
        }
        o->maximum_size = t;
        return true;
    }
    if (p == "visible")
    {
        bool t;
        if (!from_json(v, t))
        {
            return false;
        }
        o->visible = t;
        return true;
    }
    if (p == "enabled")
    {
        bool t;
        if (!from_json(v, t))
        {
            return false;
        }
        o->enabled = t;
        return true;
    }
    if (p == "focus_policy")
    {
        tokotachi::FocusPolicy t;
        if (!from_json(v, t))
        {
            return false;
        }
        o->focus_policy = t;
        return true;
    }
    if (p == "cursor")
    {
        tokotachi::MouseCursor t;
        if (!from_json(v, t))
        {
            return false;
        }
        o->cursor = t;
        return true;
    }
    if (p == "tooltip")
    {
        std::string t;
        if (!from_json(v, t))
        {
            return false;
        }
        o->tooltip = t;
        return true;
    }
    if (p == "theme_override")
    {
        tokotachi::ThemeOverride t;
        if (!from_json(v, t))
        {
            return false;
        }
        o->theme_override = t;
        return true;
    }
    if (p == "accepts_drops")
    {
        bool t;
        if (!from_json(v, t))
        {
            return false;
        }
        o->accepts_drops = t;
        return true;
    }
    if (p == "cached")
    {
        bool t;
        if (!from_json(v, t))
        {
            return false;
        }
        o->cached = t;
        return true;
    }
    return false;
}

bool get_PaintBox(tokotachi::PaintBox* o, const std::string& p, Json& out)
{
    if (p == "x")
    {
        out = to_json(o->x.get());
        return true;
    }
    if (p == "y")
    {
        out = to_json(o->y.get());
        return true;
    }
    if (p == "width")
    {
        out = to_json(o->width.get());
        return true;
    }
    if (p == "height")
    {
        out = to_json(o->height.get());
        return true;
    }
    if (p == "minimum_size")
    {
        out = to_json(o->minimum_size.get());
        return true;
    }
    if (p == "maximum_size")
    {
        out = to_json(o->maximum_size.get());
        return true;
    }
    if (p == "visible")
    {
        out = to_json(o->visible.get());
        return true;
    }
    if (p == "enabled")
    {
        out = to_json(o->enabled.get());
        return true;
    }
    if (p == "focus_policy")
    {
        out = to_json(o->focus_policy.get());
        return true;
    }
    if (p == "cursor")
    {
        out = to_json(o->cursor.get());
        return true;
    }
    if (p == "tooltip")
    {
        out = to_json(o->tooltip.get());
        return true;
    }
    if (p == "theme_override")
    {
        out = to_json(o->theme_override.get());
        return true;
    }
    if (p == "accepts_drops")
    {
        out = to_json(o->accepts_drops.get());
        return true;
    }
    if (p == "cached")
    {
        out = to_json(o->cached.get());
        return true;
    }
    return false;
}

bool set_Timer(tokotachi::Timer* o, const std::string& p, const Json& v)
{
    if (p == "interval")
    {
        int t;
        if (!from_json(v, t))
        {
            return false;
        }
        o->interval = t;
        return true;
    }
    if (p == "repeat")
    {
        bool t;
        if (!from_json(v, t))
        {
            return false;
        }
        o->repeat = t;
        return true;
    }
    if (p == "enabled")
    {
        bool t;
        if (!from_json(v, t))
        {
            return false;
        }
        o->enabled = t;
        return true;
    }
    return false;
}

bool get_Timer(tokotachi::Timer* o, const std::string& p, Json& out)
{
    if (p == "interval")
    {
        out = to_json(o->interval.get());
        return true;
    }
    if (p == "repeat")
    {
        out = to_json(o->repeat.get());
        return true;
    }
    if (p == "enabled")
    {
        out = to_json(o->enabled.get());
        return true;
    }
    return false;
}

bool set_BoxLayout(tokotachi::BoxLayout* o, const std::string& p, const Json& v)
{
    if (p == "margins")
    {
        tokotachi::Margins t;
        if (!from_json(v, t))
        {
            return false;
        }
        o->margins = t;
        return true;
    }
    if (p == "orientation")
    {
        tokotachi::Orientation t;
        if (!from_json(v, t))
        {
            return false;
        }
        o->orientation = t;
        return true;
    }
    if (p == "spacing")
    {
        float t;
        if (!from_json(v, t))
        {
            return false;
        }
        o->spacing = t;
        return true;
    }
    return false;
}

bool get_BoxLayout(tokotachi::BoxLayout* o, const std::string& p, Json& out)
{
    if (p == "margins")
    {
        out = to_json(o->margins.get());
        return true;
    }
    if (p == "orientation")
    {
        out = to_json(o->orientation.get());
        return true;
    }
    if (p == "spacing")
    {
        out = to_json(o->spacing.get());
        return true;
    }
    return false;
}

bool set_GridLayout(tokotachi::GridLayout* o, const std::string& p, const Json& v)
{
    if (p == "margins")
    {
        tokotachi::Margins t;
        if (!from_json(v, t))
        {
            return false;
        }
        o->margins = t;
        return true;
    }
    if (p == "row_spacing")
    {
        float t;
        if (!from_json(v, t))
        {
            return false;
        }
        o->row_spacing = t;
        return true;
    }
    if (p == "column_spacing")
    {
        float t;
        if (!from_json(v, t))
        {
            return false;
        }
        o->column_spacing = t;
        return true;
    }
    return false;
}

bool get_GridLayout(tokotachi::GridLayout* o, const std::string& p, Json& out)
{
    if (p == "margins")
    {
        out = to_json(o->margins.get());
        return true;
    }
    if (p == "row_spacing")
    {
        out = to_json(o->row_spacing.get());
        return true;
    }
    if (p == "column_spacing")
    {
        out = to_json(o->column_spacing.get());
        return true;
    }
    return false;
}

bool set_PlaceLayout(tokotachi::PlaceLayout* o, const std::string& p, const Json& v)
{
    if (p == "margins")
    {
        tokotachi::Margins t;
        if (!from_json(v, t))
        {
            return false;
        }
        o->margins = t;
        return true;
    }
    return false;
}

bool get_PlaceLayout(tokotachi::PlaceLayout* o, const std::string& p, Json& out)
{
    if (p == "margins")
    {
        out = to_json(o->margins.get());
        return true;
    }
    return false;
}

bool set_Action(tokotachi::Action* o, const std::string& p, const Json& v)
{
    if (p == "text")
    {
        std::string t;
        if (!from_json(v, t))
        {
            return false;
        }
        o->text = t;
        return true;
    }
    if (p == "shortcut")
    {
        tokotachi::Shortcut t;
        if (!from_json(v, t))
        {
            return false;
        }
        o->shortcut = t;
        return true;
    }
    if (p == "icon")
    {
        tokotachi::Image t;
        if (!from_json(v, t))
        {
            return false;
        }
        o->icon = t;
        return true;
    }
    if (p == "tooltip")
    {
        std::string t;
        if (!from_json(v, t))
        {
            return false;
        }
        o->tooltip = t;
        return true;
    }
    if (p == "enabled")
    {
        bool t;
        if (!from_json(v, t))
        {
            return false;
        }
        o->enabled = t;
        return true;
    }
    if (p == "checkable")
    {
        bool t;
        if (!from_json(v, t))
        {
            return false;
        }
        o->checkable = t;
        return true;
    }
    if (p == "checked")
    {
        bool t;
        if (!from_json(v, t))
        {
            return false;
        }
        o->checked = t;
        return true;
    }
    return false;
}

bool get_Action(tokotachi::Action* o, const std::string& p, Json& out)
{
    if (p == "text")
    {
        out = to_json(o->text.get());
        return true;
    }
    if (p == "shortcut")
    {
        out = to_json(o->shortcut.get());
        return true;
    }
    if (p == "icon")
    {
        out = to_json(o->icon.get());
        return true;
    }
    if (p == "tooltip")
    {
        out = to_json(o->tooltip.get());
        return true;
    }
    if (p == "enabled")
    {
        out = to_json(o->enabled.get());
        return true;
    }
    if (p == "checkable")
    {
        out = to_json(o->checkable.get());
        return true;
    }
    if (p == "checked")
    {
        out = to_json(o->checked.get());
        return true;
    }
    return false;
}

bool set_MenuBar(tokotachi::MenuBar* o, const std::string& p, const Json& v)
{
    if (p == "x")
    {
        float t;
        if (!from_json(v, t))
        {
            return false;
        }
        o->x = t;
        return true;
    }
    if (p == "y")
    {
        float t;
        if (!from_json(v, t))
        {
            return false;
        }
        o->y = t;
        return true;
    }
    if (p == "width")
    {
        float t;
        if (!from_json(v, t))
        {
            return false;
        }
        o->width = t;
        return true;
    }
    if (p == "height")
    {
        float t;
        if (!from_json(v, t))
        {
            return false;
        }
        o->height = t;
        return true;
    }
    if (p == "minimum_size")
    {
        tokotachi::Size t;
        if (!from_json(v, t))
        {
            return false;
        }
        o->minimum_size = t;
        return true;
    }
    if (p == "maximum_size")
    {
        tokotachi::Size t;
        if (!from_json(v, t))
        {
            return false;
        }
        o->maximum_size = t;
        return true;
    }
    if (p == "visible")
    {
        bool t;
        if (!from_json(v, t))
        {
            return false;
        }
        o->visible = t;
        return true;
    }
    if (p == "enabled")
    {
        bool t;
        if (!from_json(v, t))
        {
            return false;
        }
        o->enabled = t;
        return true;
    }
    if (p == "focus_policy")
    {
        tokotachi::FocusPolicy t;
        if (!from_json(v, t))
        {
            return false;
        }
        o->focus_policy = t;
        return true;
    }
    if (p == "cursor")
    {
        tokotachi::MouseCursor t;
        if (!from_json(v, t))
        {
            return false;
        }
        o->cursor = t;
        return true;
    }
    if (p == "tooltip")
    {
        std::string t;
        if (!from_json(v, t))
        {
            return false;
        }
        o->tooltip = t;
        return true;
    }
    if (p == "theme_override")
    {
        tokotachi::ThemeOverride t;
        if (!from_json(v, t))
        {
            return false;
        }
        o->theme_override = t;
        return true;
    }
    if (p == "accepts_drops")
    {
        bool t;
        if (!from_json(v, t))
        {
            return false;
        }
        o->accepts_drops = t;
        return true;
    }
    return false;
}

bool get_MenuBar(tokotachi::MenuBar* o, const std::string& p, Json& out)
{
    if (p == "x")
    {
        out = to_json(o->x.get());
        return true;
    }
    if (p == "y")
    {
        out = to_json(o->y.get());
        return true;
    }
    if (p == "width")
    {
        out = to_json(o->width.get());
        return true;
    }
    if (p == "height")
    {
        out = to_json(o->height.get());
        return true;
    }
    if (p == "minimum_size")
    {
        out = to_json(o->minimum_size.get());
        return true;
    }
    if (p == "maximum_size")
    {
        out = to_json(o->maximum_size.get());
        return true;
    }
    if (p == "visible")
    {
        out = to_json(o->visible.get());
        return true;
    }
    if (p == "enabled")
    {
        out = to_json(o->enabled.get());
        return true;
    }
    if (p == "focus_policy")
    {
        out = to_json(o->focus_policy.get());
        return true;
    }
    if (p == "cursor")
    {
        out = to_json(o->cursor.get());
        return true;
    }
    if (p == "tooltip")
    {
        out = to_json(o->tooltip.get());
        return true;
    }
    if (p == "theme_override")
    {
        out = to_json(o->theme_override.get());
        return true;
    }
    if (p == "accepts_drops")
    {
        out = to_json(o->accepts_drops.get());
        return true;
    }
    return false;
}

bool set_Menu(tokotachi::Menu* o, const std::string& p, const Json& v)
{
    if (p == "title")
    {
        std::string t;
        if (!from_json(v, t))
        {
            return false;
        }
        o->title = t;
        return true;
    }
    return false;
}

bool get_Menu(tokotachi::Menu* o, const std::string& p, Json& out)
{
    if (p == "title")
    {
        out = to_json(o->title.get());
        return true;
    }
    return false;
}

bool set_ToolBar(tokotachi::ToolBar* o, const std::string& p, const Json& v)
{
    if (p == "x")
    {
        float t;
        if (!from_json(v, t))
        {
            return false;
        }
        o->x = t;
        return true;
    }
    if (p == "y")
    {
        float t;
        if (!from_json(v, t))
        {
            return false;
        }
        o->y = t;
        return true;
    }
    if (p == "width")
    {
        float t;
        if (!from_json(v, t))
        {
            return false;
        }
        o->width = t;
        return true;
    }
    if (p == "height")
    {
        float t;
        if (!from_json(v, t))
        {
            return false;
        }
        o->height = t;
        return true;
    }
    if (p == "minimum_size")
    {
        tokotachi::Size t;
        if (!from_json(v, t))
        {
            return false;
        }
        o->minimum_size = t;
        return true;
    }
    if (p == "maximum_size")
    {
        tokotachi::Size t;
        if (!from_json(v, t))
        {
            return false;
        }
        o->maximum_size = t;
        return true;
    }
    if (p == "visible")
    {
        bool t;
        if (!from_json(v, t))
        {
            return false;
        }
        o->visible = t;
        return true;
    }
    if (p == "enabled")
    {
        bool t;
        if (!from_json(v, t))
        {
            return false;
        }
        o->enabled = t;
        return true;
    }
    if (p == "focus_policy")
    {
        tokotachi::FocusPolicy t;
        if (!from_json(v, t))
        {
            return false;
        }
        o->focus_policy = t;
        return true;
    }
    if (p == "cursor")
    {
        tokotachi::MouseCursor t;
        if (!from_json(v, t))
        {
            return false;
        }
        o->cursor = t;
        return true;
    }
    if (p == "tooltip")
    {
        std::string t;
        if (!from_json(v, t))
        {
            return false;
        }
        o->tooltip = t;
        return true;
    }
    if (p == "theme_override")
    {
        tokotachi::ThemeOverride t;
        if (!from_json(v, t))
        {
            return false;
        }
        o->theme_override = t;
        return true;
    }
    if (p == "accepts_drops")
    {
        bool t;
        if (!from_json(v, t))
        {
            return false;
        }
        o->accepts_drops = t;
        return true;
    }
    if (p == "button_style")
    {
        tokotachi::ToolButtonStyle t;
        if (!from_json(v, t))
        {
            return false;
        }
        o->button_style = t;
        return true;
    }
    return false;
}

bool get_ToolBar(tokotachi::ToolBar* o, const std::string& p, Json& out)
{
    if (p == "x")
    {
        out = to_json(o->x.get());
        return true;
    }
    if (p == "y")
    {
        out = to_json(o->y.get());
        return true;
    }
    if (p == "width")
    {
        out = to_json(o->width.get());
        return true;
    }
    if (p == "height")
    {
        out = to_json(o->height.get());
        return true;
    }
    if (p == "minimum_size")
    {
        out = to_json(o->minimum_size.get());
        return true;
    }
    if (p == "maximum_size")
    {
        out = to_json(o->maximum_size.get());
        return true;
    }
    if (p == "visible")
    {
        out = to_json(o->visible.get());
        return true;
    }
    if (p == "enabled")
    {
        out = to_json(o->enabled.get());
        return true;
    }
    if (p == "focus_policy")
    {
        out = to_json(o->focus_policy.get());
        return true;
    }
    if (p == "cursor")
    {
        out = to_json(o->cursor.get());
        return true;
    }
    if (p == "tooltip")
    {
        out = to_json(o->tooltip.get());
        return true;
    }
    if (p == "theme_override")
    {
        out = to_json(o->theme_override.get());
        return true;
    }
    if (p == "accepts_drops")
    {
        out = to_json(o->accepts_drops.get());
        return true;
    }
    if (p == "button_style")
    {
        out = to_json(o->button_style.get());
        return true;
    }
    return false;
}

bool set_StatusBar(tokotachi::StatusBar* o, const std::string& p, const Json& v)
{
    if (p == "x")
    {
        float t;
        if (!from_json(v, t))
        {
            return false;
        }
        o->x = t;
        return true;
    }
    if (p == "y")
    {
        float t;
        if (!from_json(v, t))
        {
            return false;
        }
        o->y = t;
        return true;
    }
    if (p == "width")
    {
        float t;
        if (!from_json(v, t))
        {
            return false;
        }
        o->width = t;
        return true;
    }
    if (p == "height")
    {
        float t;
        if (!from_json(v, t))
        {
            return false;
        }
        o->height = t;
        return true;
    }
    if (p == "minimum_size")
    {
        tokotachi::Size t;
        if (!from_json(v, t))
        {
            return false;
        }
        o->minimum_size = t;
        return true;
    }
    if (p == "maximum_size")
    {
        tokotachi::Size t;
        if (!from_json(v, t))
        {
            return false;
        }
        o->maximum_size = t;
        return true;
    }
    if (p == "visible")
    {
        bool t;
        if (!from_json(v, t))
        {
            return false;
        }
        o->visible = t;
        return true;
    }
    if (p == "enabled")
    {
        bool t;
        if (!from_json(v, t))
        {
            return false;
        }
        o->enabled = t;
        return true;
    }
    if (p == "focus_policy")
    {
        tokotachi::FocusPolicy t;
        if (!from_json(v, t))
        {
            return false;
        }
        o->focus_policy = t;
        return true;
    }
    if (p == "cursor")
    {
        tokotachi::MouseCursor t;
        if (!from_json(v, t))
        {
            return false;
        }
        o->cursor = t;
        return true;
    }
    if (p == "tooltip")
    {
        std::string t;
        if (!from_json(v, t))
        {
            return false;
        }
        o->tooltip = t;
        return true;
    }
    if (p == "theme_override")
    {
        tokotachi::ThemeOverride t;
        if (!from_json(v, t))
        {
            return false;
        }
        o->theme_override = t;
        return true;
    }
    if (p == "accepts_drops")
    {
        bool t;
        if (!from_json(v, t))
        {
            return false;
        }
        o->accepts_drops = t;
        return true;
    }
    if (p == "text")
    {
        std::string t;
        if (!from_json(v, t))
        {
            return false;
        }
        o->text = t;
        return true;
    }
    return false;
}

bool get_StatusBar(tokotachi::StatusBar* o, const std::string& p, Json& out)
{
    if (p == "x")
    {
        out = to_json(o->x.get());
        return true;
    }
    if (p == "y")
    {
        out = to_json(o->y.get());
        return true;
    }
    if (p == "width")
    {
        out = to_json(o->width.get());
        return true;
    }
    if (p == "height")
    {
        out = to_json(o->height.get());
        return true;
    }
    if (p == "minimum_size")
    {
        out = to_json(o->minimum_size.get());
        return true;
    }
    if (p == "maximum_size")
    {
        out = to_json(o->maximum_size.get());
        return true;
    }
    if (p == "visible")
    {
        out = to_json(o->visible.get());
        return true;
    }
    if (p == "enabled")
    {
        out = to_json(o->enabled.get());
        return true;
    }
    if (p == "focus_policy")
    {
        out = to_json(o->focus_policy.get());
        return true;
    }
    if (p == "cursor")
    {
        out = to_json(o->cursor.get());
        return true;
    }
    if (p == "tooltip")
    {
        out = to_json(o->tooltip.get());
        return true;
    }
    if (p == "theme_override")
    {
        out = to_json(o->theme_override.get());
        return true;
    }
    if (p == "accepts_drops")
    {
        out = to_json(o->accepts_drops.get());
        return true;
    }
    if (p == "text")
    {
        out = to_json(o->text.get());
        return true;
    }
    return false;
}

bool set_Tab(tokotachi::Tab* o, const std::string& p, const Json& v)
{
    if (p == "title")
    {
        std::string t;
        if (!from_json(v, t))
        {
            return false;
        }
        o->title = t;
        return true;
    }
    if (p == "enabled")
    {
        bool t;
        if (!from_json(v, t))
        {
            return false;
        }
        o->enabled = t;
        return true;
    }
    return false;
}

bool get_Tab(tokotachi::Tab* o, const std::string& p, Json& out)
{
    if (p == "title")
    {
        out = to_json(o->title.get());
        return true;
    }
    if (p == "enabled")
    {
        out = to_json(o->enabled.get());
        return true;
    }
    return false;
}

bool set_SplitterPane(tokotachi::SplitterPane* o, const std::string& p, const Json& v)
{
    if (p == "stretch")
    {
        int t;
        if (!from_json(v, t))
        {
            return false;
        }
        o->stretch = t;
        return true;
    }
    if (p == "collapsible")
    {
        bool t;
        if (!from_json(v, t))
        {
            return false;
        }
        o->collapsible = t;
        return true;
    }
    return false;
}

bool get_SplitterPane(tokotachi::SplitterPane* o, const std::string& p, Json& out)
{
    if (p == "stretch")
    {
        out = to_json(o->stretch.get());
        return true;
    }
    if (p == "collapsible")
    {
        out = to_json(o->collapsible.get());
        return true;
    }
    return false;
}

bool set_Column(tokotachi::Column* o, const std::string& p, const Json& v)
{
    if (p == "title")
    {
        std::string t;
        if (!from_json(v, t))
        {
            return false;
        }
        o->title = t;
        return true;
    }
    if (p == "width")
    {
        float t;
        if (!from_json(v, t))
        {
            return false;
        }
        o->width = t;
        return true;
    }
    if (p == "alignment")
    {
        tokotachi::Align t;
        if (!from_json(v, t))
        {
            return false;
        }
        o->alignment = t;
        return true;
    }
    return false;
}

bool get_Column(tokotachi::Column* o, const std::string& p, Json& out)
{
    if (p == "title")
    {
        out = to_json(o->title.get());
        return true;
    }
    if (p == "width")
    {
        out = to_json(o->width.get());
        return true;
    }
    if (p == "alignment")
    {
        out = to_json(o->alignment.get());
        return true;
    }
    return false;
}

}  // namespace

tokotachi::Object* create(const std::string& cls, tokotachi::Object* parent)
{
    tokotachi::Widget* widget = dynamic_cast<tokotachi::Widget*>(parent);
    if (cls == "Button")
    {
        return new tokotachi::Button(widget);
    }
    if (cls == "CheckBox")
    {
        return new tokotachi::CheckBox(widget);
    }
    if (cls == "RadioButton")
    {
        return new tokotachi::RadioButton(widget);
    }
    if (cls == "LineEdit")
    {
        return new tokotachi::LineEdit(widget);
    }
    if (cls == "TextEdit")
    {
        return new tokotachi::TextEdit(widget);
    }
    if (cls == "SpinBox")
    {
        return new tokotachi::SpinBox(widget);
    }
    if (cls == "Slider")
    {
        return new tokotachi::Slider(widget);
    }
    if (cls == "ComboBox")
    {
        return new tokotachi::ComboBox(widget);
    }
    if (cls == "Label")
    {
        return new tokotachi::Label(widget);
    }
    if (cls == "ProgressBar")
    {
        return new tokotachi::ProgressBar(widget);
    }
    if (cls == "ImageBox")
    {
        return new tokotachi::ImageBox(widget);
    }
    if (cls == "ListView")
    {
        return new tokotachi::ListView(widget);
    }
    if (cls == "TreeView")
    {
        return new tokotachi::TreeView(widget);
    }
    if (cls == "Widget")
    {
        return new tokotachi::Widget(widget);
    }
    if (cls == "TabWidget")
    {
        return new tokotachi::TabWidget(widget);
    }
    if (cls == "Splitter")
    {
        return new tokotachi::Splitter(widget);
    }
    if (cls == "ScrollArea")
    {
        return new tokotachi::ScrollArea(widget);
    }
    if (cls == "PaintBox")
    {
        return new tokotachi::PaintBox(widget);
    }
    if (cls == "Timer")
    {
        return new tokotachi::Timer(parent);
    }
    if (cls == "BoxLayout")
    {
        return widget ? new tokotachi::BoxLayout(widget) : nullptr;
    }
    if (cls == "GridLayout")
    {
        return widget ? new tokotachi::GridLayout(widget) : nullptr;
    }
    if (cls == "PlaceLayout")
    {
        return widget ? new tokotachi::PlaceLayout(widget) : nullptr;
    }
    if (cls == "Action")
    {
        return new tokotachi::Action(parent);
    }
    if (cls == "MenuBar")
    {
        return new tokotachi::MenuBar(widget);
    }
    if (cls == "ToolBar")
    {
        return new tokotachi::ToolBar(widget);
    }
    if (cls == "StatusBar")
    {
        return new tokotachi::StatusBar(widget);
    }
    return nullptr;
}

bool set_property(tokotachi::Object* object, const std::string& cls, const std::string& property, const Json& value)
{
    if (cls == "Window")
    {
        return set_Window(static_cast<tokotachi::Window*>(object), property, value);
    }
    if (cls == "Dialog")
    {
        return set_Dialog(static_cast<tokotachi::Dialog*>(object), property, value);
    }
    if (cls == "Button")
    {
        return set_Button(static_cast<tokotachi::Button*>(object), property, value);
    }
    if (cls == "CheckBox")
    {
        return set_CheckBox(static_cast<tokotachi::CheckBox*>(object), property, value);
    }
    if (cls == "RadioButton")
    {
        return set_RadioButton(static_cast<tokotachi::RadioButton*>(object), property, value);
    }
    if (cls == "LineEdit")
    {
        return set_LineEdit(static_cast<tokotachi::LineEdit*>(object), property, value);
    }
    if (cls == "TextEdit")
    {
        return set_TextEdit(static_cast<tokotachi::TextEdit*>(object), property, value);
    }
    if (cls == "SpinBox")
    {
        return set_SpinBox(static_cast<tokotachi::SpinBox*>(object), property, value);
    }
    if (cls == "Slider")
    {
        return set_Slider(static_cast<tokotachi::Slider*>(object), property, value);
    }
    if (cls == "ComboBox")
    {
        return set_ComboBox(static_cast<tokotachi::ComboBox*>(object), property, value);
    }
    if (cls == "Label")
    {
        return set_Label(static_cast<tokotachi::Label*>(object), property, value);
    }
    if (cls == "ProgressBar")
    {
        return set_ProgressBar(static_cast<tokotachi::ProgressBar*>(object), property, value);
    }
    if (cls == "ImageBox")
    {
        return set_ImageBox(static_cast<tokotachi::ImageBox*>(object), property, value);
    }
    if (cls == "ListView")
    {
        return set_ListView(static_cast<tokotachi::ListView*>(object), property, value);
    }
    if (cls == "TreeView")
    {
        return set_TreeView(static_cast<tokotachi::TreeView*>(object), property, value);
    }
    if (cls == "Widget")
    {
        return set_Widget(static_cast<tokotachi::Widget*>(object), property, value);
    }
    if (cls == "TabWidget")
    {
        return set_TabWidget(static_cast<tokotachi::TabWidget*>(object), property, value);
    }
    if (cls == "Splitter")
    {
        return set_Splitter(static_cast<tokotachi::Splitter*>(object), property, value);
    }
    if (cls == "ScrollArea")
    {
        return set_ScrollArea(static_cast<tokotachi::ScrollArea*>(object), property, value);
    }
    if (cls == "PaintBox")
    {
        return set_PaintBox(static_cast<tokotachi::PaintBox*>(object), property, value);
    }
    if (cls == "Timer")
    {
        return set_Timer(static_cast<tokotachi::Timer*>(object), property, value);
    }
    if (cls == "BoxLayout")
    {
        return set_BoxLayout(static_cast<tokotachi::BoxLayout*>(object), property, value);
    }
    if (cls == "GridLayout")
    {
        return set_GridLayout(static_cast<tokotachi::GridLayout*>(object), property, value);
    }
    if (cls == "PlaceLayout")
    {
        return set_PlaceLayout(static_cast<tokotachi::PlaceLayout*>(object), property, value);
    }
    if (cls == "Action")
    {
        return set_Action(static_cast<tokotachi::Action*>(object), property, value);
    }
    if (cls == "MenuBar")
    {
        return set_MenuBar(static_cast<tokotachi::MenuBar*>(object), property, value);
    }
    if (cls == "Menu")
    {
        return set_Menu(static_cast<tokotachi::Menu*>(object), property, value);
    }
    if (cls == "ToolBar")
    {
        return set_ToolBar(static_cast<tokotachi::ToolBar*>(object), property, value);
    }
    if (cls == "StatusBar")
    {
        return set_StatusBar(static_cast<tokotachi::StatusBar*>(object), property, value);
    }
    if (cls == "Tab")
    {
        return set_Tab(static_cast<tokotachi::Tab*>(object), property, value);
    }
    if (cls == "SplitterPane")
    {
        return set_SplitterPane(static_cast<tokotachi::SplitterPane*>(object), property, value);
    }
    if (cls == "Column")
    {
        return set_Column(static_cast<tokotachi::Column*>(object), property, value);
    }
    return false;
}

bool get_property(tokotachi::Object* object, const std::string& cls, const std::string& property, Json& out)
{
    if (cls == "Window")
    {
        return get_Window(static_cast<tokotachi::Window*>(object), property, out);
    }
    if (cls == "Dialog")
    {
        return get_Dialog(static_cast<tokotachi::Dialog*>(object), property, out);
    }
    if (cls == "Button")
    {
        return get_Button(static_cast<tokotachi::Button*>(object), property, out);
    }
    if (cls == "CheckBox")
    {
        return get_CheckBox(static_cast<tokotachi::CheckBox*>(object), property, out);
    }
    if (cls == "RadioButton")
    {
        return get_RadioButton(static_cast<tokotachi::RadioButton*>(object), property, out);
    }
    if (cls == "LineEdit")
    {
        return get_LineEdit(static_cast<tokotachi::LineEdit*>(object), property, out);
    }
    if (cls == "TextEdit")
    {
        return get_TextEdit(static_cast<tokotachi::TextEdit*>(object), property, out);
    }
    if (cls == "SpinBox")
    {
        return get_SpinBox(static_cast<tokotachi::SpinBox*>(object), property, out);
    }
    if (cls == "Slider")
    {
        return get_Slider(static_cast<tokotachi::Slider*>(object), property, out);
    }
    if (cls == "ComboBox")
    {
        return get_ComboBox(static_cast<tokotachi::ComboBox*>(object), property, out);
    }
    if (cls == "Label")
    {
        return get_Label(static_cast<tokotachi::Label*>(object), property, out);
    }
    if (cls == "ProgressBar")
    {
        return get_ProgressBar(static_cast<tokotachi::ProgressBar*>(object), property, out);
    }
    if (cls == "ImageBox")
    {
        return get_ImageBox(static_cast<tokotachi::ImageBox*>(object), property, out);
    }
    if (cls == "ListView")
    {
        return get_ListView(static_cast<tokotachi::ListView*>(object), property, out);
    }
    if (cls == "TreeView")
    {
        return get_TreeView(static_cast<tokotachi::TreeView*>(object), property, out);
    }
    if (cls == "Widget")
    {
        return get_Widget(static_cast<tokotachi::Widget*>(object), property, out);
    }
    if (cls == "TabWidget")
    {
        return get_TabWidget(static_cast<tokotachi::TabWidget*>(object), property, out);
    }
    if (cls == "Splitter")
    {
        return get_Splitter(static_cast<tokotachi::Splitter*>(object), property, out);
    }
    if (cls == "ScrollArea")
    {
        return get_ScrollArea(static_cast<tokotachi::ScrollArea*>(object), property, out);
    }
    if (cls == "PaintBox")
    {
        return get_PaintBox(static_cast<tokotachi::PaintBox*>(object), property, out);
    }
    if (cls == "Timer")
    {
        return get_Timer(static_cast<tokotachi::Timer*>(object), property, out);
    }
    if (cls == "BoxLayout")
    {
        return get_BoxLayout(static_cast<tokotachi::BoxLayout*>(object), property, out);
    }
    if (cls == "GridLayout")
    {
        return get_GridLayout(static_cast<tokotachi::GridLayout*>(object), property, out);
    }
    if (cls == "PlaceLayout")
    {
        return get_PlaceLayout(static_cast<tokotachi::PlaceLayout*>(object), property, out);
    }
    if (cls == "Action")
    {
        return get_Action(static_cast<tokotachi::Action*>(object), property, out);
    }
    if (cls == "MenuBar")
    {
        return get_MenuBar(static_cast<tokotachi::MenuBar*>(object), property, out);
    }
    if (cls == "Menu")
    {
        return get_Menu(static_cast<tokotachi::Menu*>(object), property, out);
    }
    if (cls == "ToolBar")
    {
        return get_ToolBar(static_cast<tokotachi::ToolBar*>(object), property, out);
    }
    if (cls == "StatusBar")
    {
        return get_StatusBar(static_cast<tokotachi::StatusBar*>(object), property, out);
    }
    if (cls == "Tab")
    {
        return get_Tab(static_cast<tokotachi::Tab*>(object), property, out);
    }
    if (cls == "SplitterPane")
    {
        return get_SplitterPane(static_cast<tokotachi::SplitterPane*>(object), property, out);
    }
    if (cls == "Column")
    {
        return get_Column(static_cast<tokotachi::Column*>(object), property, out);
    }
    return false;
}

std::vector<std::string> property_names(const std::string& cls)
{
    if (cls == "Window")
    {
        return std::vector<std::string>{"x", "y", "width", "height", "minimum_size", "maximum_size", "visible", "enabled", "focus_policy", "cursor", "tooltip", "theme_override", "accepts_drops", "title", "background"};
    }
    if (cls == "Dialog")
    {
        return std::vector<std::string>{"x", "y", "width", "height", "minimum_size", "maximum_size", "visible", "enabled", "focus_policy", "cursor", "tooltip", "theme_override", "accepts_drops", "title", "background"};
    }
    if (cls == "Button")
    {
        return std::vector<std::string>{"x", "y", "width", "height", "minimum_size", "maximum_size", "visible", "enabled", "focus_policy", "cursor", "tooltip", "theme_override", "accepts_drops", "text", "font", "style", "icon", "icon_size"};
    }
    if (cls == "CheckBox")
    {
        return std::vector<std::string>{"x", "y", "width", "height", "minimum_size", "maximum_size", "visible", "enabled", "focus_policy", "cursor", "tooltip", "theme_override", "accepts_drops", "text", "font", "checked"};
    }
    if (cls == "RadioButton")
    {
        return std::vector<std::string>{"x", "y", "width", "height", "minimum_size", "maximum_size", "visible", "enabled", "focus_policy", "cursor", "tooltip", "theme_override", "accepts_drops", "text", "font", "value", "group_value"};
    }
    if (cls == "LineEdit")
    {
        return std::vector<std::string>{"x", "y", "width", "height", "minimum_size", "maximum_size", "visible", "enabled", "focus_policy", "cursor", "tooltip", "theme_override", "accepts_drops", "text", "placeholder_text", "font"};
    }
    if (cls == "TextEdit")
    {
        return std::vector<std::string>{"x", "y", "width", "height", "minimum_size", "maximum_size", "visible", "enabled", "focus_policy", "cursor", "tooltip", "theme_override", "accepts_drops", "text", "placeholder_text", "font", "word_wrap"};
    }
    if (cls == "SpinBox")
    {
        return std::vector<std::string>{"x", "y", "width", "height", "minimum_size", "maximum_size", "visible", "enabled", "focus_policy", "cursor", "tooltip", "theme_override", "accepts_drops", "minimum", "maximum", "value", "single_step", "page_step", "decimals"};
    }
    if (cls == "Slider")
    {
        return std::vector<std::string>{"x", "y", "width", "height", "minimum_size", "maximum_size", "visible", "enabled", "focus_policy", "cursor", "tooltip", "theme_override", "accepts_drops", "minimum", "maximum", "value", "single_step", "page_step", "orientation"};
    }
    if (cls == "ComboBox")
    {
        return std::vector<std::string>{"x", "y", "width", "height", "minimum_size", "maximum_size", "visible", "enabled", "focus_policy", "cursor", "tooltip", "theme_override", "accepts_drops", "items", "current_index", "placeholder_text", "font"};
    }
    if (cls == "Label")
    {
        return std::vector<std::string>{"x", "y", "width", "height", "minimum_size", "maximum_size", "visible", "enabled", "focus_policy", "cursor", "tooltip", "theme_override", "accepts_drops", "text", "font", "color", "alignment", "word_wrap", "icon", "icon_size"};
    }
    if (cls == "ProgressBar")
    {
        return std::vector<std::string>{"x", "y", "width", "height", "minimum_size", "maximum_size", "visible", "enabled", "focus_policy", "cursor", "tooltip", "theme_override", "accepts_drops", "minimum", "maximum", "value", "text_visible"};
    }
    if (cls == "ImageBox")
    {
        return std::vector<std::string>{"x", "y", "width", "height", "minimum_size", "maximum_size", "visible", "enabled", "focus_policy", "cursor", "tooltip", "theme_override", "accepts_drops", "image", "fit"};
    }
    if (cls == "ListView")
    {
        return std::vector<std::string>{"x", "y", "width", "height", "minimum_size", "maximum_size", "visible", "enabled", "focus_policy", "cursor", "tooltip", "theme_override", "accepts_drops", "header_visible", "sorting_enabled", "sort_column", "sort_order", "selection_mode", "drag_enabled", "drop_enabled"};
    }
    if (cls == "TreeView")
    {
        return std::vector<std::string>{"x", "y", "width", "height", "minimum_size", "maximum_size", "visible", "enabled", "focus_policy", "cursor", "tooltip", "theme_override", "accepts_drops", "header_visible", "sorting_enabled", "sort_column", "sort_order", "selection_mode", "drag_enabled", "drop_enabled", "indentation"};
    }
    if (cls == "Widget")
    {
        return std::vector<std::string>{"x", "y", "width", "height", "minimum_size", "maximum_size", "visible", "enabled", "focus_policy", "cursor", "tooltip", "theme_override", "accepts_drops"};
    }
    if (cls == "TabWidget")
    {
        return std::vector<std::string>{"x", "y", "width", "height", "minimum_size", "maximum_size", "visible", "enabled", "focus_policy", "cursor", "tooltip", "theme_override", "accepts_drops", "current_index"};
    }
    if (cls == "Splitter")
    {
        return std::vector<std::string>{"x", "y", "width", "height", "minimum_size", "maximum_size", "visible", "enabled", "focus_policy", "cursor", "tooltip", "theme_override", "accepts_drops", "orientation", "sizes"};
    }
    if (cls == "ScrollArea")
    {
        return std::vector<std::string>{"x", "y", "width", "height", "minimum_size", "maximum_size", "visible", "enabled", "focus_policy", "cursor", "tooltip", "theme_override", "accepts_drops", "fit_width"};
    }
    if (cls == "PaintBox")
    {
        return std::vector<std::string>{"x", "y", "width", "height", "minimum_size", "maximum_size", "visible", "enabled", "focus_policy", "cursor", "tooltip", "theme_override", "accepts_drops", "cached"};
    }
    if (cls == "Timer")
    {
        return std::vector<std::string>{"interval", "repeat", "enabled"};
    }
    if (cls == "BoxLayout")
    {
        return std::vector<std::string>{"margins", "orientation", "spacing"};
    }
    if (cls == "GridLayout")
    {
        return std::vector<std::string>{"margins", "row_spacing", "column_spacing"};
    }
    if (cls == "PlaceLayout")
    {
        return std::vector<std::string>{"margins"};
    }
    if (cls == "Action")
    {
        return std::vector<std::string>{"text", "shortcut", "icon", "tooltip", "enabled", "checkable", "checked"};
    }
    if (cls == "MenuBar")
    {
        return std::vector<std::string>{"x", "y", "width", "height", "minimum_size", "maximum_size", "visible", "enabled", "focus_policy", "cursor", "tooltip", "theme_override", "accepts_drops"};
    }
    if (cls == "Menu")
    {
        return std::vector<std::string>{"title"};
    }
    if (cls == "ToolBar")
    {
        return std::vector<std::string>{"x", "y", "width", "height", "minimum_size", "maximum_size", "visible", "enabled", "focus_policy", "cursor", "tooltip", "theme_override", "accepts_drops", "button_style"};
    }
    if (cls == "StatusBar")
    {
        return std::vector<std::string>{"x", "y", "width", "height", "minimum_size", "maximum_size", "visible", "enabled", "focus_policy", "cursor", "tooltip", "theme_override", "accepts_drops", "text"};
    }
    if (cls == "Tab")
    {
        return std::vector<std::string>{"title", "enabled"};
    }
    if (cls == "SplitterPane")
    {
        return std::vector<std::string>{"stretch", "collapsible"};
    }
    if (cls == "Column")
    {
        return std::vector<std::string>{"title", "width", "alignment"};
    }
    return std::vector<std::string>();
}

std::vector<std::string> creatable_classes()
{
    return std::vector<std::string>{"Button", "CheckBox", "RadioButton", "LineEdit", "TextEdit", "SpinBox", "Slider", "ComboBox", "Label", "ProgressBar", "ImageBox", "ListView", "TreeView", "Widget", "TabWidget", "Splitter", "ScrollArea", "PaintBox", "Timer", "BoxLayout", "GridLayout", "PlaceLayout", "Action", "MenuBar", "ToolBar", "StatusBar"};
}

}  // namespace ttc
