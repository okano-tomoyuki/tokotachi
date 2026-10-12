/// @file
/// tokotachi::Window の宣言。
#ifndef TOKOTACHI_WINDOWS_WINDOW_HPP
#define TOKOTACHI_WINDOWS_WINDOW_HPP

#include <string>

#include "../core/export.hpp"
#include "../graphics/bitmap.hpp"
#include "../graphics/theme.hpp"
#include "../widgets/widget.hpp"

namespace tokotachi
{

class MenuBar;
class StatusBar;
class ToolBar;

/// トップレベルのウィンドウ。ウィジェットのツリーの根になる。
/// 表示は visible に true を代入する(初めは表示されていない)。
///
/// new で作る(docs/adr/0062)。所有者を渡さなければ Application が所有し、所有者のウィンドウを渡せば、そのウィンドウが所有して
/// その上に出す(ツールのウィンドウ・検索の窓など。モーダルにするには Dialog)。
///
/// @code
/// tokotachi::Window* window_main = new tokotachi::Window("メイン", 640, 480);           // Application が所有する
/// tokotachi::Window* window_find = new tokotachi::Window(window_main, "検索", 360, 120);  // window_main が所有し、その上に出す
/// window_find->visible = true;
/// @endcode
///
/// @image html window.png
///
/// @ingroup windows
class TOKOTACHI_API Window : public Widget
{
public:
    /// 大きさは dp。Application を作った後に作る。Application が所有する。
    explicit Window(const std::string& title, float width = 640, float height = 480);
    /// owner が所有し、表示すると owner の上に出る(モーダルにはしない)ウィンドウ。owner を閉じる(隠す)と、このウィンドウも隠れる。
    Window(Window* owner, const std::string& title, float width = 640, float height = 480);
    ~Window();

    Property<std::string> title;
    /// 背景の色(既定はテーマの WINDOW。ポップアップは POPUP)。
    Property<ThemeColor> background;
    /// フォーカスのあるウィジェット(無ければ nullptr)。
    ReadOnlyProperty<Widget*> focused_widget;
    /// 上端のメニューバー(docs/adr/0027)。代入するとウィンドウの子になり、レイアウトの領域はその下になる。前のものは破棄する。
    Property<MenuBar*> menu_bar;
    /// メニューバーの下のツールバー・下端のステータスバー(docs/adr/0030)。menu_bar と同じく、代入すると子になり、前のものは破棄する。
    Property<ToolBar*> tool_bar;
    Property<StatusBar*> status_bar;

    /// 閉じる操作がされた(ウィンドウの閉じるボタンなど)。can_close を false にすると閉じない。
    Property<CloseHandler> on_close;
    /// 閉じたら(close。閉じるボタンも)破棄するか(既定 false)。必要なときに作る補助のウィンドウに使う。new で作ったウィンドウにだけ使う。
    /// Dialog では on_finish を呼んだ後に破棄する。
    Property<bool> destroy_on_close;

    /// 閉じる(隠す)。on_close は呼ばない。Application の決まり(main_window・auto_quit_enabled)で exec() を抜ける。
    /// destroy_on_close なら、この後に破棄する。
    virtual void close();
    /// 全体を描いて PNG に保存する(目視の確認・ヘッドレスのテスト用)。
    bool save_png(const std::string& path);
    /// 全体を描いて Bitmap にする(大きさはピクセル。scale は dp からピクセルへの倍率)。表示していなくても描く。
    Bitmap capture();

protected:
    /// ポップアップ(Popup)のための作り方。SDL3 のウィンドウは Popup::open で作る。
    explicit Window(Object* owner_object);

    void paint(Canvas& canvas) override;
    void arrange_children() override;
    Size compute_preferred_size() const override;
    Size compute_minimum_size() const override;

private:
    template <typename>
    friend struct detail::ImplAccess;
    class Impl;
    Impl* d_;
};

}  // namespace tokotachi

#endif
