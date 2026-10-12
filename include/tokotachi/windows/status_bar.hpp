/// @file
/// tokotachi::StatusBar の宣言。
#ifndef TOKOTACHI_WINDOWS_STATUS_BAR_HPP
#define TOKOTACHI_WINDOWS_STATUS_BAR_HPP

#include <string>
#include <vector>

#include "../core/export.hpp"
#include "../graphics/text_layout.hpp"
#include "../core/timer.hpp"
#include "../widgets/widget.hpp"

namespace tokotachi
{

/// ウィンドウの下端のステータスバー(docs/adr/0030)。
///
/// @image html widget_window_bars.png
///
/// @code
/// tokotachi::StatusBar* status_bar = new tokotachi::StatusBar();
/// status_bar->text = "準備完了";                      // 普段の文字(左)
/// status_bar->show_message("保存した", 3000);          // 3 秒だけ差し替える
/// status_bar->add_widget(new tokotachi::Label(0, "行 1, 列 1"));   // 右に並べる部品
/// window.status_bar = status_bar;
/// @endcode
///
/// @ingroup windows
class TOKOTACHI_API StatusBar : public Widget
{
public:
    explicit StatusBar(Widget* parent = nullptr);
    ~StatusBar() override;

    /// 普段に出す文字。
    Property<std::string> text;
    /// 今出している一時的な知らせ(無ければ空)。
    ReadOnlyProperty<std::string> message;

    /// 知らせを出す。timeout_ms が正なら、その時間が経つと text に戻す(0 なら clear_message まで出す)。
    void show_message(const std::string& message, int timeout_ms = 0);
    void clear_message();
    /// 右に部品を並べる(加えた順に左から。ステータスバーの子になる)。
    void add_widget(Widget* widget);

protected:
    void paint(Canvas& canvas) override;
    void arrange_children() override;
    void scale_changed() override;
    Size compute_preferred_size() const override;
    Size compute_minimum_size() const override;

private:
    class Impl;
    Impl* d_;
};

}  // namespace tokotachi

#endif
