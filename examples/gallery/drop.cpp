// ドラッグ&ドロップ: エクスプローラー・ファイルマネージャーからファイルや文字を落とす。
// 左の枠に落とすと内容を表示し、右の一覧に落とすとファイルを加える(一覧の中はドラッグで並べ替えられる)。
#include <tokotachi/core/drag.hpp>

#include "gallery.hpp"

int main(int argc, char** argv)
{
    tokotachi::Application app(argc, argv);
    tokotachi::Window* window_main = new tokotachi::Window("ドラッグ&ドロップ", 560, 260);
    tokotachi::BoxLayout* box = new tokotachi::BoxLayout(window_main);
    box->margins = tokotachi::Margins(12);
    box->orientation = tokotachi::Orientation::HORIZONTAL;

    // 落とす枠: ドラッグが来たら枠の色を変え、落としたものを表示する。
    tokotachi::PaintBox* paint_box_drop = new tokotachi::PaintBox(window_main);
    paint_box_drop->accepts_drops = true;
    paint_box_drop->minimum_size = tokotachi::Size(220, 0);
    bool hovering = false;
    std::string message = "ここにファイルや文字を\n落としてください";
    paint_box_drop->on_paint = [&](tokotachi::Object*, tokotachi::Canvas& c)
    {
        const tokotachi::Rect r = paint_box_drop->local_rect.get().adjusted(1, 1, 1, 1);
        const tokotachi::Color line = hovering ? tokotachi::Color::rgb(0x2563eb) : tokotachi::Color::rgb(0x9ca3af);
        if (hovering)
        {
            c.fill_round_rect(r, 8, tokotachi::Color::rgb(0x2563eb).with_alpha(24));
        }
        c.stroke_round_rect(r, 8, hovering ? 2.0f : 1.0f, line);
        const tokotachi::TextLayout text(message, tokotachi::Font(), paint_box_drop->display_scale, r.width - 24);
        c.draw_text(text, tokotachi::Point(r.x + 12, r.y + (r.height - text.height()) / 2),
                    tokotachi::Color::rgb(0x374151));
    };
    paint_box_drop->on_drag_move = [&](tokotachi::Object*, tokotachi::DragEvent& e)
    {
        // OS からのものは、落とすまで中身(ファイルか文字か)が分からないので、受け取ると答えておく
        e.accepted = e.data.from_outside() || !e.data.text.empty();
        if (e.accepted && !hovering)
        {
            hovering = true;
            paint_box_drop->invalidate();
        }
    };
    paint_box_drop->on_drag_leave = [&](tokotachi::Object*)
    {
        hovering = false;
        paint_box_drop->invalidate();
    };
    paint_box_drop->on_drop = [&](tokotachi::Object*, tokotachi::DragEvent& e)
    {
        hovering = false;
        if (!e.data.files.empty())
        {
            message = "ファイル " + std::to_string(e.data.files.size()) + " 個:\n";
            for (std::size_t i = 0; i < e.data.files.size(); ++i)
            {
                message += e.data.files[i] + "\n";
            }
        }
        else
        {
            message = "文字:\n" + e.data.text;
        }
        paint_box_drop->invalidate();
    };

    // 一覧: 落としたファイルを加える。中はドラッグで並べ替えられる。
    tokotachi::ListModel* files = new tokotachi::ListModel(window_main);
    tokotachi::Item* placeholder = files->add_item("(ファイルを落とすと加わります)");
    placeholder->enabled = false;
    tokotachi::ListView* list_view_files = new tokotachi::ListView(window_main);
    list_view_files->model = files;
    list_view_files->selection_mode = tokotachi::SelectionMode::MULTI;
    list_view_files->drag_enabled = true;
    list_view_files->drop_enabled = true;
    // モデルが受け取らないもの(ほかからのデータ)はハンドラが受け取る。落とした行の前に入れる
    list_view_files->on_drag_move = [](tokotachi::Object*, tokotachi::DragEvent& e) { e.accepted = e.data.from_outside(); };
    list_view_files->on_drop = [&](tokotachi::Object*, tokotachi::DragEvent& e)
    {
        if (placeholder && !e.data.files.empty())
        {
            files->remove_item(placeholder);
            placeholder = nullptr;
        }
        const tokotachi::ModelIndex at = list_view_files->index_at(e.pos);
        int row = at.is_valid() ? at.row() : static_cast<int>(files->items.get().size());
        for (std::size_t i = 0; i < e.data.files.size(); ++i)
        {
            files->insert_item(row++, e.data.files[i]);
        }
        e.accepted = !e.data.files.empty();
    };

    box->add(paint_box_drop, 1);
    box->add(list_view_files, 1);
    return run_gallery(app, window_main, argc, argv);
}
