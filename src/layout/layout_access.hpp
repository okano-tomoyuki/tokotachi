// Widget・Window の実装から Layout の保護された部分を扱うための窓口(内部)。
#ifndef GUI_SRC_LAYOUT_LAYOUT_ACCESS_HPP
#define GUI_SRC_LAYOUT_LAYOUT_ACCESS_HPP

#include <gui/layout.hpp>

namespace gui
{
namespace detail
{

struct LayoutAccess
{
    // 余白を含めた大きさ。
    static Size preferred_size(const Layout* l)
    {
        const Size s = l->compute_preferred_size();
        return Size(s.width + l->margins_.left + l->margins_.right, s.height + l->margins_.top + l->margins_.bottom);
    }

    static Size minimum_size(const Layout* l)
    {
        const Size s = l->compute_minimum_size();
        return Size(s.width + l->margins_.left + l->margins_.right, s.height + l->margins_.top + l->margins_.bottom);
    }

    // rect(ウィジェットの座標系)から余白を除いた中に配置する。
    static void arrange(Layout* l, const Rect& rect)
    {
        const Margins& m = l->margins_;
        l->arrange(rect.adjusted(m.left, m.top, m.right, m.bottom));
    }

    static void widget_removed(Layout* l, Widget* w) { l->widget_removed(w); }
};

}  // namespace detail
}  // namespace gui

#endif
