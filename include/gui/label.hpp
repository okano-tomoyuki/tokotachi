// テキストを表示する。改行文字で行を分け、word_wrap なら幅で折り返す(docs/adr/0019)。
#ifndef GUI_LABEL_HPP
#define GUI_LABEL_HPP

#include <string>

#include "alignment.hpp"
#include "text_layout.hpp"
#include "widget.hpp"

namespace gui
{

class Label : public Widget
{
public:
    explicit Label(Widget* parent = nullptr, const std::string& text = std::string());

    Property<std::string> text;
    Property<Font> font;
    Property<Color> color;
    // 文字の横のそろえ方(既定 START。FILL は START と同じ)。複数行なら各行をそろえる。
    Property<Align> alignment;
    // 幅で折り返すか(既定 false)。折り返すと、望む高さは今の幅で折り返したときの高さになり、最小の幅は最も長い単語の幅になる。
    Property<bool> word_wrap;

protected:
    void paint(Canvas& canvas) override;
    void scale_changed() override;
    Size compute_preferred_size() const override;
    // 文字が切れないよう、望む大きさを最小とする。
    Size compute_minimum_size() const override;
    void resized() override;

private:
    void set_text(const std::string& v);
    void set_font(const Font& v);
    void set_color(const Color& v);
    void set_alignment(const Align& v);
    void set_word_wrap(const bool& v);
    const TextLayout& layout() const;

    std::string text_;
    Font font_;
    Color color_;
    Align alignment_;
    bool word_wrap_;
    mutable TextLayout layout_;
    mutable float layout_wrap_;  // layout_ を作ったときの折り返しの幅(0 なら折り返さない)
};

}  // namespace gui

#endif
