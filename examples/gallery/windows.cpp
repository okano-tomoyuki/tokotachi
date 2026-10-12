// 複数のウィンドウ(docs/adr/0062)。
// メインのウィンドウを閉じると終える。ツールのウィンドウはメインのウィンドウが所有し、その上に出る(閉じても隠すだけで、使い回す)。
// メモのウィンドウは開くたびに作り、閉じたら破棄する。
#include "gallery.hpp"

int main(int argc, char** argv)
{
    tokotachi::Application app(argc, argv);
    tokotachi::Window* window_main = new tokotachi::Window("複数のウィンドウ", 500, 200);

    // メインのウィンドウが所有する。メインのウィンドウを閉じると一緒に隠れ、破棄すると一緒に破棄される。
    tokotachi::Window* window_tool = new tokotachi::Window(window_main, "ツール", 240, 160);
    tokotachi::BoxLayout* tool_box = new tokotachi::BoxLayout(window_tool);
    tool_box->margins = tokotachi::Margins(12);
    tool_box->add(new tokotachi::CheckBox(window_tool, "グリッドを表示する"));
    tool_box->add(new tokotachi::CheckBox(window_tool, "ガイドに吸着する"));
    tool_box->add_stretch();

    tokotachi::BoxLayout* box = new tokotachi::BoxLayout(window_main);
    box->margins = tokotachi::Margins(16);
    box->add(new tokotachi::Label(window_main, "メインのウィンドウを閉じると、ほかのウィンドウが開いていても終わる。"));

    tokotachi::Button* button_tool = new tokotachi::Button(window_main, "ツールのウィンドウ(&T)");
    button_tool->on_click = [=](tokotachi::Object*) { window_tool->visible = true; };
    box->add(button_tool, 0, tokotachi::Align::START);

    tokotachi::IntVariable* note_count = new tokotachi::IntVariable(window_main, 0);
    tokotachi::Button* button_note = new tokotachi::Button(window_main, "新しいメモ(&N)");
    button_note->style = tokotachi::ButtonStyle::SECONDARY;
    button_note->on_click = [=](tokotachi::Object*)
    {
        // 所有者を渡さないウィンドウは Application が所有する。閉じたら破棄する。
        note_count->value = note_count->value + 1;
        tokotachi::Window* window_note = new tokotachi::Window("メモ " + std::to_string(note_count->value.get()), 320, 200);
        window_note->destroy_on_close = true;
        tokotachi::BoxLayout* note_box = new tokotachi::BoxLayout(window_note);
        note_box->margins = tokotachi::Margins(8);
        note_box->add(new tokotachi::TextEdit(window_note), 1);
        window_note->visible = true;
    };
    box->add(button_note, 0, tokotachi::Align::START);
    box->add_stretch();

    return run_gallery(app, window_main, argc, argv);
}
