// <tokotachi-designer:begin id="tokotachi_source_begin">
#include "settings_dialog.hpp"
// <tokotachi-designer:end id="tokotachi_source_begin" hash="4fa17596">

// <tokotachi-designer:begin id="tokotachi_namespace_begin">
// <tokotachi-designer:end id="tokotachi_namespace_begin" hash="811c9dc5">

SettingsDialog::SettingsDialog(tokotachi::Window* parent)
    : tokotachi::Dialog(parent, std::string())
{
    tokotachi_setup_ui();
}

// <tokotachi-designer:begin id="tokotachi_setup_ui">
// Creates the widgets, sets their properties, adds them to the layouts and connects the handlers (generated).
void SettingsDialog::tokotachi_setup_ui()
{
    minimum_size = tokotachi::Size(400, 0);
    title = "設定";
    on_finish = [this](tokotachi::Object* sender) { settings_finish(sender); };
    add_button(tokotachi::DialogButton::OK);
    add_button(tokotachi::DialogButton::CANCEL, "やめる");

    tokotachi::BoxLayout* window_layout = new tokotachi::BoxLayout(this);
    window_layout->margins = tokotachi::Margins(16);
    window_layout->spacing = 12;

    label_heading = new tokotachi::Label(this);
    label_heading->text = "表示の設定";
    label_heading->font = tokotachi::Font(18, true);
    label_heading->color = tokotachi::Color(0x33, 0x66, 0xCC);
    window_layout->add(label_heading);

    window_layout->add_spacing(4);

    panel_user = new LabeledEdit(this);
    panel_user->tooltip = "表示する名前";
    window_layout->add(panel_user);

    scroll_area_badges = new tokotachi::ScrollArea(this);
    scroll_area_badges->minimum_size = tokotachi::Size(0, 120);
    scroll_area_badges->fit_width = true;

    area_badges = new tokotachi::Widget(scroll_area_badges);
    area_badges->minimum_size = tokotachi::Size(200, 160);
    tokotachi::PlaceLayout* area_badges_layout = new tokotachi::PlaceLayout(area_badges);

    label_badge = new tokotachi::Label(area_badges);
    label_badge->text = "中央";
    {
        tokotachi::Placement placement;
        placement.relative_x = 0.5f;
        placement.relative_y = 0.5f;
        placement.anchor = tokotachi::Anchor::CENTER;
        area_badges_layout->add(label_badge, placement);
    }

    check_box_corner = new tokotachi::CheckBox(area_badges);
    check_box_corner->text = "右上";
    {
        tokotachi::Placement placement;
        placement.x = -8;
        placement.y = 8;
        placement.relative_x = 1;
        placement.anchor = tokotachi::Anchor::TOP_RIGHT;
        area_badges_layout->add(check_box_corner, placement);
    }

    scroll_area_badges->content = area_badges;

    window_layout->add(scroll_area_badges, 1);

    group_box_zoom = new tokotachi::GroupBox(this);
    group_box_zoom->minimum_size = tokotachi::Size(0, 76);
    group_box_zoom->title = "拡大率(&Z)";

    slider_zoom = new tokotachi::Slider(group_box_zoom);
    slider_zoom->x = 12;
    slider_zoom->y = 28;
    slider_zoom->width = 200;
    slider_zoom->height = 38;
    slider_zoom->maximum = 200;
    slider_zoom->value = 100;
    slider_zoom->tick_interval = 50;
    slider_zoom->on_change = [this](tokotachi::Object* sender) { slider_zoom_change(sender); };

    level_bar_zoom = new samples::LevelBar(group_box_zoom);
    level_bar_zoom->x = 224;
    level_bar_zoom->y = 36;
    level_bar_zoom->width = 128;
    level_bar_zoom->height = 16;
    level_bar_zoom->maximum = 200;
    level_bar_zoom->value = 100;
    level_bar_zoom->warning_level = 150;

    window_layout->add(group_box_zoom);
}
// <tokotachi-designer:end id="tokotachi_setup_ui" hash="dd7f483d">

// <tokotachi-designer:handler-stubs>
void SettingsDialog::settings_finish(tokotachi::Object* sender)
{
}

void SettingsDialog::slider_zoom_change(tokotachi::Object* sender)
{
}

// <tokotachi-designer:begin id="tokotachi_namespace_end">
// <tokotachi-designer:end id="tokotachi_namespace_end" hash="811c9dc5">
