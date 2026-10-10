/// @file
/// 中身のウィジェットを、見えている範囲より大きくてもスクロールして見せる入れ物(docs/adr/0021)。
///
/// @code
/// tokotachi::ScrollArea* scroll = new tokotachi::ScrollArea(&window);
/// tokotachi::Widget* form = new tokotachi::Widget();         // 中身(親は content への代入で決まる)
/// tokotachi::GridLayout* grid = new tokotachi::GridLayout(form);
/// ...
/// scroll->content = form;
/// @endcode
///
/// 中身の大きさは、中身の preferred_size(fit_width なら幅は見えている幅)。見えている範囲に収まらない向きにだけ
/// スクロールバーを出す。ScrollArea 自身にレイアウトを付けてはいけない(子の配置は ScrollArea が決める)。
#ifndef TOKOTACHI_WIDGETS_SCROLL_AREA_HPP
#define TOKOTACHI_WIDGETS_SCROLL_AREA_HPP

#include "scroll_bar.hpp"
#include "widget.hpp"

namespace tokotachi
{

class ScrollArea : public Widget
{
public:
    explicit ScrollArea(Widget* parent = nullptr);

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
    Widget* get_content() const;
    void set_content(Widget* const& v);
    bool get_fit_width() const { return fit_width_; }
    void set_fit_width(const bool& v);
    Point get_scroll_position() const;
    void set_scroll_position(const Point& p);
    /// スクロールバーの値に合わせて中身を動かす。
    void place_content();

    Widget* viewport_;
    ScrollBar* vertical_;
    ScrollBar* horizontal_;
    Widget* content_;
    bool fit_width_;
};

}  // namespace tokotachi

#endif
