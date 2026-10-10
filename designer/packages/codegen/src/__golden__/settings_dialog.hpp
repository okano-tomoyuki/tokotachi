// <tokotachi-designer:begin id="tokotachi_header_begin">
#ifndef SETTINGS_DIALOG_HPP
#define SETTINGS_DIALOG_HPP

#include <tokotachi/tokotachi.hpp>
// <tokotachi-designer:end id="tokotachi_header_begin" hash="790c3e9a">

// <tokotachi-designer:begin id="tokotachi_namespace_begin">
// <tokotachi-designer:end id="tokotachi_namespace_begin" hash="811c9dc5">

/// Window created with the Tokotachi designer (settings_dialog.ttui.json). Regions enclosed in markers are overwritten when regenerated.
class SettingsDialog : public tokotachi::Dialog
{
public:
    explicit SettingsDialog(tokotachi::Window* parent);

    // <tokotachi-designer:begin id="declarations">
    tokotachi::Label* heading;
    tokotachi::ScrollArea* scroll;
    tokotachi::Widget* badge_area;
    tokotachi::Label* badge;
    tokotachi::CheckBox* corner_check;
    tokotachi::Widget* free_area;
    tokotachi::Slider* zoom_slider;
    tokotachi::ProgressBar* zoom_bar;

    void settings_finish(tokotachi::Object* sender);
    void zoom_change(tokotachi::Object* sender);
    // <tokotachi-designer:end id="declarations" hash="c370c31d">

private:
    void tokotachi_setup_ui();
};

// <tokotachi-designer:begin id="tokotachi_namespace_end">
// <tokotachi-designer:end id="tokotachi_namespace_end" hash="811c9dc5">

// <tokotachi-designer:begin id="tokotachi_header_end">
#endif  // SETTINGS_DIALOG_HPP
// <tokotachi-designer:end id="tokotachi_header_end" hash="b3c5885f">
