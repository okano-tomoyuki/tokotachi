// アプリケーション: SDL3 の初期化とイベントループ(docs/adr/0010)。1 つのプロセスに 1 つだけ作る。
#ifndef TOKOTACHI_CORE_APPLICATION_HPP
#define TOKOTACHI_CORE_APPLICATION_HPP

#include "event.hpp"
#include "property.hpp"
#include "../graphics/theme.hpp"

namespace tokotachi
{

namespace detail
{
struct ApplicationImpl;
}

class Application : public Object
{
public:
    Application(int argc, char** argv);
    ~Application();

    static Application* instance();

    // ライト・ダークの選び方(既定は SYSTEM。docs/adr/0023)。
    Property<ColorScheme> color_scheme;
    // ライト・ダークのときに使うテーマ(既定は Theme::light()・Theme::dark())。
    Property<Theme> light_theme;
    Property<Theme> dark_theme;
    // 今ダークのテーマを使っているか。
    ReadOnlyProperty<bool> dark;
    // 今使っているテーマ。
    ReadOnlyProperty<Theme> theme;

    // 使うテーマが変わった(color_scheme・OS の設定・light_theme・dark_theme の変更による)。
    // 呼ばれる時点で、すべてのウィンドウの描き直しは依頼済み。
    Property<NotifyHandler> on_theme_change;

    // イベントループを回し、quit() で抜ける。戻り値は quit() に渡した値。
    int exec();
    void quit(int code = 0);

    detail::ApplicationImpl* impl() const { return d_; }

private:
    friend struct detail::ApplicationImpl;

    ColorScheme get_color_scheme() const;
    void set_color_scheme(const ColorScheme& v);
    Theme get_light_theme() const;
    void set_light_theme(const Theme& v);
    Theme get_dark_theme() const;
    void set_dark_theme(const Theme& v);
    bool get_dark() const;
    Theme get_theme() const;

    detail::ApplicationImpl* d_;
    NotifyHandler on_theme_change_;
};

}  // namespace tokotachi

#endif
