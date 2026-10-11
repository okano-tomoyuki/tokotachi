#include <tokotachi/widgets/label.hpp>

#include <algorithm>
#include <cmath>

#include <tokotachi/graphics/canvas.hpp>

#include "../actions/mnemonic.hpp"
#include "../core/property_impl.hpp"
#include "theme.hpp"

namespace tokotachi
{

Label::Label(Widget* parent_widget, const std::string& initial_text)
    : Widget(parent_widget),
      TOKOTACHI_PROPERTY_MEMBER(Label, std::string, text),
      TOKOTACHI_PROPERTY_MEMBER(Label, Font, font),
      TOKOTACHI_PROPERTY_MEMBER(Label, ThemeColor, color),
      TOKOTACHI_PROPERTY_MEMBER(Label, Align, alignment),
      TOKOTACHI_PROPERTY_MEMBER(Label, bool, word_wrap),
      TOKOTACHI_PROPERTY_MEMBER(Label, Image, icon),
      TOKOTACHI_PROPERTY_MEMBER(Label, float, icon_size),
      TOKOTACHI_PROPERTY_GETTER(Label, Widget*, buddy),
      text_(initial_text),
      mnemonic_offset_(std::string::npos),
      mnemonic_(Key::UNKNOWN),
      color_(ColorRole::TEXT),
      alignment_(Align::START),
      word_wrap_(false),
      icon_size_(16),
      layout_wrap_(0)
{
    const detail::Mnemonic m = detail::parse_mnemonic(text_);
    display_ = m.text;
    mnemonic_offset_ = m.offset;
    mnemonic_ = m.key;
}

void Label::set_text(const std::string& v)
{
    if (v == text_)
    {
        return;
    }
    text_ = v;
    const detail::Mnemonic m = detail::parse_mnemonic(text_);
    display_ = m.text;
    mnemonic_offset_ = m.offset;
    mnemonic_ = m.key;
    layout_ = TextLayout();
    invalidate();
    request_layout();
}

void Label::set_buddy(Widget* const& v) { buddy_ = WeakRef<Widget>(v); }

Key Label::mnemonic_key() const { return mnemonic_; }

void Label::mnemonic_activated()
{
    if (Widget* b = buddy_.get())
    {
        b->focus_first_within();
    }
    else
    {
        focus_next_after();
    }
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

void Label::set_icon(const Image& v)
{
    if (v == icon_)
    {
        return;
    }
    const bool shown = !icon_.is_null();
    icon_ = v;
    invalidate();
    if (shown != !icon_.is_null())
    {
        layout_ = TextLayout();
        request_layout();
    }
}

void Label::set_icon_size(const float& v)
{
    if (v == icon_size_)
    {
        return;
    }
    icon_size_ = std::max(0.0f, v);
    layout_ = TextLayout();
    invalidate();
    request_layout();
}

namespace
{
const float ICON_TEXT_GAP = 6;  // 画像と文字の間
}  // namespace

float Label::icon_extent() const
{
    if (icon_.is_null())
    {
        return 0;
    }
    return display_.empty() ? icon_size_ : icon_size_ + ICON_TEXT_GAP;
}

const TextLayout& Label::layout() const
{
    // 折り返すときは今の幅(画像の分を除く)で作る。幅・倍率が変わったら作り直す。
    const float scale = display_scale;
    const float wrap = word_wrap_ ? std::max(1.0f, width.get() - icon_extent()) : 0;
    if (layout_.is_null() || layout_.scale() != scale || layout_wrap_ != wrap)
    {
        layout_ = TextLayout(display_, font_, scale, wrap, alignment_);
        layout_wrap_ = wrap;
    }
    return layout_;
}

Size Label::compute_preferred_size() const
{
    // 幅は折り返さないときの幅、高さは今の幅で折り返したときの高さ。
    const TextLayout& l = layout();
    const float natural = word_wrap_ ? TextLayout(display_, font_, l.scale()).width() : l.width();
    const float icon_h = icon_.is_null() ? 0 : icon_size_;
    return Size(std::ceil(icon_extent() + natural), std::ceil(std::max(l.height(), icon_h)));
}

Size Label::compute_minimum_size() const
{
    if (!word_wrap_)
    {
        return compute_preferred_size();  // 文字が切れないよう、望む大きさを最小とする
    }
    // 最も長い単語の幅(幅 1 で折り返すと、1 行に 1 語ずつ並ぶ)と、今の幅での高さ。
    const TextLayout narrow(display_, font_, layout().scale(), 1);
    const float icon_h = icon_.is_null() ? 0 : icon_size_;
    return Size(std::ceil(icon_extent() + narrow.width()), std::ceil(std::max(layout().height(), icon_h)));
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
    const bool rtl = l.line_is_rtl(0);
    // START・END は段落の向きの始め・終わり(右から左の段落では START が右。docs/adr/0020)。
    Align a = alignment_ == Align::FILL ? Align::START : alignment_;
    if (rtl && a != Align::CENTER)
    {
        a = a == Align::START ? Align::END : Align::START;
    }
    // 画像と文字をひとまとまりとしてそろえる。画像は段落の向きの始め(左から右なら左)に置く
    const float extent = icon_extent();
    const float content_w = extent + (word_wrap_ ? std::max(0.0f, w - extent) : l.width());
    float x = 0;
    if (a == Align::CENTER)
    {
        x = (w - content_w) / 2;
    }
    else if (a == Align::END)
    {
        x = w - content_w;
    }
    if (word_wrap_)
    {
        x = 0;  // 折り返すときは、行のそろえ方を TextLayout が折り返しの幅(= 自分の幅から画像を除いた幅)に対して行う
    }
    const float tx = rtl ? x : x + extent;
    if (!icon_.is_null())
    {
        const float ix = rtl ? x + content_w - icon_size_ : x;
        canvas.draw_image(icon_, Rect(ix, (height - icon_size_) / 2, icon_size_, icon_size_), effectively_enabled ? 1.0f : 0.4f);
    }
    const Point at(tx, (height - l.height()) / 2);
    const Color fg = effectively_enabled ? color_.resolve() : theme::text_disabled();
    canvas.draw_text(l, at, fg);
    if (mnemonic_cues())
    {
        detail::paint_mnemonic_underline(canvas, l, at, mnemonic_offset_, fg);
    }
    Widget::paint(canvas);
}

void Label::scale_changed() { layout_ = TextLayout(); }

}  // namespace tokotachi
