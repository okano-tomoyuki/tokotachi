// <tokotachi-designer:begin id="tokotachi_source_begin">
#include "main_window.hpp"
// <tokotachi-designer:end id="tokotachi_source_begin" hash="27964efa">

// <tokotachi-designer:begin id="tokotachi_namespace_begin">
// <tokotachi-designer:end id="tokotachi_namespace_begin" hash="811c9dc5">

MainWindow::MainWindow()
    : tokotachi::Window(std::string())
{
    tokotachi_setup_ui();
}

// <tokotachi-designer:begin id="tokotachi_setup_ui">
// Creates the widgets, sets their properties, adds them to the layouts and connects the handlers (generated).
void MainWindow::tokotachi_setup_ui()
{
    open_action = new tokotachi::Action(this);
    open_action->text = "開く(&O)...";
    open_action->shortcut = tokotachi::Shortcut("Ctrl+O");
    open_action->on_trigger = [this](tokotachi::Object* sender) { open_action_trigger(sender); };

    clear_recent_action = new tokotachi::Action(this);
    clear_recent_action->text = "一覧を消す(&C)";

    details_action = new tokotachi::Action(this);
    details_action->text = "詳細を表示(&D)";
    details_action->checkable = true;
    details_action->checked = true;

    quit_action = new tokotachi::Action(this);
    quit_action->text = "終了(&X)";
    quit_action->shortcut = tokotachi::Shortcut("Ctrl+Q");
    quit_action->on_trigger = [this](tokotachi::Object* sender) { quit_action_trigger(sender); };

    refresh_timer = new tokotachi::Timer(this);
    refresh_timer->interval = 5000;
    refresh_timer->on_timer = [this](tokotachi::Object* sender) { refresh(sender); };

    width = 720;
    height = 480;
    title = "ファイルの整理";
    on_close = [this](tokotachi::Object* sender, bool& can_close) { main_window_close(sender, can_close); };

    tokotachi::MenuBar* window_menu_bar = new tokotachi::MenuBar();
    file_menu = window_menu_bar->add_menu("ファイル(&F)");
    file_menu->add_action(open_action);
    tokotachi::Menu* file_menu_menu_2 = file_menu->add_submenu("最近使ったファイル(&R)");
    file_menu_menu_2->add_action(clear_recent_action);
    file_menu->add_separator();
    file_menu->add_action(quit_action);
    tokotachi::Menu* window_menu_bar_menu_2 = window_menu_bar->add_menu("表示(&V)");
    window_menu_bar_menu_2->add_action(details_action);
    menu_bar = window_menu_bar;

    tokotachi::ToolBar* window_tool_bar = new tokotachi::ToolBar();
    window_tool_bar->button_style = tokotachi::ToolButtonStyle::TEXT_BESIDE_ICON;
    window_tool_bar->add_action(open_action);
    window_tool_bar->add_separator();
    view_combo = new tokotachi::ComboBox(window_tool_bar);
    view_combo->items = std::vector<std::string>{"一覧", "詳細"};
    view_combo->current_index = 0;
    window_tool_bar->add_widget(view_combo);
    tool_bar = window_tool_bar;

    tokotachi::StatusBar* window_status_bar = new tokotachi::StatusBar();
    window_status_bar->text = "準備ができました";
    count_label = new tokotachi::Label(window_status_bar);
    count_label->text = "0 件";
    window_status_bar->add_widget(count_label);
    status_bar = window_status_bar;

    tokotachi::BoxLayout* window_layout = new tokotachi::BoxLayout(this);
    window_layout->margins = tokotachi::Margins(12);

    search_row = new tokotachi::Widget(this);
    tokotachi::BoxLayout* search_row_layout = new tokotachi::BoxLayout(search_row);
    search_row_layout->orientation = tokotachi::Orientation::HORIZONTAL;

    search_edit = new tokotachi::LineEdit(search_row);
    search_edit->placeholder_text = "検索する言葉";
    search_edit->on_submit = [this](tokotachi::Object* sender) { search(sender); };
    search_row_layout->add(search_edit, 1);

    search_button = new tokotachi::Button(search_row);
    search_button->text = "検索";
    search_button->on_click = [this](tokotachi::Object* sender) { search(sender); };
    search_row_layout->add(search_button);

    window_layout->add(search_row);

    split = new tokotachi::Splitter(this);

    folder_tree = new tokotachi::TreeView(split);
    tokotachi::SplitterPane* split_pane_1 = split->add_widget(folder_tree, 0);
    split_pane_1->collapsible = true;

    tabs = new tokotachi::TabWidget(split);
    tabs->on_change = [this](tokotachi::Object* sender) { tabs_change(sender); };

    file_list = new tokotachi::ListView(tabs);
    file_list->sorting_enabled = true;
    file_list->selection_mode = tokotachi::SelectionMode::MULTI;
    file_list->on_activate = [this](tokotachi::Object* sender) { file_list_activate(sender); };
    tabs->add_tab(file_list, "ファイル");

    detail_page = new tokotachi::Widget(tabs);
    tokotachi::GridLayout* detail_page_layout = new tokotachi::GridLayout(detail_page);
    detail_page_layout->margins = tokotachi::Margins(8, 8, 8, 8);
    detail_page_layout->set_column_stretch(1, 1);

    name_label = new tokotachi::Label(detail_page);
    name_label->text = "名前";
    detail_page_layout->add(name_label, 0, 0);

    name_edit = new tokotachi::LineEdit(detail_page);
    detail_page_layout->add(name_edit, 0, 1);

    size_label = new tokotachi::Label(detail_page);
    size_label->text = "大きさ";
    size_label->color = tokotachi::ColorRole::TEXT_SECONDARY;
    detail_page_layout->add(size_label, 1, 0);

    size_spin = new tokotachi::SpinBox(detail_page);
    size_spin->maximum = 1000;
    size_spin->value = 10;
    detail_page_layout->add(size_spin, 1, 1, 1, 1, tokotachi::Align::START);

    detail_tab = tabs->add_tab(detail_page, "詳細");
    detail_tab->enabled = false;

    split->add_widget(tabs);

    split->sizes = std::vector<float>{180, 500};
    window_layout->add(split, 1);

    button_row = new tokotachi::Widget(this);
    tokotachi::BoxLayout* button_row_layout = new tokotachi::BoxLayout(button_row);
    button_row_layout->orientation = tokotachi::Orientation::HORIZONTAL;

    hidden_check = new tokotachi::CheckBox(button_row);
    hidden_check->text = "隠しファイルも表示する";
    button_row_layout->add(hidden_check);

    button_row_layout->add_stretch();

    close_button = new tokotachi::Button(button_row);
    close_button->text = "閉じる";
    close_button->style = tokotachi::ButtonStyle::SECONDARY;
    close_button->on_click = [this](tokotachi::Object* sender) { close_button_click(sender); };
    close_button->add_action(quit_action);
    button_row_layout->add(close_button);

    window_layout->add(button_row);
}
// <tokotachi-designer:end id="tokotachi_setup_ui" hash="490540ff">

// <tokotachi-designer:handler-stubs>
void MainWindow::open_action_trigger(tokotachi::Object* sender)
{
}

void MainWindow::quit_action_trigger(tokotachi::Object* sender)
{
}

void MainWindow::refresh(tokotachi::Object* sender)
{
}

void MainWindow::main_window_close(tokotachi::Object* sender, bool& can_close)
{
}

void MainWindow::search(tokotachi::Object* sender)
{
}

void MainWindow::tabs_change(tokotachi::Object* sender)
{
}

void MainWindow::file_list_activate(tokotachi::Object* sender)
{
}

void MainWindow::close_button_click(tokotachi::Object* sender)
{
}

// <tokotachi-designer:begin id="tokotachi_namespace_end">
// <tokotachi-designer:end id="tokotachi_namespace_end" hash="811c9dc5">
