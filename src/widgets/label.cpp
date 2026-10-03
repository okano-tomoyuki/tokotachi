#include <gui/label.hpp>

#include <algorithm>
#include <cmath>

#include <gui/canvas.hpp>

#include "../core/property_impl.hpp"
#include "theme.hpp"

namespace gui
{

Label::Label(Widget* parent_widget, const std::string& initial_text)
    : Widget(parent_widget),
      GUI_PROPERTY_MEMBER(Label, std::string, text),
      GUI_PROPERTY_MEMBER(Label, Font, font),
      GUI_PROPERTY_MEMBER(Label, ThemeColor, color),
      GUI_PROPERTY_MEMBER(Label, Align, alignment),
      GUI_PROPERTY_MEMBER(Label, bool, word_wrap),
      text_(initial_text),
      color_(ColorRole::TEXT),
      alignment_(Align::START),
      word_wrap_(false),
      layout_wrap_(0)
{
}

void Label::set_text(const std::string& v)
{
    if (v == text_)
    {
        return;
    }
    text_ = v;
    layout_ = TextLayout();
    invalidate();
    request_layout();
}

void Label::set_font(const Font& v)
{
    if (v == font_)
    {
        return;
    }
    font_ = v;
    layout_ = TextLayout();
    invalidate();
    request_layout();
}

void Label::set_color(const ThemeColor& v)
{
    color_ = v;
    invalidate();
}

void Label::set_alignment(const Align& v)
{
    alignment_ = v;
    layout_ = TextLayout();
    invalidate();
}

void Label::set_word_wrap(const bool& v)
{
    word_wrap_ = v;
    layout_ = TextLayout();
    invalidate();
    request_layout();
}

const TextLayout& Label::layout() const
{
    // 折り返すときは今の幅で作る。幅・倍率が変わったら作り直す。
    const float scale = display_scale;
    const float wrap = word_wrap_ ? std::max(1.0f, width.get()) : 0;
    if (layout_.is_null() || layout_.scale() != scale || layout_wrap_ != wrap)
    {
        layout_ = TextLayout(text_, font_, scale, wrap, alignment_);
        layout_wrap_ = wrap;
    }
    return layout_;
}

Size Label::compute_preferred_size() const
{
    // 幅は折り返さないときの幅、高さは今の幅で折り返したときの高さ。
    const TextLayout& l = layout();
    const float natural = word_wrap_ ? TextLayout(text_, font_, l.scale()).width() : l.width();
    return Size(std::ceil(natural), std::ceil(l.height()));
}

Size Label::compute_minimum_size() const
{
    if (!word_wrap_)
    {
        return compute_preferred_size();  // 文字が切れないよう、望む大きさを最小とする
    }
    // 最も長い単語の幅(幅 1 で折り返すと、1 行に 1 語ずつ並ぶ)と、今の幅での高さ。
    const TextLayout narrow(text_, font_, layout().scale(), 1);
    return Size(std::ceil(narrow.width()), std::ceil(layout().height()));
}

void Label::resized()
{
    if (word_wrap_)
    {
        request_layout();  // 幅が変わると、望む高さが変わる
    }
    Widget::resized();
}

void Label::paint(Canvas& canvas)
{
    const TextLayout& l = layout();
    const float w = width;
    // START・END は段落の向きの始め・終わり(右から左の段落では START が右。docs/adr/0020)。
    Align a = alignment_ == Align::FILL ? Align::START : alignment_;
    if (l.line_is_rtl(0) && a != Align::CENTER)
    {
        a = a == Align::START ? Align::END : Align::START;
    }
    float tx = 0;
    if (word_wrap_)
    {
        a = Align::START;  // 折り返すときは、行のそろえ方を TextLayout が折り返しの幅(= 自分の幅)に対して行う
    }
    if (a == Align::CENTER)
    {
        tx = (w - l.width()) / 2;
    }
    else if (a == Align::END)
    {
        tx = w - l.width();
    }
    canvas.draw_text(l, Point(tx, (height - l.height()) / 2), effectively_enabled ? color_.resolve() : theme::text_disabled());
    Widget::paint(canvas);
}

void Label::scale_changed() { layout_ = TextLayout(); }

}  // namespace gui
