/// @file
/// tokotachi::ScrollBar の宣言。
#ifndef TOKOTACHI_WIDGETS_SCROLL_BAR_HPP
#define TOKOTACHI_WIDGETS_SCROLL_BAR_HPP

#include "../core/export.hpp"
#include "../layout/alignment.hpp"
#include "widget.hpp"

namespace tokotachi
{

/// スクロールバー(docs/adr/0021)。value は 0 から maximum まで。
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
class TOKOTACHI_API ScrollBar : public Widget
{
public:
    explicit ScrollBar(Widget* parent = nullptr, Orientation orientation = Orientation::VERTICAL);
    ~ScrollBar() override;

    Property<Orientation> orientation;
    /// 位置(0 から maximum。範囲の外の値は丸める)。
    Property<float> value;
    /// 動かせる量(中身の長さ - 見えている長さ。0 なら動かせない)。小さくすると value も丸める。
    Property<float> maximum;
    /// 見えている長さ(つまみの大きさと、溝のクリックで動く量)。
    Property<float> page_step;
    /// ホイールの 1 目盛りで動く量(dp、既定 48)。
    Property<float> single_step;

    /// value が変わった(利用者の操作・代入のどちらでも)。
    Property<NotifyHandler> on_change;

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
