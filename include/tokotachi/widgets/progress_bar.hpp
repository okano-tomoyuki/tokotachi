/// @file
/// tokotachi::ProgressBar の宣言。
#ifndef TOKOTACHI_WIDGETS_PROGRESS_BAR_HPP
#define TOKOTACHI_WIDGETS_PROGRESS_BAR_HPP

#include "../core/export.hpp"
#include "../graphics/text_layout.hpp"
#include "../core/timer.hpp"
#include "widget.hpp"

namespace tokotachi
{

/// 進み具合を横の棒で見せる(docs/adr/0024)。利用者は操作できない。
///
/// @image html widget_progress_bar.png
///
/// @code
/// tokotachi::ProgressBar* progress_bar_copy = new tokotachi::ProgressBar(panel);
/// progress_bar_copy->maximum = total;
/// progress_bar_copy->value = done;
/// progress_bar_copy->text_visible = true;  // 右に「40%」を出す
/// progress_bar_copy->indeterminate = true;  // 全体の量が分からない間: 棒の一部が動き続ける
/// @endcode
///
/// @ingroup widgets_display
class TOKOTACHI_API ProgressBar : public Widget
{
public:
    explicit ProgressBar(Widget* parent = nullptr);
    ~ProgressBar() override;

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
    class Impl;
    Impl* d_;
};

}  // namespace tokotachi

#endif
