/// @file
/// tokotachi::GroupBox の宣言。
#ifndef TOKOTACHI_WIDGETS_GROUP_BOX_HPP
#define TOKOTACHI_WIDGETS_GROUP_BOX_HPP

#include <string>

#include "../graphics/text_layout.hpp"
#include "widget.hpp"

namespace tokotachi
{

/// 見出しの付いた枠(docs/adr/0049)。中の部品をまとめて見せる。子はレイアウトで並べる(枠と見出しの内側に並ぶ)。
///
/// @image html widget_group_box.png
///
/// @code
/// tokotachi::GroupBox* group_box_output = new tokotachi::GroupBox(&window, "出力(&O)");
/// tokotachi::BoxLayout* box_output = new tokotachi::BoxLayout(group_box_output);
/// box_output->add(new tokotachi::CheckBox(group_box_output, "上書きする"));
/// @endcode
///
/// title の「&」の次の文字はニーモニック。Alt と一緒に押すと、中の最初の部品へフォーカスを移す。
///
/// @ingroup widgets_container
class GroupBox : public Widget
{
public:
    explicit GroupBox(Widget* parent = nullptr, const std::string& title = std::string());

    /// 見出し(空なら見出しの無い枠)。
    Property<std::string> title;
    /// 中の部品を置く矩形(自分の座標系。枠・見出しと余白を除く)。
    ReadOnlyProperty<Rect> content_rect;

protected:
    void paint(Canvas& canvas) override;
    void scale_changed() override;
    void arrange_children() override;
    Size compute_preferred_size() const override;
    Size compute_minimum_size() const override;
    Key mnemonic_key() const override;
    void mnemonic_activated() override;

private:
    void set_title(const std::string& v);
    Rect get_content_rect() const;
    const TextLayout& title_layout() const;
    /// 枠・見出しと余白の大きさ(内側の部品の大きさに足す)。
    Size extra() const;

    std::string title_;
    std::string display_;          ///< title_ から & を除いた、表示する文字
    std::size_t mnemonic_offset_;  ///< display_ の中のニーモニックの文字の位置(無ければ npos)
    Key mnemonic_;
    mutable TextLayout layout_;
};

}  // namespace tokotachi

#endif
