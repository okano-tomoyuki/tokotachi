/// @file
/// tokotachi::GridLayout の宣言。
#ifndef TOKOTACHI_LAYOUT_GRID_LAYOUT_HPP
#define TOKOTACHI_LAYOUT_GRID_LAYOUT_HPP

#include "../core/export.hpp"
#include "layout.hpp"

namespace tokotachi
{

/// 子を行と列の格子に置くレイアウト(cpp_tk の grid に当たる。docs/adr/0017)。
///
/// @code
/// tokotachi::GridLayout* form = new tokotachi::GridLayout(window_main);
/// form->add(label_name, 0, 0);
/// form->add(line_edit_name, 0, 1);
/// form->add(row_buttons, 1, 0, 1, 2);      // 2 列にまたがる
/// form->set_column_stretch(1, 1);      // 余った幅は 1 列目が受け取る
/// @endcode
///
/// 列の幅(行の高さ)は、その列の項目の望む大きさの最大。複数の列にまたがる項目は、足りない分を
/// またがる列に分ける。余った長さは伸びる割合(stretch)に応じて列に分け、どの列も伸びなければ余りは右(下)に残る。
/// 足りないときは、各列を最小の大きさまで縮める。セルの中では、項目ごとのそろえ方(cpp_tk の sticky に当たる)に従う。
/// 項目が無く最小の大きさも決めていない行・列は、大きさ 0 で、間隔も入れない。
///
/// @ingroup layout
class TOKOTACHI_API GridLayout : public Layout
{
public:
    /// target のレイアウトになる(target が所有する)。
    explicit GridLayout(Widget* target);
    ~GridLayout() override;

    /// 行の間隔・列の間隔(dp、既定 8)。
    Property<float> row_spacing;
    /// 列の間隔(dp、既定 8)。
    Property<float> column_spacing;

    /// w(target の子)を置く。すでに置いていれば位置と設定を変える。
    void add(Widget* w, int row, int column, int row_span = 1, int column_span = 1,
             Align horizontal = Align::FILL, Align vertical = Align::CENTER);

    /// 行の伸びる割合(既定 0。余った高さを、割合に応じて分ける)。
    void set_row_stretch(int row, int stretch);
    /// 列の伸びる割合(既定 0。余った幅を、割合に応じて分ける)。
    void set_column_stretch(int column, int stretch);
    /// 行・列の最小の大きさ(dp)。
    void set_row_minimum(int row, float size);
    /// 列の最小の幅(dp)。
    void set_column_minimum(int column, float size);

protected:
    Size compute_preferred_size() const override;
    Size compute_minimum_size() const override;
    void arrange(const Rect& area) override;
    void widget_removed(Widget* w) override;

private:
    class Impl;
    Impl* d_;
};

}  // namespace tokotachi

#endif
