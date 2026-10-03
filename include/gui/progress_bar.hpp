// 進み具合を横の棒で見せる(docs/adr/0024)。利用者は操作できない。
//
//   gui::ProgressBar* progress = new gui::ProgressBar(panel);
//   progress->maximum = total;
//   progress->value = done;
//   progress->text_visible = true;  // 右に「40%」を出す
#ifndef GUI_PROGRESS_BAR_HPP
#define GUI_PROGRESS_BAR_HPP

#include "text_layout.hpp"
#include "widget.hpp"

namespace gui
{

class ProgressBar : public Widget
{
public:
    explicit ProgressBar(Widget* parent = nullptr);

    // 範囲(既定 0〜100)。minimum を maximum より大きくすると maximum も同じ値にする(逆も同様)。
    Property<double> minimum;
    Property<double> maximum;
    // 値(既定 0)。範囲の外は丸める。
    Property<double> value;
    // 割合(「40%」)を棒の右に出すか(既定 false)。
    Property<bool> text_visible;

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

    // 割合(0〜1)。
    double ratio() const;
    // 割合の文字のレイアウトと、その幅として取っておく大きさ(「100%」の幅)。
    const TextLayout& text_layout() const;
    float text_width() const;

    double minimum_;
    double maximum_;
    double value_;
    bool text_visible_;
    mutable TextLayout layout_;
    mutable int layout_percent_;  // layout_ を作った割合(-1 なら未作成)
};

}  // namespace gui

#endif
