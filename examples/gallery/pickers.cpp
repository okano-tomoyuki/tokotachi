// 日付と色: DateEdit(書式・暦のポップアップ)・Calendar・ColorButton(押すと ColorDialog を開く)。
// --screenshot ColorDialog の画像を撮るときは、引数に --color-dialog を加える。
#include <cstring>

#include "gallery.hpp"

int main(int argc, char** argv)
{
    tokotachi::Application app(argc, argv);
    tokotachi::Window window("日付と色", 520, 330);
    tokotachi::BoxLayout* row = new tokotachi::BoxLayout(&window, tokotachi::Orientation::HORIZONTAL);
    row->margins = tokotachi::Margins(16);
    row->spacing = 20;

    // 左: 入力欄と色のボタン
    tokotachi::Widget* widget_form = new tokotachi::Widget(&window);
    tokotachi::GridLayout* grid = new tokotachi::GridLayout(widget_form);
    grid->row_spacing = 10;
    grid->column_spacing = 12;
    tokotachi::DateEdit* date_edit_start = new tokotachi::DateEdit(widget_form);
    date_edit_start->date = tokotachi::Date(2026, 10, 11);
    tokotachi::DateEdit* date_edit_due = new tokotachi::DateEdit(widget_form);
    date_edit_due->format = "yyyy-MM-dd";  // 書式を変える
    date_edit_due->first_day_of_week = tokotachi::DayOfWeek::MONDAY;
    date_edit_due->placeholder_text = "未定";
    tokotachi::ColorButton* color_button_line = new tokotachi::ColorButton(widget_form, tokotachi::Color::rgb(0x2563EB));
    tokotachi::ColorButton* color_button_fill = new tokotachi::ColorButton(widget_form, tokotachi::Color::rgb(0x16A34A, 128));
    color_button_fill->alpha_enabled = true;  // 不透明度も選ぶ
    const char* const LABELS[] = {"開始日(&S)", "期限(&D)", "線の色(&L)", "塗りの色(&F)"};
    tokotachi::Widget* const FIELDS[] = {date_edit_start, date_edit_due, color_button_line, color_button_fill};
    for (int i = 0; i < 4; ++i)
    {
        tokotachi::Label* label = new tokotachi::Label(widget_form, LABELS[i]);
        label->buddy = FIELDS[i];
        grid->add(label, i, 0);
        grid->add(FIELDS[i], i, 1, 1, 1, tokotachi::Align::START);
    }
    tokotachi::Label* label_days = new tokotachi::Label(widget_form);
    grid->add(label_days, 4, 0, 1, 2);
    grid->set_row_stretch(5, 1);
    const auto update_days = [=](tokotachi::Object*)
    {
        const tokotachi::Date a = date_edit_start->date, b = date_edit_due->date;
        label_days->text = b.is_null() ? "期限は未定です" : "期限まで " + std::to_string(a.days_to(b)) + " 日";
    };
    date_edit_start->on_change = update_days;
    date_edit_due->on_change = update_days;
    update_days(nullptr);

    // 右: 暦。選んだ日を期限にする
    tokotachi::Calendar* calendar_due = new tokotachi::Calendar(&window);
    calendar_due->on_change = [=](tokotachi::Object*) { date_edit_due->date = calendar_due->date.get(); };
    calendar_due->date = tokotachi::Date(2026, 10, 25);

    row->add(widget_form, 1);
    row->add(calendar_due, 0, tokotachi::Align::START);

    // 画像を撮るときだけ: 色のダイアログを開いて、それを撮る
    tokotachi::Window* target = nullptr;
    for (int i = 1; i < argc; ++i)
    {
        if (std::strcmp(argv[i], "--color-dialog") == 0)
        {
            tokotachi::ColorDialog* color_dialog = new tokotachi::ColorDialog(&window);
            color_dialog->alpha_enabled = true;
            color_dialog->color = tokotachi::Color::rgb(0x2563EB, 200);
            window.visible = true;
            color_dialog->open();
            target = color_dialog;
        }
    }
    return run_gallery(app, window, argc, argv, target);
}
