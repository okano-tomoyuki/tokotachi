// ウィンドウの部品: メニューバー・ツールバー・ステータスバー・タブ。メニューとツールバーは同じ Action を共有する。
#include "gallery.hpp"

int main(int argc, char** argv)
{
    tokotachi::Application app(argc, argv);
    tokotachi::Window window("ウィンドウ", 520, 300);

    // 操作は Action で表し、メニューとツールバーの両方に入れる(ショートカットはウィンドウ全体で効く)。
    tokotachi::Action* open = new tokotachi::Action(&window, "開く(&O)...");
    open->shortcut = "Ctrl+O";
    tokotachi::Action* save = new tokotachi::Action(&window, "保存(&S)");
    save->shortcut = "Ctrl+S";
    tokotachi::Action* bold = new tokotachi::Action(&window, "太字(&B)");
    bold->checkable = true;
    bold->checked = true;

    tokotachi::MenuBar* bar = new tokotachi::MenuBar();
    tokotachi::Menu* file = bar->add_menu("ファイル(&F)");
    file->add_action(open);
    file->add_action(save);
    file->add_separator();
    file->add_item("終了(&X)", [&](tokotachi::Object*) { window.close(); });
    bar->add_menu("書式(&O)")->add_action(bold);
    window.menu_bar = bar;

    tokotachi::ToolBar* tools = new tokotachi::ToolBar();
    tools->add_action(open);
    tools->add_action(save);
    tools->add_separator();
    tools->add_action(bold);
    window.tool_bar = tools;

    tokotachi::StatusBar* status = new tokotachi::StatusBar();
    status->text = "準備完了";
    status->add_widget(new tokotachi::Label(nullptr, "行 1, 列 1"));
    window.status_bar = status;
    save->on_trigger = [=](tokotachi::Object*) { status->show_message("保存した", 3000); };

    tokotachi::BoxLayout* box = new tokotachi::BoxLayout(&window);
    box->margins = tokotachi::Margins(12);
    tokotachi::TabWidget* tabs = new tokotachi::TabWidget(&window);
    tabs->add_tab(new tokotachi::TextEdit(nullptr, "タブの中身。ページごとに好きな部品を置ける。"), "メモ");
    tabs->add_tab(new tokotachi::Widget(), "設定");
    tabs->add_tab(new tokotachi::Widget(), "履歴");
    box->add(tabs, 1);
    return run_gallery(app, window, argc, argv);
}
