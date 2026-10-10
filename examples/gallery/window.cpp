// ウィンドウの部品: メニューバー・ツールバー・ステータスバー・タブ。メニューとツールバーは同じ Action を共有する。
#include "gallery.hpp"

int main(int argc, char** argv)
{
    tokotachi::Application app(argc, argv);
    tokotachi::Window window("ウィンドウ", 520, 300);

    // 操作は Action で表し、メニューとツールバーの両方に入れる(ショートカットはウィンドウ全体で効く)。
    tokotachi::Action* action_open = new tokotachi::Action(&window, "開く(&O)...");
    action_open->shortcut = "Ctrl+O";
    tokotachi::Action* action_save = new tokotachi::Action(&window, "保存(&S)");
    action_save->shortcut = "Ctrl+S";
    tokotachi::Action* action_bold = new tokotachi::Action(&window, "太字(&B)");
    action_bold->checkable = true;
    action_bold->checked = true;

    tokotachi::MenuBar* menu_bar = new tokotachi::MenuBar();
    tokotachi::Menu* menu_file = menu_bar->add_menu("ファイル(&F)");
    menu_file->add_action(action_open);
    menu_file->add_action(action_save);
    menu_file->add_separator();
    menu_file->add_item("終了(&X)", [&](tokotachi::Object*) { window.close(); });
    menu_bar->add_menu("書式(&O)")->add_action(action_bold);
    window.menu_bar = menu_bar;

    tokotachi::ToolBar* tool_bar = new tokotachi::ToolBar();
    tool_bar->add_action(action_open);
    tool_bar->add_action(action_save);
    tool_bar->add_separator();
    tool_bar->add_action(action_bold);
    window.tool_bar = tool_bar;

    tokotachi::StatusBar* status_bar = new tokotachi::StatusBar();
    status_bar->text = "準備完了";
    status_bar->add_widget(new tokotachi::Label(nullptr, "行 1, 列 1"));
    window.status_bar = status_bar;
    action_save->on_trigger = [=](tokotachi::Object*) { status_bar->show_message("保存した", 3000); };

    tokotachi::BoxLayout* box = new tokotachi::BoxLayout(&window);
    box->margins = tokotachi::Margins(12);
    tokotachi::TabWidget* tab_widget_main = new tokotachi::TabWidget(&window);
    tab_widget_main->add_tab(new tokotachi::TextEdit(nullptr, "タブの中身。ページごとに好きな部品を置ける。"), "メモ");
    tab_widget_main->add_tab(new tokotachi::Widget(), "設定");
    tab_widget_main->add_tab(new tokotachi::Widget(), "履歴");
    box->add(tab_widget_main, 1);
    return run_gallery(app, window, argc, argv);
}
