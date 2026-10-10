// ボタン・チェックボックス・ラジオボタン。
#include "gallery.hpp"

int main(int argc, char** argv)
{
    tokotachi::Application app(argc, argv);
    tokotachi::Window window("ボタン", 360, 200);
    tokotachi::BoxLayout* column = new tokotachi::BoxLayout(&window);
    column->margins = tokotachi::Margins(16);
    column->spacing = 10;

    tokotachi::Widget* row = new tokotachi::Widget(&window);
    tokotachi::BoxLayout* buttons = new tokotachi::BoxLayout(row, tokotachi::Orientation::HORIZONTAL);
    buttons->spacing = 8;
    tokotachi::Button* save = new tokotachi::Button(row, "保存");
    tokotachi::Button* cancel = new tokotachi::Button(row, "キャンセル");
    cancel->style = tokotachi::ButtonStyle::SECONDARY;  // 控えめな見た目
    tokotachi::Button* disabled = new tokotachi::Button(row, "無効");
    disabled->enabled = false;
    buttons->add(save);
    buttons->add(cancel);
    buttons->add(disabled);
    buttons->add_stretch();
    save->on_click = [](tokotachi::Object*) { /* 押された */ };

    tokotachi::CheckBox* agree = new tokotachi::CheckBox(&window, "規約に同意する");
    agree->checked = true;

    // ラジオボタンは、同じ変数に group_value を結んで 1 つのグループにする。
    tokotachi::Widget* sizes = new tokotachi::Widget(&window);
    tokotachi::BoxLayout* size_box = new tokotachi::BoxLayout(sizes, tokotachi::Orientation::HORIZONTAL);
    size_box->spacing = 16;
    tokotachi::IntVariable* size = new tokotachi::IntVariable(&window, 2);
    const char* const NAMES[] = {"小", "中", "大"};
    for (int i = 0; i < 3; ++i)
    {
        tokotachi::RadioButton* radio = new tokotachi::RadioButton(sizes, NAMES[i], i + 1);
        tokotachi::bind(radio->group_value, size->value);
        size_box->add(radio);
    }
    size_box->add_stretch();

    column->add(row);
    column->add(agree);
    column->add(sizes);
    column->add_stretch();
    return run_gallery(app, window, argc, argv);
}
