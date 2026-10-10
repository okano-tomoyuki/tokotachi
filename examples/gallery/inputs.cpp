// 文字と値の入力: LineEdit・TextEdit・SpinBox・ComboBox・Slider・ProgressBar。
#include "gallery.hpp"

int main(int argc, char** argv)
{
    tokotachi::Application app(argc, argv);
    tokotachi::Window window("入力", 420, 330);
    tokotachi::GridLayout* grid = new tokotachi::GridLayout(&window);
    grid->margins = tokotachi::Margins(16);
    grid->row_spacing = 10;
    grid->column_spacing = 12;

    tokotachi::LineEdit* line_edit_name = new tokotachi::LineEdit(&window);
    line_edit_name->placeholder_text = "名前を入力";
    tokotachi::ComboBox* combo_box_city = new tokotachi::ComboBox(&window);
    combo_box_city->items = std::vector<std::string>{"東京", "大阪", "名古屋"};
    combo_box_city->current_index = 0;
    tokotachi::SpinBox* spin_box_age = new tokotachi::SpinBox(&window);
    spin_box_age->maximum = 150;
    spin_box_age->value = 30;
    tokotachi::TextEdit* text_edit_note = new tokotachi::TextEdit(&window, "複数行の入力欄。\n折り返しとスクロールに対応する。");

    // Slider と ProgressBar を同じ変数に結ぶと、片方を動かせばもう片方も揃う。
    tokotachi::Slider* slider_volume = new tokotachi::Slider(&window);
    tokotachi::ProgressBar* progress_bar_level = new tokotachi::ProgressBar(&window);
    progress_bar_level->text_visible = true;
    tokotachi::DoubleVariable* amount = new tokotachi::DoubleVariable(&window, 65);
    tokotachi::bind(slider_volume->value, amount->value);
    amount->on_change = [=](tokotachi::Object*) { progress_bar_level->value = amount->value.get(); };
    progress_bar_level->value = amount->value.get();

    const char* const LABELS[] = {"名前", "都市", "年齢", "メモ", "音量", "進み具合"};
    tokotachi::Widget* const FIELDS[] = {line_edit_name, combo_box_city, spin_box_age, text_edit_note, slider_volume, progress_bar_level};
    for (int i = 0; i < 6; ++i)
    {
        grid->add(new tokotachi::Label(&window, LABELS[i]), i, 0);
        grid->add(FIELDS[i], i, 1);
    }
    grid->set_column_stretch(1, 1);
    grid->set_row_stretch(3, 1);
    return run_gallery(app, window, argc, argv);
}
