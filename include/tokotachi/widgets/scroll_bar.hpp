// スクロールバー(docs/adr/0021)。value は 0 から maximum まで。
//
//   tokotachi::ScrollBar* bar = new tokotachi::ScrollBar(panel, tokotachi::Orientation::VERTICAL);
//   bar->maximum = content_height - view_height;  // 動かせる量
//   bar->page_step = view_height;                  // 見えている長さ(つまみの大きさになる)
//   bar->on_change = [](tokotachi::Object* sender) { ... };
//
// つまみのドラッグ、溝のクリック(1 ページ分)、ホイール(single_step × 目盛り)で動かす。
#ifndef TOKOTACHI_WIDGETS_SCROLL_BAR_HPP
#define TOKOTACHI_WIDGETS_SCROLL_BAR_HPP

#include "../layout/alignment.hpp"
#include "widget.hpp"

namespace tokotachi
{

class ScrollBar : public Widget
{
public:
    explicit ScrollBar(Widget* parent = nullptr, Orientation orientation = Orientation::VERTICAL);

    Property<Orientation> orientation;
    // 位置(0 から maximum。範囲の外の値は丸める)。
    Property<float> value;
    // 動かせる量(中身の長さ - 見えている長さ。0 なら動かせない)。小さくすると value も丸める。
    Property<float> maximum;
    // 見えている長さ(つまみの大きさと、溝のクリックで動く量)。
    Property<float> page_step;
    // ホイールの 1 目盛りで動く量(dp、既定 48)。
    Property<float> single_step;

    // value が変わった(利用者の操作・代入のどちらでも)。
    Property<NotifyHandler> on_change;

    // スクロールバーの太さ(dp)。
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
    Orientation get_orientation() const { return orientation_; }
    void set_orientation(const Orientation& v);
    float get_value() const { return value_; }
    void set_value(const float& v);
    float get_maximum() const { return maximum_; }
    void set_maximum(const float& v);
    float get_page_step() const { return page_step_; }
    void set_page_step(const float& v);
    float get_single_step() const { return single_step_; }
    void set_single_step(const float& v);

    // 溝の長さ・つまみの位置と長さ(並ぶ向き、dp)。
    float track_length() const;
    void thumb(float& pos, float& len) const;
    // 並ぶ向きの座標。
    float along(const Point& p) const;

    Orientation orientation_;
    float value_;
    float maximum_;
    float page_step_;
    float single_step_;
    NotifyHandler on_change_;
    bool hovered_;
    bool dragging_;
    float drag_offset_;  // つまみの始めから、押した位置まで
};

}  // namespace tokotachi

#endif
