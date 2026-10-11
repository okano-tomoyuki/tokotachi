// ボタン・チェックボックス・ラジオボタン・見出しの付いた枠。「(&S)」などは Alt と一緒に押すキー(ニーモニック)。
#include "gallery.hpp"

int main(int argc, char** argv)
{
    tokotachi::Application app(argc, argv);
    tokotachi::Window window("ボタン", 360, 240);
    tokotachi::BoxLayout* column = new tokotachi::BoxLayout(&window);
    column->margins = tokotachi::Margins(16);
    column->spacing = 10;

    tokotachi::Widget* row = new tokotachi::Widget(&window);
    tokotachi::BoxLayout* buttons = new tokotachi::BoxLayout(row, tokotachi::Orientation::HORIZONTAL);
    buttons->spacing = 8;
    tokotachi::Button* button_save = new tokotachi::Button(row, "保存(&S)");
    tokotachi::Button* button_cancel = new tokotachi::Button(row, "キャンセル");
    button_cancel->style = tokotachi::ButtonStyle::SECONDARY;  // 控えめな見た目
    tokotachi::Button* button_disabled = new tokotachi::Button(row, "無効");
    button_disabled->enabled = false;
    buttons->add(button_save);
    buttons->add(button_cancel);
    buttons->add(button_disabled);
    buttons->add_stretch();
    button_save->on_click = [](tokotachi::Object*) { /* 押された */ };

    tokotachi::CheckBox* check_box_agree = new tokotachi::CheckBox(&window, "規約に同意する(&A)");
    check_box_agree->checked = true;

    // ラジオボタンは、同じ変数に group_value を結んで 1 つのグループにする。見出しの付いた枠(GroupBox)にまとめる
    tokotachi::GroupBox* sizes = new tokotachi::GroupBox(&window, "大きさ(&Z)");
    tokotachi::BoxLayout* size_box = new tokotachi::BoxLayout(sizes, tokotachi::Orientation::HORIZONTAL);
    size_box->spacing = 16;
    tokotachi::IntVariable* size = new tokotachi::IntVariable(&window, 2);
    const char* const NAMES[] = {"小", "中", "大"};
    for (int i = 0; i < 3; ++i)
    {
        tokotachi::RadioButton* radio_button = new tokotachi::RadioButton(sizes, NAMES[i], i + 1);
        tokotachi::bind(radio_button->group_value, size->value);
        size_box->add(radio_button);
    }
    size_box->add_stretch();

    column->add(row);
    column->add(check_box_agree);
    column->add(sizes);
    column->add_stretch();
    return run_gallery(app, window, argc, argv);
}
