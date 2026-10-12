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
// Images embedded in main_window_images.cpp (generated).
namespace tokotachi_images_MainWindow
{
extern const unsigned char tokotachi_image_icons_open_png[];
extern const std::size_t tokotachi_image_icons_open_png_size;
}

// Creates the widgets, sets their properties, adds them to the layouts and connects the handlers (generated).
void MainWindow::tokotachi_setup_ui()
{
    action_open = new tokotachi::Action(this);
    action_open->text = "開く(&O)...";
    action_open->shortcut = tokotachi::Shortcut("Ctrl+O");
    action_open->icon = tokotachi::Image::load_from_data(tokotachi_images_MainWindow::tokotachi_image_icons_open_png, tokotachi_images_MainWindow::tokotachi_image_icons_open_png_size);
    action_open->on_trigger = [this](tokotachi::Object* sender) { action_open_trigger(sender); };

    action_clear_recent = new tokotachi::Action(this);
    action_clear_recent->text = "一覧を消す(&C)";

    action_details = new tokotachi::Action(this);
    action_details->text = "詳細を表示(&D)";
    action_details->checkable = true;
    action_details->checked = true;

    action_quit = new tokotachi::Action(this);
    action_quit->text = "終了(&X)";
    action_quit->shortcut = tokotachi::Shortcut("Ctrl+Q");
    action_quit->icon = tokotachi::Image::icon(tokotachi::Icon::LOG_OUT);
    action_quit->on_trigger = [this](tokotachi::Object* sender) { action_quit_trigger(sender); };

    timer_refresh = new tokotachi::Timer(this);
    timer_refresh->interval = 5000;
    timer_refresh->on_timer = [this](tokotachi::Object* sender) { refresh(sender); };

    file_name = new tokotachi::Variable<std::string>(this, "メモ.txt");
    file_name->on_change = [this](tokotachi::Object* sender) { file_name_change(sender); };

    show_hidden = new tokotachi::Variable<bool>(this);

    width = 720;
    height = 480;
    title = "ファイルの整理";
    on_close = [this](tokotachi::Object* sender, bool& can_close) { main_window_close(sender, can_close); };

    tokotachi::MenuBar* window_menu_bar = new tokotachi::MenuBar();
    menu_file = window_menu_bar->add_menu("ファイル(&F)");
    menu_file->add_action(action_open);
    tokotachi::Menu* menu_file_menu_2 = menu_file->add_submenu("最近使ったファイル(&R)");
    menu_file_menu_2->add_action(action_clear_recent);
    menu_file->add_separator();
    menu_file->add_action(action_quit);
    tokotachi::Menu* window_menu_bar_menu_2 = window_menu_bar->add_menu("表示(&V)");
    window_menu_bar_menu_2->add_action(action_details);
    menu_bar = window_menu_bar;

    tokotachi::ToolBar* window_tool_bar = new tokotachi::ToolBar();
    window_tool_bar->button_style = tokotachi::ToolButtonStyle::TEXT_BESIDE_ICON;
    window_tool_bar->add_action(action_open);
    window_tool_bar->add_separator();
    combo_box_view = new tokotachi::ComboBox(window_tool_bar);
    combo_box_view->items = std::vector<std::string>{"一覧", "詳細"};
    combo_box_view->current_index = 0;
    window_tool_bar->add_widget(combo_box_view);
    tool_bar = window_tool_bar;

    tokotachi::StatusBar* window_status_bar = new tokotachi::StatusBar();
    window_status_bar->text = "準備ができました";
    label_count = new tokotachi::Label(window_status_bar);
    label_count->text = "0 件";
    window_status_bar->add_widget(label_count);
    status_bar = window_status_bar;

    tokotachi::BoxLayout* window_layout = new tokotachi::BoxLayout(this);
    window_layout->margins = tokotachi::Margins(12);

    row_search = new tokotachi::Widget(this);
    tokotachi::BoxLayout* row_search_layout = new tokotachi::BoxLayout(row_search);
    row_search_layout->orientation = tokotachi::Orientation::HORIZONTAL;

    line_edit_search = new tokotachi::LineEdit(row_search);
    line_edit_search->placeholder_text = "検索する言葉";
    line_edit_search->on_submit = [this](tokotachi::Object* sender) { search(sender); };
    row_search_layout->add(line_edit_search, 1);

    button_search = new tokotachi::Button(row_search);
    button_search->theme_override = tokotachi::ThemeOverride().set(tokotachi::ColorRole::ACCENT, tokotachi::Color(0x05, 0x96, 0x69), tokotachi::Color(0x34, 0xD3, 0x99)).set(tokotachi::ColorRole::ON_ACCENT, tokotachi::Color(0xFF, 0xFF, 0xFF)).set_radius(0);
    button_search->text = "検索";
    button_search->on_click = [this](tokotachi::Object* sender) { search(sender); };
    row_search_layout->add(button_search);

    window_layout->add(row_search);

    splitter_main = new tokotachi::Splitter(this);

    tree_view_folders = new tokotachi::TreeView(splitter_main);
    tokotachi::SplitterPane* splitter_main_pane_1 = splitter_main->add_widget(tree_view_folders, 0);
    splitter_main_pane_1->collapsible = true;

    tab_widget_files = new tokotachi::TabWidget(splitter_main);
    tab_widget_files->on_change = [this](tokotachi::Object* sender) { tab_widget_files_change(sender); };

    list_view_files = new tokotachi::ListView(tab_widget_files);
    list_view_files->sorting_enabled = true;
    list_view_files->selection_mode = tokotachi::SelectionMode::MULTI;
    list_view_files->drag_enabled = true;
    list_view_files->drop_enabled = true;
    list_view_files->on_drop = [this](tokotachi::Object* sender, tokotachi::DragEvent& e) { list_view_files_drop(sender, e); };
    list_view_files->on_activate = [this](tokotachi::Object* sender) { list_view_files_activate(sender); };
    list_view_files->add_column("名前", 200);
    tokotachi::Column* list_view_files_column_2 = list_view_files->add_column("大きさ", 80);
    list_view_files_column_2->alignment = tokotachi::Align::END;
    column_modified = list_view_files->add_column("更新日時");
    tab_widget_files->add_tab(list_view_files, "ファイル");

    page_detail = new tokotachi::Widget(tab_widget_files);
    tokotachi::GridLayout* page_detail_layout = new tokotachi::GridLayout(page_detail);
    page_detail_layout->margins = tokotachi::Margins(8, 8, 8, 8);
    page_detail_layout->set_column_stretch(1, 1);

    label_name = new tokotachi::Label(page_detail);
    label_name->text = "名前(&N)";
    page_detail_layout->add(label_name, 0, 0);

    line_edit_name = new tokotachi::LineEdit(page_detail);
    tokotachi::bind(line_edit_name->text, file_name->value);
    page_detail_layout->add(line_edit_name, 0, 1);

    label_size = new tokotachi::Label(page_detail);
    label_size->text = "大きさ(&S)";
    label_size->color = tokotachi::ColorRole::TEXT_SECONDARY;
    page_detail_layout->add(label_size, 1, 0);

    spin_box_size = new tokotachi::SpinBox(page_detail);
    spin_box_size->maximum = 1000;
    spin_box_size->value = 10;
    page_detail_layout->add(spin_box_size, 1, 1, 1, 1, tokotachi::Align::START);

    tab_detail = tab_widget_files->add_tab(page_detail, "詳細");
    tab_detail->enabled = false;

    splitter_main->add_widget(tab_widget_files);

    splitter_main->sizes = std::vector<float>{180, 500};
    window_layout->add(splitter_main, 1);

    row_buttons = new tokotachi::Widget(this);
    tokotachi::BoxLayout* row_buttons_layout = new tokotachi::BoxLayout(row_buttons);
    row_buttons_layout->orientation = tokotachi::Orientation::HORIZONTAL;

    check_box_hidden = new tokotachi::CheckBox(row_buttons);
    check_box_hidden->text = "隠しファイルも表示する";
    tokotachi::bind(check_box_hidden->checked, show_hidden->value);
    row_buttons_layout->add(check_box_hidden);

    row_buttons_layout->add_stretch();

    button_close = new tokotachi::Button(row_buttons);
    button_close->text = "閉じる";
    button_close->style = tokotachi::ButtonStyle::SECONDARY;
    button_close->on_click = [this](tokotachi::Object* sender) { button_close_click(sender); };
    button_close->add_action(action_quit);
    row_buttons_layout->add(button_close);

    window_layout->add(row_buttons);

    label_name->buddy = line_edit_name;
    label_size->buddy = spin_box_size;
}
// <tokotachi-designer:end id="tokotachi_setup_ui" hash="ebecddad">

// <tokotachi-designer:handler-stubs>
void MainWindow::action_open_trigger(tokotachi::Object* sender)
{
}

void MainWindow::action_quit_trigger(tokotachi::Object* sender)
{
}

void MainWindow::refresh(tokotachi::Object* sender)
{
}

void MainWindow::file_name_change(tokotachi::Object* sender)
{
}

void MainWindow::main_window_close(tokotachi::Object* sender, bool& can_close)
{
}

void MainWindow::search(tokotachi::Object* sender)
{
}

void MainWindow::tab_widget_files_change(tokotachi::Object* sender)
{
}

void MainWindow::list_view_files_drop(tokotachi::Object* sender, tokotachi::DragEvent& e)
{
}

void MainWindow::list_view_files_activate(tokotachi::Object* sender)
{
}

void MainWindow::button_close_click(tokotachi::Object* sender)
{
}

// <tokotachi-designer:begin id="tokotachi_namespace_end">
// <tokotachi-designer:end id="tokotachi_namespace_end" hash="811c9dc5">
