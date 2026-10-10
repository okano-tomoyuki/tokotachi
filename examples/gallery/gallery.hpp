// サンプルの一覧(docs/guide/gallery.md)のプログラムの共通の部分。
//   <サンプル>                     ウィンドウを開く
//   <サンプル> --screenshot a.png  ライトのテーマで 1 回描いて PNG に保存して終える(リファレンスの画像を作るため)
#ifndef TOKOTACHI_EXAMPLES_GALLERY_HPP
#define TOKOTACHI_EXAMPLES_GALLERY_HPP

#include <string>

#include <tokotachi/tokotachi.hpp>

// --screenshot の保存先(無ければ空)。
inline std::string screenshot_path(int argc, char** argv)
{
    for (int i = 1; i + 1 < argc; ++i)
    {
        if (std::string(argv[i]) == "--screenshot")
        {
            return argv[i + 1];
        }
    }
    return std::string();
}

// window を表示して動かす。--screenshot なら target(既定は window)を PNG に保存して終える。
inline int run_gallery(tokotachi::Application& app, tokotachi::Window& window, int argc, char** argv,
                       tokotachi::Window* target = nullptr)
{
    const std::string shot = screenshot_path(argc, argv);
    if (!shot.empty())
    {
        app.color_scheme = tokotachi::ColorScheme::LIGHT;
    }
    window.visible = true;
    if (!shot.empty())
    {
        return (target ? target : &window)->save_png(shot) ? 0 : 1;
    }
    return app.exec();
}

#endif
