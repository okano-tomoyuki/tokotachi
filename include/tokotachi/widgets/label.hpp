/// @file
/// テキストを表示する。改行文字で行を分け、word_wrap なら幅で折り返す(docs/adr/0019)。
#ifndef TOKOTACHI_WIDGETS_LABEL_HPP
#define TOKOTACHI_WIDGETS_LABEL_HPP

#include <string>

#include "../layout/alignment.hpp"
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
    const TextLayout& layout() const;

    std::string text_;
    Font font_;
    ThemeColor color_;
    Align alignment_;
    bool word_wrap_;
    mutable TextLayout layout_;
    mutable float layout_wrap_;  ///< layout_ を作ったときの折り返しの幅(0 なら折り返さない)
};

}  // namespace tokotachi

#endif
