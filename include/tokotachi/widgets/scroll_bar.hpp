/// @file
/// tokotachi::ScrollBar の宣言。
#ifndef TOKOTACHI_WIDGETS_SCROLL_BAR_HPP
#define TOKOTACHI_WIDGETS_SCROLL_BAR_HPP

#include "../core/export.hpp"
#include "../layout/alignment.hpp"
#include "range_base.hpp"

namespace tokotachi
{

/// スクロールバー(docs/adr/0021)。値・範囲・刻みは RangeBase のもの(docs/adr/0058)。
///
/// value は minimum(既定 0)から maximum まで。maximum は動かせる量(中身の長さ - 見えている長さ。既定 0 で、動かせない)、
/// page_step は見えている長さ(つまみの大きさと、溝のクリックで動く量)、single_step はホイールの 1 目盛りで動く量(dp、既定 48)。
///
/// @code
/// tokotachi::ScrollBar* scroll_bar = new tokotachi::ScrollBar(panel, tokotachi::Orientation::VERTICAL);
/// scroll_bar->maximum = content_height - view_height;  // 動かせる量
/// scroll_bar->page_step = view_height;                  // 見えている長さ(つまみの大きさになる)
/// scroll_bar->on_change = [](tokotachi::Object* sender) { ... };
/// @endcode
///
/// つまみのドラッグ、溝のクリック(1 ページ分)、ホイール(single_step × 目盛り)で動かす。
///
/// @ingroup widgets_base
class TOKOTACHI_API ScrollBar : public RangeBase
{
public:
    /// parent の子として作る(parent が所有する)。
    explicit ScrollBar(Widget* parent = nullptr, Orientation orientation = Orientation::VERTICAL);
    ~ScrollBar() override;

    /// 向き。
    Property<Orientation> orientation;

    /// スクロールバーの太さ(dp)。
    static const float THICKNESS;

protected:
    void paint(Canvas& canvas) override;
    void mouse_down(MouseEvent& e) override;
    void mouse_move(MouseEvent& e) override;
    void mouse_up(MouseEvent& e) override;
    void mouse_enter() override;
    void mouse_leave() override;
    void mouse_wheel(WheelEvent& e) override;
    Size compute_preferred_size() const override;
    Size compute_minimum_size() const override;

private:
    class Impl;
    Impl* d_;
};

}  // namespace tokotachi

#endif
