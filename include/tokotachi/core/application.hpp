/// @file
/// tokotachi::Application の宣言。
#ifndef TOKOTACHI_CORE_APPLICATION_HPP
#define TOKOTACHI_CORE_APPLICATION_HPP

#include <string>

#include "export.hpp"
#include "event.hpp"
#include "property.hpp"
#include "../graphics/theme.hpp"

namespace tokotachi
{

class Window;

/// アプリケーション: SDL3 の初期化とイベントループ(docs/adr/0010)。1 つのプロセスに 1 つだけ作る。
///
/// main の中でスタックに置く(ライブラリのオブジェクトのうち、スタックに置くのはこれだけ。ほかは new で作る。docs/adr/0062)。
/// 所有者を渡さずに作ったウィンドウは Application が所有し、Application の破棄で破棄する。
///
/// @code
/// int main(int argc, char** argv)
/// {
///     tokotachi::Application app(argc, argv);
///     tokotachi::Window* window_main = new tokotachi::Window("メイン", 640, 480);   // app が所有する
///     app.main_window = window_main;                                             // 閉じたら終える
///     window_main->visible = true;
///     return app.exec();
/// }
/// @endcode
///
/// @ingroup core_objects
class TOKOTACHI_API Application : public Object
{
public:
    Application(int argc, char** argv);
    ~Application();

    static Application* instance();

    /// ライト・ダークの選び方(既定は SYSTEM。docs/adr/0023)。
    Property<ColorScheme> color_scheme;
    /// ライト・ダークのときに使うテーマ(既定は Theme::light()・Theme::dark())。
    Property<Theme> light_theme;
    Property<Theme> dark_theme;
    /// 今ダークのテーマを使っているか。
    ReadOnlyProperty<bool> dark;
    /// 今使っているテーマ。
    ReadOnlyProperty<Theme> theme;

    /// 利用者の言語(「ja」「en-US」などの言語の名前。既定は OS の設定の最初の言語。docs/adr/0051)。
    /// 標準の文字(ダイアログのボタン・暦の月と曜日など)を選ぶ。今は「ja」で始まれば日本語、ほかは英語。
    Property<std::string> language;

    /// メインのウィンドウ(既定 nullptr)。決めていれば、このウィンドウを閉じたら(破棄したときも)ほかのウィンドウが開いていても
    /// exec() を抜ける。決めていなければ、表示しているウィンドウ(ポップアップを除く)が無くなったら抜ける。
    /// ウィンドウが破棄されると nullptr になる。
    Property<Window*> main_window;
    /// ウィンドウを閉じたときに、上の決まりで exec() を抜けるか(既定 true)。false なら quit() を呼ぶまで抜けない(常駐するものなど)。
    Property<bool> auto_quit_enabled;

    /// 使うテーマが変わった(color_scheme・OS の設定・light_theme・dark_theme の変更による)。
    /// 呼ばれる時点で、すべてのウィンドウの描き直しは依頼済み。
    Property<NotifyHandler> on_theme_change;

    /// イベントループを回し、quit() で抜ける。戻り値は quit() に渡した値。
    int exec();
    void quit(int code = 0);

private:
    template <typename>
    friend struct detail::ImplAccess;
    class Impl;
    Impl* d_;
};

}  // namespace tokotachi

#endif
