#include <tokotachi/windows/popup.hpp>

#include <algorithm>
#include <cmath>
#include <cstdio>

#include <tokotachi/core/application.hpp>
#include <tokotachi/graphics/canvas.hpp>

#include "../widgets/theme.hpp"
#include "../core/application_impl.hpp"
#include "../core/property_impl.hpp"
#include "window_impl.hpp"

namespace tokotachi
{

Popup::Popup(Widget* owner, PopupKind popup_kind, Object* owner_object)
    : Window(owner_object ? owner_object : owner),
      TOKOTACHI_READONLY_PROPERTY(Popup, Widget*, owner_widget),
      TOKOTACHI_READONLY_PROPERTY(Popup, PopupKind, kind),
      TOKOTACHI_HANDLER_PROPERTY(Popup, NotifyHandler, on_closed),
      owner_widget_(owner),
      kind_(popup_kind)
{
}

Popup::~Popup()
{
    if (Application* app = Application::instance())
    {
        std::vector<detail::WindowImpl*>& menus = app->impl()->open_menus;
        menus.erase(std::remove(menus.begin(), menus.end(), impl()), menus.end());
    }
}

void Popup::open(const Rect& anchor, const Size& size, PopupPlacement placement)
{
    Window* parent_window = owner_widget_ ? owner_widget_->window.get() : nullptr;
    if (!parent_window || !parent_window->impl()->window)
    {
        return;
    }
    detail::WindowImpl* pw = parent_window->impl();
    // 位置は親のウィンドウからの相対(window の座標。docs/adr/0008)。
    Point at(anchor.x, anchor.y);
    if (placement == PopupPlacement::BELOW)
    {
        at = Point(anchor.x, anchor.bottom());
    }
    else if (placement == PopupPlacement::RIGHT)
    {
        at = Point(anchor.right(), anchor.y);
    }
    at = owner_widget_->map_to_window(at);
    const float k = pw->scale / pw->density;
    const int x = static_cast<int>(std::floor(at.x * k + 0.5f));
    const int y = static_cast<int>(std::floor(at.y * k + 0.5f));
    const int w = static_cast<int>(std::ceil(size.width * k));
    const int h = static_cast<int>(std::ceil(size.height * k));
    if (!impl()->create_popup(pw->window, kind_ == PopupKind::TOOLTIP, x, y, w, h))
    {
        return;
    }
    if (detail::trace_enabled())
    {
        std::fprintf(stderr, "[tokotachi] popup open    %-10s parent=%s at=(%d, %d) size=(%d, %d) bounds=(%.1f, %.1f)\n",
                     detail::trace_name(impl()), detail::trace_name(pw), x, y, w, h, width.get(), height.get());
    }
    if (kind_ == PopupKind::MENU)
    {
        std::vector<detail::WindowImpl*>& menus = Application::instance()->impl()->open_menus;
        if (std::find(menus.begin(), menus.end(), impl()) == menus.end())
        {
            menus.push_back(impl());
        }
    }
    visible = true;
}

void Popup::close()
{
    if (Application* app = Application::instance())
    {
        // このポップアップから後に開いたもの(サブメニューなど)を先に閉じる。
        std::vector<detail::WindowImpl*>& menus = app->impl()->open_menus;
        std::vector<detail::WindowImpl*>::iterator self = std::find(menus.begin(), menus.end(), impl());
        if (self != menus.end())
        {
            while (menus.back() != impl())
            {
                menus.back()->q->close();
            }
            menus.pop_back();
        }
    }
    if (!visible)
    {
        return;
    }
    if (detail::trace_enabled())
    {
        std::fprintf(stderr, "[tokotachi] popup close   %s\n", detail::trace_name(impl()));
    }
    visible = false;
    // ハンドラの中でこのポップアップが破棄されてもよいよう、最後に呼ぶ。
    Object* self_object = this;
    detail::fire(on_closed_, self_object);
}

void Popup::paint(Canvas& canvas)
{
    Window::paint(canvas);
    if (kind_ == PopupKind::MENU)
    {
        canvas.stroke_round_rect(local_rect, 0, 1, theme::border());
    }
}

}  // namespace tokotachi
