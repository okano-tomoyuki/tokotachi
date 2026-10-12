// インストールしたライブラリを、利用者のプロジェクトとして使えるか確かめる(docs/adr/0061)。
// 公開のヘッダだけを読み、部品を作り、派生させて描き、イベントループを回して終える。
#include <tokotachi/tokotachi.hpp>

#include <cstdio>
#include <cstring>

namespace
{

// 公開のクラスから派生し、仮想関数の上書きと標準の見た目を描く関数がライブラリの外から使えるか。
class BadgeButton : public tokotachi::Button
{
public:
    explicit BadgeButton(tokotachi::Widget* parent) : tokotachi::Button(parent, "badge"), painted(false) {}

    bool painted;

protected:
    void paint(tokotachi::Canvas& canvas) override
    {
        tokotachi::draw::button_face(canvas, local_rect, state());
        canvas.fill_ellipse(tokotachi::Rect(2, 2, 8, 8), tokotachi::Color::rgb(0xdc2626));
        painted = true;
    }
};

}  // namespace

int main(int argc, char** argv)
{
    if (std::strcmp(tokotachi::version(), TOKOTACHI_VERSION_STRING) != 0)
    {
        std::fprintf(stderr, "版が違う: ライブラリ %s、ヘッダ %s\n", tokotachi::version(), TOKOTACHI_VERSION_STRING);
        return 1;
    }

    tokotachi::Application app(argc, argv);
    tokotachi::Window window("install", 240, 120);
    BadgeButton* button_badge = new BadgeButton(&window);
    button_badge->bounds = tokotachi::Rect(16, 16, 120, 32);

    // テンプレート(ヘッダの中の実装)の結びつけも、ライブラリの外で使えるか。
    tokotachi::Variable<std::string> message(nullptr, "ok");
    tokotachi::Label* label_message = new tokotachi::Label(&window);
    label_message->bounds = tokotachi::Rect(16, 60, 120, 24);
    tokotachi::bind(label_message->text, message.value);

    tokotachi::Timer timer_quit;
    timer_quit.interval = 200;
    timer_quit.repeat = false;
    timer_quit.on_timeout = [&](tokotachi::Object*)
    {
        app.quit(button_badge->painted && label_message->text.get() == "ok" ? 0 : 2);
    };

    window.visible = true;
    timer_quit.enabled = true;
    int code = app.exec();
    std::printf("tokotachi %s: %s\n", tokotachi::version(), code == 0 ? "ok" : "失敗");
    return code;
}
