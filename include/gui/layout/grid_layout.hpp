// 子を行と列の格子に置くレイアウト(cpp_tk の grid に当たる。docs/adr/0017)。
//
//   gui::GridLayout* form = new gui::GridLayout(&window);
//   form->add(name_label, 0, 0);
//   form->add(name_edit, 0, 1);
//   form->add(buttons, 1, 0, 1, 2);      // 2 列にまたがる
//   form->set_column_stretch(1, 1);      // 余った幅は 1 列目が受け取る
//
// 列の幅(行の高さ)は、その列の項目の望む大きさの最大。複数の列にまたがる項目は、足りない分を
// またがる列に分ける。余った長さは伸びる割合(stretch)に応じて列に分け、どの列も伸びなければ余りは右(下)に残る。
// 足りないときは、各列を最小の大きさまで縮める。セルの中では、項目ごとのそろえ方(cpp_tk の sticky に当たる)に従う。
// 項目が無く最小の大きさも決めていない行・列は、大きさ 0 で、間隔も入れない。
#ifndef GUI_GRID_LAYOUT_HPP
#define GUI_GRID_LAYOUT_HPP

#include "layout.hpp"

namespace gui
{

class GridLayout : public Layout
{
public:
    explicit GridLayout(Widget* target);

    // 行の間隔・列の間隔(dp、既定 8)。
    Property<float> row_spacing;
    Property<float> column_spacing;

    // w(target の子)を置く。すでに置いていれば位置と設定を変える。
    void add(Widget* w, int row, int column, int row_span = 1, int column_span = 1,
             Align horizontal = Align::FILL, Align vertical = Align::CENTER);

    void set_row_stretch(int row, int stretch);
    void set_column_stretch(int column, int stretch);
    // 行・列の最小の大きさ(dp)。
    void set_row_minimum(int row, float size);
    void set_column_minimum(int column, float size);

protected:
    Size compute_preferred_size() const override;
    Size compute_minimum_size() const override;
    void arrange(const Rect& area) override;
    void widget_removed(Widget* w) override;

private:
    struct Item
    {
        Widget* widget;
        int row, column, row_span, column_span;
        Align horizontal, vertical;
    };

    // 1 つの向き(行または列)の設定。
    struct Track
    {
        int stretch;
        float minimum;
    };

    float get_row_spacing() const { return row_spacing_; }
    void set_row_spacing(const float& v);
    float get_column_spacing() const { return column_spacing_; }
    void set_column_spacing(const float& v);
    Track& track(std::vector<Track>& tracks, int index);
    // 1 つの向きの、各行(列)の望む大きさ・最小の大きさと、使われているか(項目か最小の大きさがあるか)を求める。
    void measure(bool columns, std::vector<float>& preferred, std::vector<float>& minimum, std::vector<bool>& used) const;
    int count(bool columns) const;

    float row_spacing_;
    float column_spacing_;
    std::vector<Item> items_;
    std::vector<Track> rows_;
    std::vector<Track> columns_;
};

}  // namespace gui

#endif
