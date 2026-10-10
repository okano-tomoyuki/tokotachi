#include <tokotachi/widgets/progress_bar.hpp>

#include <algorithm>
#include <cmath>
#include <string>

#include <tokotachi/graphics/canvas.hpp>

#include "../core/property_impl.hpp"
#include "theme.hpp"

namespace tokotachi
{

namespace
{
const float BAR_HEIGHT = 8;  // 棒の太さ(dp)
const float TEXT_GAP = 8;    // 棒と割合の文字の間(dp)
}  // namespace

ProgressBar::ProgressBar(Widget* parent_widget)
    : Widget(parent_widget),
      TOKOTACHI_PROPERTY_GETTER_SELF_NOTIFY(ProgressBar, double, minimum),
      TOKOTACHI_PROPERTY_GETTER_SELF_NOTIFY(ProgressBar, double, maximum),
      TOKOTACHI_PROPERTY_GETTER(ProgressBar, double, value),
      TOKOTACHI_PROPERTY_GETTER(ProgressBar, bool, text_visible),
      minimum_(0),
      maximum_(100),
      value_(0),
      text_visible_(false),
      layout_percent_(-1)
{
}

void ProgressBar::set_minimum(const double& v)
{
    if (v == minimum_ || std::isnan(v))
    {
        return;
    }
    minimum_ = v;
    const bool maximum_moved = maximum_ < v;
    if (maximum_moved)
    {
        maximum_ = v;
    }
    invalidate();
    notify_changed(minimum);
    if (maximum_moved)
    {
        notify_changed(maximum);
    }
    value = value_;  // 範囲に収め直す
}

void ProgressBar::set_maximum(const double& v)
{
    if (v == maximum_ || std::isnan(v))
    {
        return;
    }
    maximum_ = v;
    const bool minimum_moved = minimum_ > v;
    if (minimum_moved)
    {
        minimum_ = v;
    }
    invalidate();
    notify_changed(maximum);
    if (minimum_moved)
    {
        notify_changed(minimum);
    }
    value = value_;
}

void ProgressBar::set_value(const double& v)
{
    if (std::isnan(v))
    {
        return;
    }
    value_ = std::max(minimum_, std::min(v, maximum_));
    invalidate();
}

void ProgressBar::set_text_visible(const bool& v)
{
    text_visible_ = v;
    invalidate();
    request_layout();
}

double ProgressBar::ratio() const { return maximum_ > minimum_ ? (value_ - minimum_) / (maximum_ - minimum_) : 0.0; }

const TextLayout& ProgressBar::text_layout() const
{
    const int percent = static_cast<int>(std::floor(ratio() * 100 + 0.5));
    const float scale = display_scale;
    if (layout_.is_null() || layout_.scale() != scale || layout_percent_ != percent)
    {
        layout_ = TextLayout(std::to_string(percent) + "%", Font(), scale);
        layout_percent_ = percent;
    }
    return layout_;
}

float ProgressBar::text_width() const { return std::ceil(TextLayout("100%", Font(), display_scale).width()); }

void ProgressBar::scale_changed() { layout_ = TextLayout(); }

Size ProgressBar::compute_preferred_size() const
{
    if (!text_visible_)
    {
        return Size(160, BAR_HEIGHT);
    }
    return Size(160 + TEXT_GAP + text_width(), std::max(BAR_HEIGHT, std::ceil(text_layout().height())));
}

Size ProgressBar::compute_minimum_size() const
{
    const Size p = compute_preferred_size();
    return Size(text_visible_ ? 48 + TEXT_GAP + text_width() : 48, p.height);
}

void ProgressBar::paint(Canvas& canvas)
{
    const Rect r = local_rect;
    const bool enabled_now = effectively_enabled;
    const float text_room = text_visible_ ? TEXT_GAP + text_width() : 0;
    const Rect bar(0, (r.height - BAR_HEIGHT) / 2, std::max(0.0f, r.width - text_room), BAR_HEIGHT);
    canvas.fill_round_rect(bar, BAR_HEIGHT / 2, enabled_now ? theme::scroll_track() : theme::field_disabled());
    const float done = static_cast<float>(ratio()) * bar.width;
    if (done > 0)
    {
        // 短いときも丸みが崩れないよう、幅を棒の太さ以上にする。
        const Rect fill(bar.x, bar.y, std::max(done, std::min(BAR_HEIGHT, bar.width)), BAR_HEIGHT);
        canvas.fill_round_rect(fill, BAR_HEIGHT / 2, enabled_now ? theme::accent() : theme::accent_disabled());
    }
    if (text_visible_)
    {
        const TextLayout& l = text_layout();
        // 右寄せにして、桁が変わっても棒の端の位置が動かないようにする。
        canvas.draw_text(l, Point(r.width - l.width(), (r.height - l.height()) / 2),
                         enabled_now ? theme::text() : theme::text_disabled());
    }
    Widget::paint(canvas);
}

}  // namespace tokotachi
