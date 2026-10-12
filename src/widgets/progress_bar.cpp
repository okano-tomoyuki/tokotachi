#include <tokotachi/widgets/progress_bar.hpp>

#include <algorithm>
#include <cmath>
#include <string>

#include <tokotachi/graphics/canvas.hpp>

#include "../core/impl.hpp"
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

class ProgressBar::Impl
{
    TOKOTACHI_IMPL(ProgressBar);

    explicit Impl(ProgressBar* object) : q(object), animation_(object) {}

    double get_minimum() const { return minimum_; }
    void set_minimum(const double& v);
    double get_maximum() const { return maximum_; }
    void set_maximum(const double& v);
    double get_value() const { return value_; }
    void set_value(const double& v);
    bool get_text_visible() const { return text_visible_; }
    void set_text_visible(const bool& v);
    bool get_indeterminate() const { return indeterminate_; }
    void set_indeterminate(const bool& v);
    /// 割合の文字を出すか(text_visible で、進み具合が分かるとき)。
    bool shows_text() const { return text_visible_ && !indeterminate_; }

    /// 割合(0〜1)。
    double ratio() const;
    /// 割合の文字のレイアウトと、その幅として取っておく大きさ(「100%」の幅)。
    const TextLayout& text_layout() const;
    float text_width() const;

    double minimum_;
    double maximum_;
    double value_;
    bool text_visible_;
    bool indeterminate_;
    float phase_;      ///< 動く部分の位置(0〜1 を繰り返す)
    Timer animation_;  ///< 動かす(表示されている間だけ)
    mutable TextLayout layout_;
    mutable int layout_percent_;  ///< layout_ を作った割合(-1 なら未作成)
};

ProgressBar::ProgressBar(Widget* parent_widget)
    : Widget(parent_widget),
      TOKOTACHI_IMPL_PROPERTY_GETTER_SELF_NOTIFY(Impl, double, minimum),
      TOKOTACHI_IMPL_PROPERTY_GETTER_SELF_NOTIFY(Impl, double, maximum),
      TOKOTACHI_IMPL_PROPERTY_GETTER(Impl, double, value),
      TOKOTACHI_IMPL_PROPERTY_GETTER(Impl, bool, text_visible),
      TOKOTACHI_IMPL_PROPERTY_GETTER(Impl, bool, indeterminate),
      d_(new Impl(this))
{
    d_->minimum_ = 0;
    d_->maximum_ = 100;
    d_->value_ = 0;
    d_->text_visible_ = false;
    d_->indeterminate_ = false;
    d_->phase_ = START_PHASE;
    d_->layout_percent_ = -1;
    d_->animation_.interval = FRAME;
    d_->animation_.on_timeout = [this](Object*)
    {
        if (!d_->indeterminate_ || !showing)
        {
            d_->animation_.enabled = false;  // 隠れている間は止める(また描くときに動かし始める)
            return;
        }
        d_->phase_ += FRAME / PERIOD;
        if (d_->phase_ >= 1)
        {
            d_->phase_ -= 1;
        }
        invalidate();
    };
}

ProgressBar::~ProgressBar() { delete d_; }

void ProgressBar::Impl::set_indeterminate(const bool& v)
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
    q->invalidate();
    q->request_layout();
}

void ProgressBar::Impl::set_minimum(const double& v)
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
    q->invalidate();
    q->notify_changed(q->minimum);
    if (maximum_moved)
    {
        q->notify_changed(q->maximum);
    }
    q->value = value_;  // 範囲に収め直す
}

void ProgressBar::Impl::set_maximum(const double& v)
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
    q->invalidate();
    q->notify_changed(q->maximum);
    if (minimum_moved)
    {
        q->notify_changed(q->minimum);
    }
    q->value = value_;
}

void ProgressBar::Impl::set_value(const double& v)
{
    if (std::isnan(v))
    {
        return;
    }
    value_ = std::max(minimum_, std::min(v, maximum_));
    q->invalidate();
}

void ProgressBar::Impl::set_text_visible(const bool& v)
{
    text_visible_ = v;
    q->invalidate();
    q->request_layout();
}

double ProgressBar::Impl::ratio() const { return maximum_ > minimum_ ? (value_ - minimum_) / (maximum_ - minimum_) : 0.0; }

const TextLayout& ProgressBar::Impl::text_layout() const
{
    const int percent = static_cast<int>(std::floor(ratio() * 100 + 0.5));
    const float scale = q->display_scale;
    if (layout_.is_null() || layout_.scale() != scale || layout_percent_ != percent)
    {
        layout_ = TextLayout(std::to_string(percent) + "%", Font(), scale);
        layout_percent_ = percent;
    }
    return layout_;
}

float ProgressBar::Impl::text_width() const { return std::ceil(TextLayout("100%", Font(), q->display_scale).width()); }

void ProgressBar::scale_changed() { d_->layout_ = TextLayout(); }

Size ProgressBar::compute_preferred_size() const
{
    if (!d_->shows_text())
    {
        return Size(160, BAR_HEIGHT);
    }
    return Size(160 + TEXT_GAP + d_->text_width(), std::max(BAR_HEIGHT, std::ceil(d_->text_layout().height())));
}

Size ProgressBar::compute_minimum_size() const
{
    const Size p = compute_preferred_size();
    return Size(d_->shows_text() ? 48 + TEXT_GAP + d_->text_width() : 48, p.height);
}

void ProgressBar::paint(Canvas& canvas)
{
    const Rect r = local_rect;
    const bool enabled_now = effectively_enabled;
    const float text_room = d_->shows_text() ? TEXT_GAP + d_->text_width() : 0;
    const Rect bar(0, (r.height - BAR_HEIGHT) / 2, std::max(0.0f, r.width - text_room), BAR_HEIGHT);
    canvas.fill_round_rect(bar, BAR_HEIGHT / 2, enabled_now ? theme::scroll_track() : theme::field_disabled());
    if (d_->indeterminate_)
    {
        // 棒の一部(SEGMENT)が左の外から右の外へ動く。棒の外は描かない
        if (!d_->animation_.enabled.get())
        {
            d_->animation_.enabled = true;
        }
        const float x = bar.x + (d_->phase_ * (1 + SEGMENT) - SEGMENT) * bar.width;
        canvas.save();
        canvas.clip_rect(bar);
        canvas.fill_round_rect(Rect(x, bar.y, SEGMENT * bar.width, BAR_HEIGHT), BAR_HEIGHT / 2,
                               enabled_now ? theme::accent() : theme::accent_disabled());
        canvas.restore();
        Widget::paint(canvas);
        return;
    }
    const float done = static_cast<float>(d_->ratio()) * bar.width;
    if (done > 0)
    {
        // 短いときも丸みが崩れないよう、幅を棒の太さ以上にする。
        const Rect fill(bar.x, bar.y, std::max(done, std::min(BAR_HEIGHT, bar.width)), BAR_HEIGHT);
        canvas.fill_round_rect(fill, BAR_HEIGHT / 2, enabled_now ? theme::accent() : theme::accent_disabled());
    }
    if (d_->shows_text())
    {
        const TextLayout& l = d_->text_layout();
        // 右寄せにして、桁が変わっても棒の端の位置が動かないようにする。
        canvas.draw_text(l, Point(r.width - l.width(), (r.height - l.height()) / 2),
                         enabled_now ? theme::text() : theme::text_disabled());
    }
    Widget::paint(canvas);
}

}  // namespace tokotachi
