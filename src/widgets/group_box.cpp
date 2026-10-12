#include <tokotachi/widgets/group_box.hpp>

#include <algorithm>
#include <cmath>

#include <tokotachi/graphics/canvas.hpp>

#include "../actions/mnemonic.hpp"
#include "../core/impl.hpp"
#include "../layout/layout_access.hpp"
#include "theme.hpp"
#include "widget_access.hpp"

namespace tokotachi
{

namespace
{
const float PADDING = 12;    // 枠と中の部品の間
const float TITLE_X = 10;    // 見出しの左端(枠の左端から)
const float TITLE_GAP = 4;   // 見出しの左右の、枠の線を切る幅
}  // namespace

class GroupBox::Impl : public detail::WidgetHooks
{
    TOKOTACHI_IMPL(GroupBox);

    explicit Impl(GroupBox* object) : q(object) {}

    // 内部のフック(docs/adr/0058)
    Key mnemonic_key() const override;
    void mnemonic_activated() override;

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

GroupBox::GroupBox(Widget* parent_widget, const std::string& initial_title)
    : Widget(parent_widget),
      TOKOTACHI_IMPL_PROPERTY_MEMBER(Impl, std::string, title),
      TOKOTACHI_IMPL_READONLY_PROPERTY(Impl, Rect, content_rect),
      d_(new Impl(this))
{
    d_->install(this);
    d_->title_ = initial_title;
    d_->mnemonic_offset_ = std::string::npos;
    d_->mnemonic_ = Key::UNKNOWN;
    const detail::Mnemonic m = detail::parse_mnemonic(d_->title_);
    d_->display_ = m.text;
    d_->mnemonic_offset_ = m.offset;
    d_->mnemonic_ = m.key;
}

GroupBox::~GroupBox() { delete d_; }

void GroupBox::Impl::set_title(const std::string& v)
{
    if (v == title_)
    {
        return;
    }
    title_ = v;
    const detail::Mnemonic m = detail::parse_mnemonic(title_);
    display_ = m.text;
    mnemonic_offset_ = m.offset;
    mnemonic_ = m.key;
    layout_ = TextLayout();
    q->invalidate();
    q->request_layout();
}

const TextLayout& GroupBox::Impl::title_layout() const
{
    const float scale = q->display_scale;
    if (layout_.is_null() || layout_.scale() != scale)
    {
        layout_ = TextLayout(display_, Font(), scale);
    }
    return layout_;
}

Size GroupBox::Impl::extra() const
{
    const float top = display_.empty() ? PADDING : std::ceil(title_layout().height()) + PADDING / 2;
    return Size(PADDING * 2, top + PADDING);
}

Rect GroupBox::Impl::get_content_rect() const
{
    const Size e = extra();
    const Rect r = q->local_rect;
    return Rect(PADDING, e.height - PADDING, std::max(0.0f, r.width - e.width), std::max(0.0f, r.height - e.height));
}

void GroupBox::arrange_children()
{
    if (Layout* l = detail::WidgetAccess::layout_of(this))
    {
        detail::LayoutAccess::arrange(l, d_->get_content_rect());
    }
}

Size GroupBox::compute_preferred_size() const
{
    const Size inner = Widget::compute_preferred_size();
    const Size e = d_->extra();
    const float title_w = d_->display_.empty() ? 0 : std::ceil(d_->title_layout().width()) + (TITLE_X + TITLE_GAP) * 2;
    return Size(std::max(inner.width + e.width, title_w), inner.height + e.height);
}

Size GroupBox::compute_minimum_size() const
{
    const Size inner = Widget::compute_minimum_size();
    const Size e = d_->extra();
    const float title_w = d_->display_.empty() ? 0 : std::ceil(d_->title_layout().width()) + (TITLE_X + TITLE_GAP) * 2;
    return Size(std::max(inner.width + e.width, title_w), inner.height + e.height);
}

void GroupBox::paint(Canvas& canvas)
{
    const Rect r = local_rect;
    const bool enabled_now = effectively_enabled;
    if (d_->display_.empty())
    {
        canvas.stroke_round_rect(r.adjusted(0.5f, 0.5f, 0.5f, 0.5f), theme::radius(), 1, theme::border());
    }
    else
    {
        // 枠の上の辺は見出しの中ほどを通し、見出しの所だけ線を切る(重ならない 3 つの範囲に分けて描く)
        const TextLayout& l = d_->title_layout();
        const float th = std::ceil(l.height());
        const Rect frame(0.5f, std::floor(th / 2) + 0.5f, r.width - 1, r.height - std::floor(th / 2) - 1);
        const float gap0 = TITLE_X, gap1 = std::min(r.width, TITLE_X + TITLE_GAP * 2 + l.width());
        const Rect parts[] = {Rect(0, th, r.width, std::max(0.0f, r.height - th)), Rect(0, 0, gap0, th),
                              Rect(gap1, 0, std::max(0.0f, r.width - gap1), th)};
        for (int i = 0; i < 3; ++i)
        {
            canvas.save();
            canvas.clip_rect(parts[i]);
            canvas.stroke_round_rect(frame, theme::radius(), 1, theme::border());
            canvas.restore();
        }
        const Point at(TITLE_X + TITLE_GAP, 0);
        const Color fg = enabled_now ? theme::text() : theme::text_disabled();
        canvas.draw_text(l, at, fg);
        if (detail::WidgetAccess::mnemonic_cues(this))
        {
            detail::paint_mnemonic_underline(canvas, l, at, d_->mnemonic_offset_, fg);
        }
    }
    Widget::paint(canvas);
}

void GroupBox::scale_changed() { d_->layout_ = TextLayout(); }

Key GroupBox::Impl::mnemonic_key() const { return mnemonic_; }

void GroupBox::Impl::mnemonic_activated() { q->focus_first_within(); }

}  // namespace tokotachi
