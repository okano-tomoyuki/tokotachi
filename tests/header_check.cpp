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

#include <tokotachi/tokotachi.hpp>

namespace
{

// プロパティ・enum を実際に使い、マクロとの衝突がテンプレートの中に隠れないようにする。
void use_everything(tokotachi::LineEdit* edit, tokotachi::Button* button)
{
    tokotachi::KeyEvent e(tokotachi::Key::DEL, tokotachi::MODIFIER_SHIFT, false, 0);
    (void)e;
    edit->text = std::string("a") + edit->text;
    edit->focus_policy = tokotachi::FocusPolicy::TAB_AND_CLICK;
    edit->cursor = tokotachi::MouseCursor::IBEAM;
    button->on_click = [](tokotachi::Object*) {};
    tokotachi::Paint p(tokotachi::Color::rgb(0));
    p.type = tokotachi::Paint::Type::LINEAR;
    tokotachi::Path path;
    path.move_to(0, 0);
    (void)tokotachi::Align::CENTER;
    (void)tokotachi::Orientation::HORIZONTAL;
    (void)tokotachi::MouseButton::NONE;
    (void)tokotachi::LineCap::ROUND;
}

}  // namespace

void tokotachi_header_check_dummy(tokotachi::LineEdit* edit, tokotachi::Button* button) { use_everything(edit, button); }
