// <tokotachi-designer:begin id="tokotachi_header_begin">
#ifndef MAIN_WINDOW_HPP
#define MAIN_WINDOW_HPP

#include <tokotachi/tokotachi.hpp>
// <tokotachi-designer:end id="tokotachi_header_begin" hash="a0e7fb92">

// <tokotachi-designer:begin id="tokotachi_namespace_begin">
// <tokotachi-designer:end id="tokotachi_namespace_begin" hash="811c9dc5">

/// Window created with the Tokotachi designer (main_window.ttui.json). Regions enclosed in markers are overwritten when regenerated.
class MainWindow : public tokotachi::Window
{
public:
    MainWindow();

    // <tokotachi-designer:begin id="declarations">
    tokotachi::Action* action_open;
    tokotachi::Action* action_clear_recent;
    tokotachi::Action* action_details;
    tokotachi::Action* action_quit;
    tokotachi::Timer* timer_refresh;
    tokotachi::Variable<std::string>* file_name;
    tokotachi::Variable<bool>* show_hidden;
    tokotachi::Menu* menu_file;
    tokotachi::ComboBox* combo_box_view;
    tokotachi::Label* label_count;
    tokotachi::Widget* row_search;
    tokotachi::LineEdit* line_edit_search;
    tokotachi::Button* button_search;
    tokotachi::Splitter* splitter_main;
    tokotachi::TreeView* tree_view_folders;
    tokotachi::TabWidget* tab_widget_files;
    tokotachi::Tab* tab_detail;
    tokotachi::ListView* list_view_files;
    tokotachi::Column* column_modified;
    tokotachi::Widget* page_detail;
    tokotachi::Label* label_name;
    tokotachi::LineEdit* line_edit_name;
    tokotachi::Label* label_size;
    tokotachi::SpinBox* spin_box_size;
    tokotachi::Widget* row_buttons;
    tokotachi::CheckBox* check_box_hidden;
    tokotachi::Button* button_close;

    void action_open_trigger(tokotachi::Object* sender);
    void action_quit_trigger(tokotachi::Object* sender);
    void refresh(tokotachi::Object* sender);
    void file_name_change(tokotachi::Object* sender);
    void main_window_close(tokotachi::Object* sender, bool& can_close);
    void search(tokotachi::Object* sender);
    void tab_widget_files_change(tokotachi::Object* sender);
    void list_view_files_drop(tokotachi::Object* sender, tokotachi::DragEvent& e);
    void list_view_files_activate(tokotachi::Object* sender);
    void button_close_click(tokotachi::Object* sender);
    // <tokotachi-designer:end id="declarations" hash="45fcb3a8">

private:
    void tokotachi_setup_ui();
};

// <tokotachi-designer:begin id="tokotachi_namespace_end">
// <tokotachi-designer:end id="tokotachi_namespace_end" hash="811c9dc5">

// <tokotachi-designer:begin id="tokotachi_header_end">
#endif  // MAIN_WINDOW_HPP
// <tokotachi-designer:end id="tokotachi_header_end" hash="20de0197">
