// トップレベルのウィンドウ。ウィジェットのツリーの根になる。
// 表示は visible に true を代入する(初めは表示されていない)。
#ifndef GUI_WINDOW_HPP
#define GUI_WINDOW_HPP

#include <string>

#include "widget.hpp"

namespace gui
{

namespace detail
{
struct WindowImpl;
}

class Window : public Widget
{
public:
    // 大きさは dp。Application を作った後に作る。
    explicit Window(const std::string& title, float width = 640, float height = 480);
    ~Window();

    Property<std::string> title;
    Property<Color> background;
    // フォーカスのあるウィジェット(無ければ nullptr)。
    ReadOnlyProperty<Widget*> focused_widget;

    // 閉じる操作がされた(ウィンドウの閉じるボタンなど)。can_close を false にすると閉じない。
    Property<CloseHandler> on_close;

    // 閉じる(隠す)。on_close は呼ばない。表示されているウィンドウ(ポップアップを除く)が無くなるとアプリケーションを終える。
    virtual void close();
    // 全体を描いて PNG に保存する(目視の確認・ヘッドレスのテスト用)。
    bool save_png(const std::string& path);

    detail::WindowImpl* impl() const { return d_; }

protected:
    // ポップアップ(Popup)のための作り方。SDL3 のウィンドウは Popup::open で作る。
    explicit Window(Object* owner_object);

    void paint(Canvas& canvas) override;

private:
    std::string get_title() const;
    void set_title(const std::string& v);
    Color get_background() const { return background_; }
    void set_background(const Color& v);
    Widget* get_focused_widget() const;

    detail::WindowImpl* d_;
    Color background_;
    CloseHandler on_close_;
};

}  // namespace gui

#endif
