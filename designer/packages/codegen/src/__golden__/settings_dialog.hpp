// <tokotachi-designer:begin id="tokotachi_header_begin">
#ifndef SETTINGS_DIALOG_HPP
#define SETTINGS_DIALOG_HPP

#include <tokotachi/tokotachi.hpp>
#include "parts/labeled_edit.hpp"
// <tokotachi-designer:end id="tokotachi_header_begin" hash="dc13bb56">

// <tokotachi-designer:begin id="tokotachi_namespace_begin">
// <tokotachi-designer:end id="tokotachi_namespace_begin" hash="811c9dc5">

/// Window created with the Tokotachi designer (settings_dialog.ttui.json). Regions enclosed in markers are overwritten when regenerated.
class SettingsDialog : public tokotachi::Dialog
{
public:
    explicit SettingsDialog(tokotachi::Window* parent);

    // <tokotachi-designer:begin id="declarations">
    tokotachi::Label* label_heading;
    LabeledEdit* panel_user;
    tokotachi::ScrollArea* scroll_area_badges;
    tokotachi::Widget* area_badges;
    tokotachi::Label* label_badge;
    tokotachi::CheckBox* check_box_corner;
    tokotachi::Widget* area_free;
    tokotachi::Slider* slider_zoom;
    tokotachi::ProgressBar* progress_bar_zoom;

    void settings_finish(tokotachi::Object* sender);
    void slider_zoom_change(tokotachi::Object* sender);
    // <tokotachi-designer:end id="declarations" hash="c22c8a8b">

private:
    void tokotachi_setup_ui();
};

// <tokotachi-designer:begin id="tokotachi_namespace_end">
// <tokotachi-designer:end id="tokotachi_namespace_end" hash="811c9dc5">

// <tokotachi-designer:begin id="tokotachi_header_end">
#endif  // SETTINGS_DIALOG_HPP
// <tokotachi-designer:end id="tokotachi_header_end" hash="b3c5885f">
