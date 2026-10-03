// 公開のヘッダの確認(ビルドが通ればよい。docs/adr/0003、0013)。
// - C++20 でもコンパイルできる(CMake が CXX_STANDARD 20 でこのファイルをビルドする)。
// - OS のヘッダのマクロ(windows.h の DELETE・ERROR 等、Xlib.h の None・Bool 等)を先に読み込んでもコンパイルできる。
#if defined(_WIN32)
#include <windows.h>
#elif defined(__has_include)
#if __has_include(<X11/Xlib.h>)
#include <X11/Xlib.h>
#endif
#endif

#include <gui/gui.hpp>

namespace
{

// プロパティ・enum を実際に使い、マクロとの衝突がテンプレートの中に隠れないようにする。
void use_everything(gui::LineEdit* edit, gui::Button* button)
{
    gui::KeyEvent e(gui::Key::DEL, gui::MODIFIER_SHIFT, false, 0);
    (void)e;
    edit->text = std::string("a") + edit->text;
    edit->focus_policy = gui::FocusPolicy::TAB_AND_CLICK;
    edit->cursor = gui::MouseCursor::IBEAM;
    button->on_click = [](gui::Object*) {};
    gui::Paint p(gui::Color::rgb(0));
    p.type = gui::Paint::Type::LINEAR;
    gui::Path path;
    path.move_to(0, 0);
    (void)gui::Align::CENTER;
    (void)gui::Orientation::HORIZONTAL;
    (void)gui::MouseButton::NONE;
    (void)gui::LineCap::ROUND;
}

}  // namespace

void gui_header_check_dummy(gui::LineEdit* edit, gui::Button* button) { use_everything(edit, button); }
