/// @file
/// tokotachi::ScrollArea の宣言。
#ifndef TOKOTACHI_WIDGETS_SCROLL_AREA_HPP
#define TOKOTACHI_WIDGETS_SCROLL_AREA_HPP

#include "../core/export.hpp"
#include "scroll_bar.hpp"
#include "widget.hpp"

namespace tokotachi
{

/// 中身のウィジェットを、見えている範囲より大きくてもスクロールして見せる入れ物(docs/adr/0021)。
///
/// @image html widget_scroll_area.png
///
/// @code
/// tokotachi::ScrollArea* scroll_area_form = new tokotachi::ScrollArea(window_main);
/// tokotachi::Widget* form = new tokotachi::Widget();         // 中身(親は content への代入で決まる)
/// tokotachi::GridLayout* grid = new tokotachi::GridLayout(form);
/// ...
/// scroll_area_form->content = form;
/// @endcode
///
/// 中身の大きさは、中身の preferred_size(fit_width なら幅は見えている幅)。見えている範囲に収まらない向きにだけ
/// スクロールバーを出す。ScrollArea 自身にレイアウトを付けてはいけない(子の配置は ScrollArea が決める)。
///
/// @ingroup widgets_container
class TOKOTACHI_API ScrollArea : public Widget
{
public:
    explicit ScrollArea(Widget* parent = nullptr);
    ~ScrollArea() override;

    /// 中身。代入すると中身はスクロール領域の中に移る(親が付け替わる)。前の中身は破棄する。
    Property<Widget*> content;
    /// 中身の幅を見えている幅に合わせ、縦だけスクロールするか(既定 true)。
    Property<bool> fit_width;
    /// スクロールの位置(中身の左上が、見えている範囲の左上からずれている量。0 以上)。
    Property<Point> scroll_position;

    /// 中身の座標系の矩形 r が見えるようにスクロールする。
    void scroll_to_visible(const Rect& r);

protected:
    void arrange_children() override;
    void mouse_wheel(WheelEvent& e) override;
    Size compute_preferred_size() const override;
    Size compute_minimum_size() const override;

private:
    class Impl;
    Impl* d_;
};

}  // namespace tokotachi

#endif
