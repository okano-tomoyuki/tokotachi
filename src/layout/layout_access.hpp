// Widget・Window の実装から Layout の保護された部分を扱うための窓口(内部)。
// Layout の実装(Layout::Impl)もここで定める。
#ifndef TOKOTACHI_SRC_LAYOUT_LAYOUT_ACCESS_HPP
#define TOKOTACHI_SRC_LAYOUT_LAYOUT_ACCESS_HPP

#include <tokotachi/layout/layout.hpp>

#include "../core/impl.hpp"

namespace tokotachi
{

class Layout::Impl
{
    TOKOTACHI_IMPL(Layout);

    Impl(Layout* layout, Widget* target) : q(layout), target_(target) {}

    Widget* get_widget() const { return target_; }

    void set_margins(const Margins& v)
    {
        margins_ = v;
        q->request_layout();
    }

    Widget* target_;
    Margins margins_;
};

namespace detail
{

template <>
struct ImplAccess<Layout>
{
    TOKOTACHI_IMPL_ACCESS(Layout)

    // 余白を含めた大きさ。
    static Size preferred_size(const Layout* l)
    {
        const Size s = l->compute_preferred_size();
        const Margins& m = of(l)->margins_;
        return Size(s.width + m.left + m.right, s.height + m.top + m.bottom);
    }

    static Size minimum_size(const Layout* l)
    {
        const Size s = l->compute_minimum_size();
        const Margins& m = of(l)->margins_;
        return Size(s.width + m.left + m.right, s.height + m.top + m.bottom);
    }

    // rect(ウィジェットの座標系)から余白を除いた中に配置する。
    static void arrange(Layout* l, const Rect& rect)
    {
        const Margins& m = of(l)->margins_;
        l->arrange(rect.adjusted(m.left, m.top, m.right, m.bottom));
    }

    static void widget_removed(Layout* l, Widget* w) { l->widget_removed(w); }
};

typedef ImplAccess<Layout> LayoutAccess;

}  // namespace detail
}  // namespace tokotachi

#endif
