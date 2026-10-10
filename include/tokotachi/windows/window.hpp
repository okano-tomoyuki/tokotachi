/// @file
/// トップレベルのウィンドウ。ウィジェットのツリーの根になる。
/// 表示は visible に true を代入する(初めは表示されていない)。
#ifndef TOKOTACHI_WINDOWS_WINDOW_HPP
#define TOKOTACHI_WINDOWS_WINDOW_HPP

#include <string>

#include "../graphics/theme.hpp"
#include "../widgets/widget.hpp"

namespace tokotachi
{

class MenuBar;
class StatusBar;
class ToolBar;

namespace detail
{
struct WindowImpl;
}

class Window : public Widget
{
public:
    /// 大きさは dp。Application を作った後に作る。
    explicit Window(const std::string& title, float width = 640, float height = 480);
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

    /// 閉じる(隠す)。on_close は呼ばない。表示されているウィンドウ(ポップアップを除く)が無くなるとアプリケーションを終える。
    virtual void close();
    /// 全体を描いて PNG に保存する(目視の確認・ヘッドレスのテスト用)。
    bool save_png(const std::string& path);

    detail::WindowImpl* impl() const { return d_; }

protected:
    /// ポップアップ(Popup)のための作り方。SDL3 のウィンドウは Popup::open で作る。
    explicit Window(Object* owner_object);

    void paint(Canvas& canvas) override;
    void arrange_children() override;
    void child_removed(Widget* child) override;
    Size compute_preferred_size() const override;
    Size compute_minimum_size() const override;

private:
    std::string get_title() const;
    void set_title(const std::string& v);
    ThemeColor get_background() const { return background_; }
    void set_background(const ThemeColor& v);
    Widget* get_focused_widget() const;
    MenuBar* get_menu_bar() const { return menu_bar_; }
    void set_menu_bar(MenuBar* const& v);
    ToolBar* get_tool_bar() const { return tool_bar_; }
    void set_tool_bar(ToolBar* const& v);
    StatusBar* get_status_bar() const { return status_bar_; }
    void set_status_bar(StatusBar* const& v);

    detail::WindowImpl* d_;
    ThemeColor background_;
    CloseHandler on_close_;
    MenuBar* menu_bar_;
    ToolBar* tool_bar_;
    StatusBar* status_bar_;
};

}  // namespace tokotachi

#endif
