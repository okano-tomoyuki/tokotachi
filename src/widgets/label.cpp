#include <tokotachi/widgets/label.hpp>

#include <algorithm>
#include <cmath>

#include <tokotachi/graphics/canvas.hpp>

#include "../actions/mnemonic.hpp"
#include "../core/impl.hpp"
#include "theme.hpp"

namespace tokotachi
{

class Label::Impl
{
    TOKOTACHI_IMPL(Label);

    explicit Impl(Label* object) : q(object) {}

    void set_text(const std::string& v);
    void set_font(const Font& v);
    void set_color(const ThemeColor& v);
    void set_alignment(const Align& v);
    void set_word_wrap(const bool& v);
    void set_icon(const Image& v);
    void set_icon_size(const float& v);
    Widget* get_buddy() const { return buddy_.get(); }
    void set_buddy(Widget* const& v);
    const TextLayout& layout() const;
    /// 画像と、文字との間の幅(画像が無ければ 0)。
    float icon_extent() const;

    std::string text_;
    std::string display_;           ///< text_ から & を除いた、表示する文字
    std::size_t mnemonic_offset_;   ///< display_ の中のニーモニックの文字の位置(無ければ npos)
    Key mnemonic_;
    WeakRef<Widget> buddy_;
    Font font_;
    ThemeColor color_;
    Align alignment_;
    bool word_wrap_;
    Image icon_;
    float icon_size_;
    mutable TextLayout layout_;
    mutable float layout_wrap_;  ///< layout_ を作ったときの折り返しの幅(0 なら折り返さない)
};

Label::Label(Widget* parent_widget, const std::string& initial_text)
    : Widget(parent_widget),
      TOKOTACHI_IMPL_PROPERTY_MEMBER(Impl, std::string, text),
      TOKOTACHI_IMPL_PROPERTY_MEMBER(Impl, Font, font),
      TOKOTACHI_IMPL_PROPERTY_MEMBER(Impl, ThemeColor, color),
      TOKOTACHI_IMPL_PROPERTY_MEMBER(Impl, Align, alignment),
      TOKOTACHI_IMPL_PROPERTY_MEMBER(Impl, bool, word_wrap),
      TOKOTACHI_IMPL_PROPERTY_MEMBER(Impl, Image, icon),
      TOKOTACHI_IMPL_PROPERTY_MEMBER(Impl, float, icon_size),
      TOKOTACHI_IMPL_PROPERTY_GETTER(Impl, Widget*, buddy),
      d_(new Impl(this))
{
    d_->text_ = initial_text;
    d_->mnemonic_offset_ = std::string::npos;
    d_->mnemonic_ = Key::UNKNOWN;
    d_->color_ = ColorRole::TEXT;
    d_->alignment_ = Align::START;
    d_->word_wrap_ = false;
    d_->icon_size_ = 16;
    d_->layout_wrap_ = 0;
    const detail::Mnemonic m = detail::parse_mnemonic(d_->text_);
    d_->display_ = m.text;
    d_->mnemonic_offset_ = m.offset;
    d_->mnemonic_ = m.key;
}

Label::~Label() { delete d_; }

void Label::Impl::set_text(const std::string& v)
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
    q->invalidate();
    q->request_layout();
}

void Label::Impl::set_buddy(Widget* const& v) { buddy_ = WeakRef<Widget>(v); }

Key Label::mnemonic_key() const { return d_->mnemonic_; }

void Label::mnemonic_activated()
{
    if (Widget* b = d_->buddy_.get())
    {
        b->focus_first_within();
    }
    else
    {
        focus_next_after();
    }
}

void Label::Impl::set_font(const Font& v)
{
    if (v == font_)
    {
        return;
    }
    font_ = v;
    layout_ = TextLayout();
    q->invalidate();
    q->request_layout();
}

void Label::Impl::set_color(const ThemeColor& v)
{
    color_ = v;
    q->invalidate();
}

void Label::Impl::set_alignment(const Align& v)
{
    alignment_ = v;
    layout_ = TextLayout();
    q->invalidate();
}

void Label::Impl::set_word_wrap(const bool& v)
{
    word_wrap_ = v;
    layout_ = TextLayout();
    q->invalidate();
    q->request_layout();
}

void Label::Impl::set_icon(const Image& v)
{
    if (v == icon_)
    {
        return;
    }
    const bool shown = !icon_.is_null();
    icon_ = v;
    q->invalidate();
    if (shown != !icon_.is_null())
    {
        layout_ = TextLayout();
        q->request_layout();
    }
}

void Label::Impl::set_icon_size(const float& v)
{
    if (v == icon_size_)
    {
        return;
    }
    icon_size_ = std::max(0.0f, v);
    layout_ = TextLayout();
    q->invalidate();
    q->request_layout();
}

namespace
{
const float ICON_TEXT_GAP = 6;  // 画像と文字の間
}  // namespace

float Label::Impl::icon_extent() const
{
    if (icon_.is_null())
    {
        return 0;
    }
    return display_.empty() ? icon_size_ : icon_size_ + ICON_TEXT_GAP;
}

const TextLayout& Label::Impl::layout() const
{
    // 折り返すときは今の幅(画像の分を除く)で作る。幅・倍率が変わったら作り直す。
    const float scale = q->display_scale;
    const float wrap = word_wrap_ ? std::max(1.0f, q->width.get() - icon_extent()) : 0;
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
    const TextLayout& l = d_->layout();
    const float natural = d_->word_wrap_ ? TextLayout(d_->display_, d_->font_, l.scale()).width() : l.width();
    const float icon_h = d_->icon_.is_null() ? 0 : d_->icon_size_;
    return Size(std::ceil(d_->icon_extent() + natural), std::ceil(std::max(l.height(), icon_h)));
}

Size Label::compute_minimum_size() const
{
    if (!d_->word_wrap_)
    {
        return compute_preferred_size();  // 文字が切れないよう、望む大きさを最小とする
    }
    // 最も長い単語の幅(幅 1 で折り返すと、1 行に 1 語ずつ並ぶ)と、今の幅での高さ。
    const TextLayout narrow(d_->display_, d_->font_, d_->layout().scale(), 1);
    const float icon_h = d_->icon_.is_null() ? 0 : d_->icon_size_;
    return Size(std::ceil(d_->icon_extent() + narrow.width()), std::ceil(std::max(d_->layout().height(), icon_h)));
}

void Label::resized()
{
    if (d_->word_wrap_)
    {
        request_layout();  // 幅が変わると、望む高さが変わる
    }
    Widget::resized();
}

void Label::paint(Canvas& canvas)
{
    const TextLayout& l = d_->layout();
    const float w = width;
    const bool rtl = l.line_is_rtl(0);
    // START・END は段落の向きの始め・終わり(右から左の段落では START が右。docs/adr/0020)。
    Align a = d_->alignment_ == Align::FILL ? Align::START : d_->alignment_;
    if (rtl && a != Align::CENTER)
    {
        a = a == Align::START ? Align::END : Align::START;
    }
    // 画像と文字をひとまとまりとしてそろえる。画像は段落の向きの始め(左から右なら左)に置く
    const float extent = d_->icon_extent();
    const float content_w = extent + (d_->word_wrap_ ? std::max(0.0f, w - extent) : l.width());
    float x = 0;
    if (a == Align::CENTER)
    {
        x = (w - content_w) / 2;
    }
    else if (a == Align::END)
    {
        x = w - content_w;
    }
    if (d_->word_wrap_)
    {
        x = 0;  // 折り返すときは、行のそろえ方を TextLayout が折り返しの幅(= 自分の幅から画像を除いた幅)に対して行う
    }
    const float tx = rtl ? x : x + extent;
    const Color fg = effectively_enabled ? d_->color_.resolve() : theme::text_disabled();
    if (!d_->icon_.is_null())
    {
        const float ix = rtl ? x + content_w - d_->icon_size_ : x;
        canvas.draw_image(d_->icon_, Rect(ix, (height - d_->icon_size_) / 2, d_->icon_size_, d_->icon_size_), fg,
                          effectively_enabled || d_->icon_.is_icon() ? 1.0f : 0.4f);
    }
    const Point at(tx, (height - l.height()) / 2);
    canvas.draw_text(l, at, fg);
    if (mnemonic_cues())
    {
        detail::paint_mnemonic_underline(canvas, l, at, d_->mnemonic_offset_, fg);
    }
    Widget::paint(canvas);
}

void Label::scale_changed() { d_->layout_ = TextLayout(); }

}  // namespace tokotachi
