/// @file
/// レイアウトの基底(docs/adr/0017)。
///
/// レイアウトは親のウィジェットに 1 つだけ付き、子のウィジェットの bounds を決める(子は従う)。
/// @code
/// tokotachi::BoxLayout* box = new tokotachi::BoxLayout(panel);  // panel に付く(panel が所有する)
/// box->add(button);
/// @endcode
/// 2 つ目のレイアウトを作ると、古いレイアウトは破棄される。レイアウトを delete すると外れ、自由配置(bounds を利用者が決める)に戻る。
/// レイアウトが管理する子の bounds に代入しても、次の配置で上書きされる。
/// add に渡したウィジェットが付いているウィジェットの子でなければ、子にする(親を付け替える。docs/adr/0018)。
///
/// 配置は 2 段階: 子の望む大きさ(preferred_size)と最小の大きさ(minimum_hint)を集めて自分の大きさを決め(下から上)、
/// 実際の大きさを受け取って子の bounds を決める(上から下)。配置のやり直しは印を付けるだけにし、描画の直前にまとめて行う。
#ifndef TOKOTACHI_LAYOUT_LAYOUT_HPP
#define TOKOTACHI_LAYOUT_LAYOUT_HPP

#include <vector>

#include "alignment.hpp"
#include "../core/geometry.hpp"
#include "../core/property.hpp"

namespace tokotachi
{

class Widget;

namespace detail
{
struct LayoutAccess;
}

/// レイアウトの内側の余白(dp)。
struct Margins
{
    float left, top, right, bottom;

    Margins() : left(0), top(0), right(0), bottom(0) {}
    explicit Margins(float all) : left(all), top(all), right(all), bottom(all) {}
    Margins(float l, float t, float r, float b) : left(l), top(t), right(r), bottom(b) {}

    bool operator==(const Margins& o) const
    {
        return left == o.left && top == o.top && right == o.right && bottom == o.bottom;
    }
    bool operator!=(const Margins& o) const { return !(*this == o); }
};

class Layout : public Object
{
public:
    /// target のレイアウトになる(target が所有する)。target がすでにレイアウトを持っていれば、それは破棄される。
    explicit Layout(Widget* target);
    ~Layout();

    /// このレイアウトが付いているウィジェット(外れていれば nullptr)。
    ReadOnlyProperty<Widget*> widget;
    Property<Margins> margins;

    /// w をこのレイアウトから外す(w は破棄しない。bounds はそのまま残る)。
    void remove(Widget* w);

protected:
    /// 子の望む大きさから、余白を含めた自分の望む大きさ・最小の大きさを決める。
    virtual Size compute_preferred_size() const = 0;
    virtual Size compute_minimum_size() const = 0;
    /// area(余白を除いた、ウィジェットの座標系の矩形)の中に子を配置する。
    virtual void arrange(const Rect& area) = 0;
    /// w が管理から外れた(remove・破棄)。派生クラスは w の項目を消す。
    virtual void widget_removed(Widget* w) = 0;

    /// 配置のやり直しを依頼する(項目や設定を変えたとき)。
    void request_layout();
    /// w をこのレイアウトで管理できるようにする。target の子でなければ子にする(付け替える)。
    /// 管理できない(nullptr・ウィンドウ・target 自身や祖先)なら false。
    bool adopt(Widget* w);

    /// 配置に使う子の大きさ(隠れている子は 0)。
    static Size preferred_of(const Widget* w);
    static Size minimum_of(const Widget* w);
    static Size maximum_of(const Widget* w);
    static bool is_managed_visible(const Widget* w);
    /// cell の中に、そろえ方に従って w を置く。辺はピクセルの整数に揃える(docs/adr/0008)。
    void place_in_cell(Widget* w, const Rect& cell, Align horizontal, Align vertical) const;

private:
    friend struct detail::LayoutAccess;

    Widget* get_widget() const { return target_; }
    void set_margins(const Margins& v);

    Widget* target_;
    Margins margins_;
};

}  // namespace tokotachi

#endif
