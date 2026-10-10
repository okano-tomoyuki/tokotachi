/// @file
/// 分割ウィンドウ: 区画を横(または縦)に並べ、間の境目をドラッグして大きさを変える(docs/adr/0031)。
///
/// @code
/// tokotachi::Splitter* split = new tokotachi::Splitter(&window);              // 既定は横に並べる
/// split->add_widget(tree_view, 0);                                // 0: ウィンドウの大きさが変わっても幅を保つ
/// split->add_widget(list_view, 1);                                // 1: 増えた・減った幅を受け持つ
/// split->sizes = std::vector<float>{200, 400};                    // 区画の大きさ(保存・復元に使う)
/// @endcode
///
/// 区画を破棄する・別の親へ移すと、その区画は消える。区画の最小の大きさ(minimum_hint)より小さくはドラッグできず、
/// collapsible な区画は、最小の半分より小さくドラッグするとたたむ(大きさ 0。隠す)。たたんだ区画は境目をドラッグして戻す。
#ifndef TOKOTACHI_WIDGETS_SPLITTER_HPP
#define TOKOTACHI_WIDGETS_SPLITTER_HPP

#include <vector>

#include "../layout/alignment.hpp"
#include "widget.hpp"

namespace tokotachi
{

class Splitter;

/// 1 つの区画。Splitter が作り、所有する。
class SplitterPane : public Object
{
public:
    /// Splitter の大きさが変わったときの伸び縮みの比率(0 なら大きさを保つ。すべて 0 なら最後の区画が受け持つ)。
    Property<int> stretch;
    /// たためるか(既定 true)。
    Property<bool> collapsible;
    /// たたんでいるか。
    ReadOnlyProperty<bool> collapsed;
    ReadOnlyProperty<Widget*> widget;

private:
    friend class Splitter;

    SplitterPane(Splitter* splitter, Widget* widget, int stretch);
    ~SplitterPane();

    void set_stretch(const int& v);
    void set_collapsible(const bool& v);
    bool get_collapsed() const { return collapsed_; }
    Widget* get_widget() const { return widget_; }

    Splitter* splitter_;
    Widget* widget_;
    int stretch_;
    bool collapsible_;
    float size_;      ///< 並ぶ向きの大きさ(dp。たたんでいる間は使わない)。負ならまだ決めていない
    bool collapsed_;  ///< たたんでいる(大きさ 0 として扱い、部品を隠す)
};

class Splitter : public Widget
{
public:
    explicit Splitter(Widget* parent = nullptr, Orientation orientation = Orientation::HORIZONTAL);
    ~Splitter();

    Property<Orientation> orientation;

    /// 区画を最後に加える(widget は Splitter の子になる)。
    SplitterPane* add_widget(Widget* widget, int stretch = 1);
    /// index の位置に加える(範囲の外なら最後)。
    SplitterPane* insert_widget(int index, Widget* widget, int stretch = 1);
    ReadOnlyProperty<std::vector<SplitterPane*> > panes;

    /// 各区画の大きさ(dp。並ぶ向き。たたんだ区画は 0)。代入すると、合計が合わない分は stretch の比率で合わせる。
    Property<std::vector<float> > sizes;

    /// 利用者が境目をドラッグして、大きさを変えた。
    Property<NotifyHandler> on_change;

    /// 境目の太さ(dp)。
    static const float HANDLE;

protected:
    void paint(Canvas& canvas) override;
    void mouse_down(MouseEvent& e) override;
    void mouse_move(MouseEvent& e) override;
    void mouse_up(MouseEvent& e) override;
    void mouse_leave() override;
    void arrange_children() override;
    void child_removed(Widget* child) override;
    Size compute_preferred_size() const override;
    Size compute_minimum_size() const override;

private:
    friend class SplitterPane;

    Orientation get_orientation() const { return orientation_; }
    void set_orientation(const Orientation& v);
    std::vector<SplitterPane*> get_panes() const { return panes_; }
    std::vector<float> get_sizes() const;
    void set_sizes(const std::vector<float>& v);

    /// 並ぶ向きの長さ・座標。
    float along(const Size& s) const { return orientation_ == Orientation::HORIZONTAL ? s.width : s.height; }
    float along(const Point& p) const { return orientation_ == Orientation::HORIZONTAL ? p.x : p.y; }
    float minimum_of(const SplitterPane* pane) const;
    /// 区画の大きさの合計を available に合わせる(初めてなら望む大きさから決める)。
    void fit(float available);
    /// 位置 p にある境目(i 番目の区画の後ろ。無ければ -1)。
    int handle_at(const Point& p) const;
    float handle_position(int i) const;
    void drag_to(float position);

    Orientation orientation_;
    std::vector<SplitterPane*> panes_;
    NotifyHandler on_change_;
    int hover_handle_;
    int drag_handle_;
    float drag_offset_;  ///< 押した位置と境目の始まりの差
    bool destroying_;
};

}  // namespace tokotachi

#endif
