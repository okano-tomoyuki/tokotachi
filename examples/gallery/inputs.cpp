// 文字と値の入力: LineEdit・TextEdit・SpinBox・ComboBox・Slider・ProgressBar。
// 項目名の「(&N)」などは Alt と一緒に押すキーで、押すとその項目の入力欄へ移る。
#include "gallery.hpp"

int main(int argc, char** argv)
{
    tokotachi::Application app(argc, argv);
    tokotachi::Window window("入力", 420, 380);
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
    spin_box_age->suffix = " 歳";  // 数値の後ろに単位
    tokotachi::TextEdit* text_edit_note = new tokotachi::TextEdit(&window, "複数行の入力欄。\n折り返しとスクロールに対応する。");

    // Slider と ProgressBar を同じ変数に結ぶと、片方を動かせばもう片方も揃う。
    tokotachi::Slider* slider_volume = new tokotachi::Slider(&window);
    slider_volume->tick_interval = 10;  // 10 ごとに目盛り
    tokotachi::ProgressBar* progress_bar_level = new tokotachi::ProgressBar(&window);
    progress_bar_level->text_visible = true;
    tokotachi::DoubleVariable* amount = new tokotachi::DoubleVariable(&window, 65);
    tokotachi::bind(slider_volume->value, amount->value);
    amount->on_change = [=](tokotachi::Object*) { progress_bar_level->value = amount->value.get(); };
    progress_bar_level->value = amount->value.get();
    // 全体の量が分からない処理: 棒の一部が動き続ける
    tokotachi::ProgressBar* progress_bar_busy = new tokotachi::ProgressBar(&window);
    progress_bar_busy->indeterminate = true;

    const char* const LABELS[] = {"名前(&N)", "都市(&C)", "年齢(&G)", "メモ(&M)", "音量(&V)", "進み具合", "処理中"};
    tokotachi::Widget* const FIELDS[] = {line_edit_name, combo_box_city, spin_box_age,     text_edit_note,
                                         slider_volume,  progress_bar_level, progress_bar_busy};
    for (int i = 0; i < 7; ++i)
    {
        // 入力欄を先に作ったので、ニーモニックで移る先(buddy)を指定する(指定しなければ、タブの順でラベルの次の部品へ移る)
        tokotachi::Label* label = new tokotachi::Label(&window, LABELS[i]);
        label->buddy = FIELDS[i];
        grid->add(label, i, 0);
        grid->add(FIELDS[i], i, 1);
    }
    grid->set_column_stretch(1, 1);
    grid->set_row_stretch(3, 1);
    return run_gallery(app, window, argc, argv);
}
