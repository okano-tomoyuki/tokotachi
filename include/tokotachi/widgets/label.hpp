/// @file
/// テキストを表示する。改行文字で行を分け、word_wrap なら幅で折り返す(docs/adr/0019)。
/// icon を与えると、文字の前(段落の向きの始め)に画像を出す(docs/adr/0048)。
#ifndef TOKOTACHI_WIDGETS_LABEL_HPP
#define TOKOTACHI_WIDGETS_LABEL_HPP

#include <string>

#include "../layout/alignment.hpp"
#include "../graphics/image.hpp"
#include "../graphics/text_layout.hpp"
#include "../graphics/theme.hpp"
#include "widget.hpp"

namespace tokotachi
{

class Label : public Widget
{
public:
    explicit Label(Widget* parent = nullptr, const std::string& text = std::string());

    Property<std::string> text;
    Property<Font> font;
    /// 文字の色(既定はテーマの TEXT。無効なときは TEXT_DISABLED で描く)。
    Property<ThemeColor> color;
    /// 文字の横のそろえ方(既定 START。FILL は START と同じ)。複数行なら各行をそろえる。
    Property<Align> alignment;
    /// 幅で折り返すか(既定 false)。折り返すと、望む高さは今の幅で折り返したときの高さになり、最小の幅は最も長い単語の幅になる。
    Property<bool> word_wrap;
    /// 文字の前に出す画像(既定は空で、出さない)。
    Property<Image> icon;
    /// icon を描く大きさ(論理座標の幅・高さ。既定 16)。
    Property<float> icon_size;

protected:
    void paint(Canvas& canvas) override;
    void scale_changed() override;
    Size compute_preferred_size() const override;
    /// 文字が切れないよう、望む大きさを最小とする。
    Size compute_minimum_size() const override;
    void resized() override;

private:
    void set_text(const std::string& v);
    void set_font(const Font& v);
    void set_color(const ThemeColor& v);
    void set_alignment(const Align& v);
    void set_word_wrap(const bool& v);
    void set_icon(const Image& v);
    void set_icon_size(const float& v);
    const TextLayout& layout() const;
    /// 画像と、文字との間の幅(画像が無ければ 0)。
    float icon_extent() const;

    std::string text_;
    Font font_;
    ThemeColor color_;
    Align alignment_;
    bool word_wrap_;
    Image icon_;
    float icon_size_;
    mutable TextLayout layout_;
    mutable float layout_wrap_;  ///< layout_ を作ったときの折り返しの幅(0 なら折り返さない)
};

}  // namespace tokotachi

#endif
