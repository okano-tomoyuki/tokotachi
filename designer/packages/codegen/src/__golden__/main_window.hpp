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
    tokotachi::Action* open_action;
    tokotachi::Action* clear_recent_action;
    tokotachi::Action* details_action;
    tokotachi::Action* quit_action;
    tokotachi::Timer* refresh_timer;
    tokotachi::Menu* file_menu;
    tokotachi::ComboBox* view_combo;
    tokotachi::Label* count_label;
    tokotachi::Widget* search_row;
    tokotachi::LineEdit* search_edit;
    tokotachi::Button* search_button;
    tokotachi::Splitter* split;
    tokotachi::TreeView* folder_tree;
    tokotachi::TabWidget* tabs;
    tokotachi::Tab* detail_tab;
    tokotachi::ListView* file_list;
    tokotachi::Widget* detail_page;
    tokotachi::Label* name_label;
    tokotachi::LineEdit* name_edit;
    tokotachi::Label* size_label;
    tokotachi::SpinBox* size_spin;
    tokotachi::Widget* button_row;
    tokotachi::CheckBox* hidden_check;
    tokotachi::Button* close_button;

    void open_action_trigger(tokotachi::Object* sender);
    void quit_action_trigger(tokotachi::Object* sender);
    void refresh(tokotachi::Object* sender);
    void main_window_close(tokotachi::Object* sender, bool& can_close);
    void search(tokotachi::Object* sender);
    void tabs_change(tokotachi::Object* sender);
    void file_list_activate(tokotachi::Object* sender);
    void close_button_click(tokotachi::Object* sender);
    // <tokotachi-designer:end id="declarations" hash="1c7f8e2e">

private:
    void tokotachi_setup_ui();
};

// <tokotachi-designer:begin id="tokotachi_namespace_end">
// <tokotachi-designer:end id="tokotachi_namespace_end" hash="811c9dc5">

// <tokotachi-designer:begin id="tokotachi_header_end">
#endif  // MAIN_WINDOW_HPP
// <tokotachi-designer:end id="tokotachi_header_end" hash="20de0197">
