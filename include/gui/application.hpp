// アプリケーション: SDL3 の初期化とイベントループ(docs/adr/0010)。1 つのプロセスに 1 つだけ作る。
#ifndef GUI_APPLICATION_HPP
#define GUI_APPLICATION_HPP

#include "property.hpp"

namespace gui
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

    // イベントループを回し、quit() で抜ける。戻り値は quit() に渡した値。
    int exec();
    void quit(int code = 0);

    detail::ApplicationImpl* impl() const { return d_; }

private:
    detail::ApplicationImpl* d_;
};

}  // namespace gui

#endif
