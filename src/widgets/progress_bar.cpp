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
const int FRAME = 16;              // 動かす間隔(ミリ秒)
const float PERIOD = 1500;         // 動く部分が端から端まで動く時間(ミリ秒)
const float SEGMENT = 0.3f;        // 動く部分の長さ(棒の長さとの比)
const float START_PHASE = 0.35f;   // 動き始めの位置(最初に描いたときにも見えるように)
}  // namespace

ProgressBar::ProgressBar(Widget* parent_widget)
    : Widget(parent_widget),
      TOKOTACHI_PROPERTY_GETTER_SELF_NOTIFY(ProgressBar, double, minimum),
      TOKOTACHI_PROPERTY_GETTER_SELF_NOTIFY(ProgressBar, double, maximum),
      TOKOTACHI_PROPERTY_GETTER(ProgressBar, double, value),
      TOKOTACHI_PROPERTY_GETTER(ProgressBar, bool, text_visible),
      TOKOTACHI_PROPERTY_GETTER(ProgressBar, bool, indeterminate),
      minimum_(0),
      maximum_(100),
      value_(0),
      text_visible_(false),
      indeterminate_(false),
      phase_(START_PHASE),
      animation_(this),
      layout_percent_(-1)
{
    animation_.interval = FRAME;
    animation_.on_timer = [this](Object*)
    {
        if (!indeterminate_ || !showing)
        {
            animation_.enabled = false;  // 隠れている間は止める(また描くときに動かし始める)
            return;
        }
        phase_ += FRAME / PERIOD;
        if (phase_ >= 1)
        {
            phase_ -= 1;
        }
        invalidate();
    };
}

void ProgressBar::set_indeterminate(const bool& v)
{
    if (v == indeterminate_)
    {
        return;
    }
    indeterminate_ = v;
    phase_ = START_PHASE;
    if (!v)
    {
        animation_.enabled = false;
    }
    invalidate();
    request_layout();
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
    if (!shows_text())
    {
        return Size(160, BAR_HEIGHT);
    }
    return Size(160 + TEXT_GAP + text_width(), std::max(BAR_HEIGHT, std::ceil(text_layout().height())));
}

Size ProgressBar::compute_minimum_size() const
{
    const Size p = compute_preferred_size();
    return Size(shows_text() ? 48 + TEXT_GAP + text_width() : 48, p.height);
}

void ProgressBar::paint(Canvas& canvas)
{
    const Rect r = local_rect;
    const bool enabled_now = effectively_enabled;
    const float text_room = shows_text() ? TEXT_GAP + text_width() : 0;
    const Rect bar(0, (r.height - BAR_HEIGHT) / 2, std::max(0.0f, r.width - text_room), BAR_HEIGHT);
    canvas.fill_round_rect(bar, BAR_HEIGHT / 2, enabled_now ? theme::scroll_track() : theme::field_disabled());
    if (indeterminate_)
    {
        // 棒の一部(SEGMENT)が左の外から右の外へ動く。棒の外は描かない
        if (!animation_.enabled.get())
        {
            animation_.enabled = true;
        }
        const float x = bar.x + (phase_ * (1 + SEGMENT) - SEGMENT) * bar.width;
        canvas.save();
        canvas.clip_rect(bar);
        canvas.fill_round_rect(Rect(x, bar.y, SEGMENT * bar.width, BAR_HEIGHT), BAR_HEIGHT / 2,
                               enabled_now ? theme::accent() : theme::accent_disabled());
        canvas.restore();
        Widget::paint(canvas);
        return;
    }
    const float done = static_cast<float>(ratio()) * bar.width;
    if (done > 0)
    {
        // 短いときも丸みが崩れないよう、幅を棒の太さ以上にする。
        const Rect fill(bar.x, bar.y, std::max(done, std::min(BAR_HEIGHT, bar.width)), BAR_HEIGHT);
        canvas.fill_round_rect(fill, BAR_HEIGHT / 2, enabled_now ? theme::accent() : theme::accent_disabled());
    }
    if (shows_text())
    {
        const TextLayout& l = text_layout();
        // 右寄せにして、桁が変わっても棒の端の位置が動かないようにする。
        canvas.draw_text(l, Point(r.width - l.width(), (r.height - l.height()) / 2),
                         enabled_now ? theme::text() : theme::text_disabled());
    }
    Widget::paint(canvas);
}

}  // namespace tokotachi
