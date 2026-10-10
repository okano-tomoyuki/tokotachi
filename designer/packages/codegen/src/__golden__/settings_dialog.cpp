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

    heading = new tokotachi::Label(this);
    heading->text = "表示の設定";
    heading->font = tokotachi::Font(18);
    heading->color = tokotachi::Color(0x33, 0x66, 0xCC);
    window_layout->add(heading);

    window_layout->add_spacing(4);

    scroll = new tokotachi::ScrollArea(this);
    scroll->minimum_size = tokotachi::Size(0, 120);
    scroll->fit_width = true;

    badge_area = new tokotachi::Widget(scroll);
    badge_area->minimum_size = tokotachi::Size(200, 160);
    tokotachi::PlaceLayout* badge_area_layout = new tokotachi::PlaceLayout(badge_area);

    badge = new tokotachi::Label(badge_area);
    badge->text = "中央";
    {
        tokotachi::Placement placement;
        placement.relative_x = 0.5f;
        placement.relative_y = 0.5f;
        placement.anchor = tokotachi::Anchor::CENTER;
        badge_area_layout->add(badge, placement);
    }

    corner_check = new tokotachi::CheckBox(badge_area);
    corner_check->text = "右上";
    {
        tokotachi::Placement placement;
        placement.x = -8;
        placement.y = 8;
        placement.relative_x = 1;
        placement.anchor = tokotachi::Anchor::TOP_RIGHT;
        badge_area_layout->add(corner_check, placement);
    }

    scroll->content = badge_area;

    window_layout->add(scroll, 1);

    free_area = new tokotachi::Widget(this);
    free_area->minimum_size = tokotachi::Size(0, 48);

    zoom_slider = new tokotachi::Slider(free_area);
    zoom_slider->x = 0;
    zoom_slider->y = 8;
    zoom_slider->width = 200;
    zoom_slider->height = 32;
    zoom_slider->maximum = 200;
    zoom_slider->value = 100;
    zoom_slider->on_change = [this](tokotachi::Object* sender) { zoom_change(sender); };

    zoom_bar = new tokotachi::ProgressBar(free_area);
    zoom_bar->x = 216;
    zoom_bar->y = 16;
    zoom_bar->width = 160;
    zoom_bar->height = 16;
    zoom_bar->maximum = 200;
    zoom_bar->value = 100;

    window_layout->add(free_area);
}
// <tokotachi-designer:end id="tokotachi_setup_ui" hash="fa6eb65c">

// <tokotachi-designer:handler-stubs>
void SettingsDialog::settings_finish(tokotachi::Object* sender)
{
}

void SettingsDialog::zoom_change(tokotachi::Object* sender)
{
}

// <tokotachi-designer:begin id="tokotachi_namespace_end">
// <tokotachi-designer:end id="tokotachi_namespace_end" hash="811c9dc5">
