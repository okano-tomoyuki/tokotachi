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

/// アプリケーション: SDL3 の初期化とイベントループ(docs/adr/0010)。1 つのプロセスに 1 つだけ作る。
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
