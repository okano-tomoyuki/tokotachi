/// @file
/// tokotachi::Splitter の宣言。
#ifndef TOKOTACHI_WIDGETS_SPLITTER_HPP
#define TOKOTACHI_WIDGETS_SPLITTER_HPP

#include <vector>

#include "../core/export.hpp"
#include "../layout/alignment.hpp"
#include "widget.hpp"

namespace tokotachi
{

class Splitter;

/// 1 つの区画。Splitter が作り、所有する。
/// @ingroup widgets_container
class TOKOTACHI_API SplitterPane : public Object
{
public:
    /// Splitter の大きさが変わったときの伸び縮みの比率(0 なら大きさを保つ。すべて 0 なら最後の区画が受け持つ)。
    Property<int> stretch;
    /// たためるか(既定 true)。
    Property<bool> collapsible;
    /// たたんでいるか。代入するとたたむ・戻す(collapsible でなければたためない)。その分は隣の区画が受け持つ。
    Property<bool> collapsed;
    ReadOnlyProperty<Widget*> widget;

private:
    friend class Splitter;

    SplitterPane(Splitter* splitter, Widget* widget, int stretch);
    ~SplitterPane();

    void set_stretch(const int& v);
    void set_collapsible(const bool& v);
    bool get_collapsed() const { return collapsed_; }
    void set_collapsed(const bool& v);
    Widget* get_widget() const { return widget_; }

    Splitter* splitter_;
    Widget* widget_;
    int stretch_;
    bool collapsible_;
    float size_;      ///< 並ぶ向きの大きさ(dp。たたんでいる間は使わない)。負ならまだ決めていない
    bool collapsed_;  ///< たたんでいる(大きさ 0 として扱い、部品を隠す)
};

/// 分割ウィンドウ: 区画を横(または縦)に並べ、間の境目をドラッグして大きさを変える(docs/adr/0031)。
///
/// @image html widget_splitter.png
///
/// @code
/// tokotachi::Splitter* splitter_main = new tokotachi::Splitter(&window);              // 既定は横に並べる
/// splitter_main->add_widget(tree_view_folders, 0);                                // 0: ウィンドウの大きさが変わっても幅を保つ
/// splitter_main->add_widget(list_view_files, 1);                                // 1: 増えた・減った幅を受け持つ
/// splitter_main->sizes = std::vector<float>{200, 400};                    // 区画の大きさ(保存・復元に使う)
/// @endcode
///
/// 区画を破棄する・別の親へ移すと、その区画は消える。区画の最小の大きさ(minimum_hint)より小さくはドラッグできず、
/// collapsible な区画は、最小の半分より小さくドラッグするとたたむ(大きさ 0。隠す)。たたんだ区画は境目をドラッグして戻す。
/// 境目をダブルクリックすると、隣のたたんだ区画を戻す(どちらもたたんでいなければ、stretch の小さい方をたたむ)。
/// 境目を押した後は、矢印キーで境目を動かし、Enter でダブルクリックと同じことをする(docs/adr/0049)。
/// 区画の部品を visible = false にすると、その区画は場所も境目も取らない(大きさは覚えていて、戻すと元の大きさで出る)。
/// たたんだ区画の部品の visible は変えない(showing が false になる)。
///
/// @ingroup widgets_container
class TOKOTACHI_API Splitter : public Widget
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

    /// 各区画の大きさ(dp。並ぶ向き。たたんだ区画は 0、部品を隠した区画は戻したときの大きさ)。代入すると、合計が合わない分は stretch の比率で合わせる。
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
    void key_down(KeyEvent& e) override;
    void focus_out() override;
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
    /// 部品を隠していない区画(並べる・境目を出すもの)。境目の番号はこの並びの番号。
    std::vector<SplitterPane*> shown_panes() const;
    /// 区画の大きさの合計を available に合わせる(初めてなら望む大きさから決める)。
    void fit(float available);
    /// 位置 p にある境目(i 番目の区画の後ろ。無ければ -1)。
    int handle_at(const Point& p) const;
    float handle_position(int i) const;
    /// i 番目の境目を position(並ぶ向きの位置)へ動かす。利用者の操作なので on_change を呼ぶ。
    void drag_to(int i, float position);
    /// i 番目の境目のダブルクリック(隣のたたんだ区画を戻す・どちらかをたたむ)。
    void toggle_handle(int i);
    /// 区画をたたむ・戻す(その分は隣の区画が受け持つ)。変えたら true。
    bool collapse_pane(SplitterPane* pane, bool collapse);

    Orientation orientation_;
    std::vector<SplitterPane*> panes_;
    NotifyHandler on_change_;
    int hover_handle_;
    int drag_handle_;
    float drag_offset_;  ///< 押した位置と境目の始まりの差
    int key_handle_;     ///< キーで動かす境目(境目を押してフォーカスを得たとき。無ければ -1)
    bool destroying_;
};

}  // namespace tokotachi

#endif
