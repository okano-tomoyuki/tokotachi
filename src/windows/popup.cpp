#include <tokotachi/windows/popup.hpp>

#include <algorithm>
#include <cmath>
#include <cstdio>

#include <tokotachi/core/application.hpp>
#include <tokotachi/graphics/canvas.hpp>

#include "../widgets/theme.hpp"
#include "../core/application_impl.hpp"
#include "../core/impl.hpp"
#include "window_impl.hpp"

namespace tokotachi
{

class Popup::Impl
{
    TOKOTACHI_IMPL(Popup);

    explicit Impl(Popup* object) : q(object) {}

    Widget* get_owner_widget() const { return owner_widget_; }
    PopupKind get_kind() const { return kind_; }

    Widget* owner_widget_;
    PopupKind kind_;
    NotifyHandler on_closed_;
};

Popup::Popup(Widget* owner, PopupKind popup_kind, Object* owner_object)
    : Window(owner_object ? owner_object : owner),
      TOKOTACHI_IMPL_READONLY_PROPERTY(Impl, Widget*, owner_widget),
      TOKOTACHI_IMPL_READONLY_PROPERTY(Impl, PopupKind, kind),
      TOKOTACHI_IMPL_HANDLER_PROPERTY(Impl, NotifyHandler, on_closed),
      d_(new Impl(this))
{
    d_->owner_widget_ = owner;
    d_->kind_ = popup_kind;
}

Popup::~Popup()
{
    if (Application* app = Application::instance())
    {
        std::vector<detail::WindowImpl*>& menus = detail::impl_of(app)->open_menus;
        menus.erase(std::remove(menus.begin(), menus.end(), detail::window_impl_of(this)), menus.end());
    }
    delete d_;
}

void Popup::open(const Rect& anchor, const Size& size, PopupPlacement placement)
{
    Window* parent_window = d_->owner_widget_ ? d_->owner_widget_->window.get() : nullptr;
    if (!parent_window || !detail::window_impl_of(parent_window)->window)
    {
        return;
    }
    detail::WindowImpl* pw = detail::window_impl_of(parent_window);
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
    at = d_->owner_widget_->map_to_window(at);
    const float k = pw->scale / pw->density;
    const int x = static_cast<int>(std::floor(at.x * k + 0.5f));
    const int y = static_cast<int>(std::floor(at.y * k + 0.5f));
    const int w = static_cast<int>(std::ceil(size.width * k));
    const int h = static_cast<int>(std::ceil(size.height * k));
    if (!detail::window_impl_of(this)->create_popup(pw->window, d_->kind_ == PopupKind::TOOLTIP, x, y, w, h))
    {
        return;
    }
    if (detail::trace_enabled())
    {
        std::fprintf(stderr, "[tokotachi] popup open    %-10s parent=%s at=(%d, %d) size=(%d, %d) bounds=(%.1f, %.1f)\n",
                     detail::trace_name(detail::window_impl_of(this)), detail::trace_name(pw), x, y, w, h, width.get(), height.get());
    }
    if (d_->kind_ == PopupKind::MENU)
    {
        std::vector<detail::WindowImpl*>& menus = detail::impl_of(Application::instance())->open_menus;
        if (std::find(menus.begin(), menus.end(), detail::window_impl_of(this)) == menus.end())
        {
            menus.push_back(detail::window_impl_of(this));
        }
    }
    visible = true;
}

void Popup::close()
{
    if (Application* app = Application::instance())
    {
        // このポップアップから後に開いたもの(サブメニューなど)を先に閉じる。
        std::vector<detail::WindowImpl*>& menus = detail::impl_of(app)->open_menus;
        std::vector<detail::WindowImpl*>::iterator self = std::find(menus.begin(), menus.end(), detail::window_impl_of(this));
        if (self != menus.end())
        {
            while (menus.back() != detail::window_impl_of(this))
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
        std::fprintf(stderr, "[tokotachi] popup close   %s\n", detail::trace_name(detail::window_impl_of(this)));
    }
    visible = false;
    // ハンドラの中でこのポップアップが破棄されてもよいよう、最後に呼ぶ。
    Object* self_object = this;
    detail::fire(d_->on_closed_, self_object);
}

void Popup::paint(Canvas& canvas)
{
    Window::paint(canvas);
    if (d_->kind_ == PopupKind::MENU)
    {
        canvas.stroke_round_rect(local_rect, 0, 1, theme::border());
    }
}

}  // namespace tokotachi
