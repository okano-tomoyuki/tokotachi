/// @file
/// 子を横または縦に並べるレイアウト(cpp_tk の pack に当たる。docs/adr/0017)。
///
/// @code
/// tokotachi::BoxLayout* row = new tokotachi::BoxLayout(panel, tokotachi::Orientation::HORIZONTAL);
/// row->add(line_edit_search, 1);       // 伸びる割合 1(余った幅を受け取る)
/// row->add(button_search);        // 望む大きさのまま
/// row->add_stretch();      // 空きを入れる
/// @endcode
///
/// 余った長さは、伸びる割合(stretch)に応じて項目に分ける。どの項目も伸びなければ、余りは末尾に残る。
/// 足りないときは、各項目を最小の大きさまで、望む大きさとの差に応じて縮める。
/// 並べる向きと直交する向きは、項目ごとのそろえ方(既定は FILL)に従う。
#ifndef TOKOTACHI_LAYOUT_BOX_LAYOUT_HPP
#define TOKOTACHI_LAYOUT_BOX_LAYOUT_HPP

#include "layout.hpp"

namespace tokotachi
{

class BoxLayout : public Layout
{
public:
    explicit BoxLayout(Widget* target, Orientation orientation = Orientation::VERTICAL);

    Property<Orientation> orientation;
    /// 項目の間隔(dp、既定 8)。
    Property<float> spacing;

    /// w(target の子)を末尾に加える。すでに加えていれば設定を変える。
    void add(Widget* w, int stretch = 0, Align cross_alignment = Align::FILL);
    /// 固定の空き(dp)を末尾に加える。
    void add_spacing(float size);
    /// 伸びる空きを末尾に加える。
    void add_stretch(int stretch = 1);

    void set_stretch(Widget* w, int stretch);
    void set_alignment(Widget* w, Align cross_alignment);

protected:
    Size compute_preferred_size() const override;
    Size compute_minimum_size() const override;
    void arrange(const Rect& area) override;
    void widget_removed(Widget* w) override;

private:
    struct Item
    {
        Widget* widget;  ///< nullptr なら空き
        float space;     ///< 固定の空き(widget が nullptr のとき)
        int stretch;
        Align align;
    };

    Orientation get_orientation() const { return orientation_; }
    void set_orientation(const Orientation& v);
    float get_spacing() const { return spacing_; }
    void set_spacing(const float& v);
    Item* find(Widget* w);
    /// 並べる向きの長さ・直交する向きの長さを、大きさから取り出す。
    float main_of(const Size& s) const;
    float cross_of(const Size& s) const;
    Size total(bool minimum) const;

    Orientation orientation_;
    float spacing_;
    std::vector<Item> items_;
};

}  // namespace tokotachi

#endif
