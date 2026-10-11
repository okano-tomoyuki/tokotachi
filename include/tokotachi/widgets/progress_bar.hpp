/// @file
/// 進み具合を横の棒で見せる(docs/adr/0024)。利用者は操作できない。
///
/// @code
/// tokotachi::ProgressBar* progress_bar_copy = new tokotachi::ProgressBar(panel);
/// progress_bar_copy->maximum = total;
/// progress_bar_copy->value = done;
/// progress_bar_copy->text_visible = true;  // 右に「40%」を出す
/// progress_bar_copy->indeterminate = true;  // 全体の量が分からない間: 棒の一部が動き続ける
/// @endcode
#ifndef TOKOTACHI_WIDGETS_PROGRESS_BAR_HPP
#define TOKOTACHI_WIDGETS_PROGRESS_BAR_HPP

#include "../graphics/text_layout.hpp"
#include "../core/timer.hpp"
#include "widget.hpp"

namespace tokotachi
{

class ProgressBar : public Widget
{
public:
    explicit ProgressBar(Widget* parent = nullptr);

    /// 範囲(既定 0〜100)。minimum を maximum より大きくすると maximum も同じ値にする(逆も同様)。
    Property<double> minimum;
    Property<double> maximum;
    /// 値(既定 0)。範囲の外は丸める。
    Property<double> value;
    /// 割合(「40%」)を棒の右に出すか(既定 false)。
    Property<bool> text_visible;
    /// 進み具合が分からない(既定 false。docs/adr/0049)。true なら value を使わず、棒の一部が左から右へ動き続ける
    /// (表示されている間だけ動かす)。割合の文字は出さない。
    Property<bool> indeterminate;

protected:
    void paint(Canvas& canvas) override;
    void scale_changed() override;
    Size compute_preferred_size() const override;
    Size compute_minimum_size() const override;

private:
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

}  // namespace tokotachi

#endif
