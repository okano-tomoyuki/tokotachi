// 見本(designer/samples の main_window.ttui.json・settings_dialog.ttui.json)から生成した C++ を Tokotachi でビルドして動かし、生成したコードが
// 定義ファイルのとおりに部品を組むかを確かめる(docs/designer/codegen-design.md §7。tools/codegen/verify-cpp.mts が使う)。
//
// verify-cpp.mts はハンドラの雛形の中身を verify_record("ハンドラの名前") に書き換えてからビルドする。
// 結果(確かめた項目と、各部品の位置と大きさ)を JSON で標準出力に出し、確かめた項目がすべて通れば 0 で終える。
#include <cstdio>
#include <set>
#include <string>
#include <vector>

#include "main_window.hpp"
#include "settings_dialog.hpp"

namespace
{

std::set<std::string> g_called;
int g_failures = 0;
bool g_first = true;

void check(bool ok, const char* what)
{
    std::printf("%s    {\"check\": \"%s\", \"ok\": %s}", g_first ? "" : ",\n", what, ok ? "true" : "false");
    g_first = false;
    if (!ok)
    {
        ++g_failures;
    }
}

bool called(const char* name)
{
    return g_called.count(name) > 0;
}

void print_rects(tokotachi::Widget* w, float ox, float oy, bool& first)
{
    const std::vector<tokotachi::Widget*> children = w->children;
    for (std::size_t i = 0; i < children.size(); ++i)
    {
        tokotachi::Widget* c = children[i];
        const tokotachi::Rect b = c->bounds;
        std::printf("%s    {\"x\": %.1f, \"y\": %.1f, \"width\": %.1f, \"height\": %.1f, \"visible\": %s}",
                    first ? "" : ",\n", ox + b.x, oy + b.y, b.width, b.height, c->visible.get() ? "true" : "false");
        first = false;
        print_rects(c, ox + b.x, oy + b.y, first);
    }
}

}  // namespace

void verify_record(const char* name)
{
    g_called.insert(name);
}

int main(int argc, char** argv)
{
    tokotachi::Application app(argc, argv);
    app.color_scheme = tokotachi::ColorScheme::LIGHT;
    MainWindow window;
    window.visible = true;
    // 1 回描いて配置を済ませる(画像は目で確かめるため)
    window.save_png(argc > 1 ? argv[1] : "main_window.png");

    std::printf("{\n  \"checks\": [\n");
    // プロパティ
    check(window.title.get() == "ファイルの整理", "window.title");
    check(window.width.get() == 720 && window.height.get() == 480, "window size");
    check(window.line_edit_search->placeholder_text.get() == "検索する言葉", "string property");
    check(window.button_close->style.get() == tokotachi::ButtonStyle::SECONDARY, "enum property");
    check(window.combo_box_view->items.get().size() == 2 && window.combo_box_view->current_index.get() == 0,
          "array property before index");
    check(window.spin_box_size->maximum.get() == 1000 && window.spin_box_size->value.get() == 10, "range before value");
    check(window.label_size->color.get().is_role() &&
              window.label_size->color.get().role() == tokotachi::ColorRole::TEXT_SECONDARY,
          "theme color");
    check(window.button_search->theme_override.get().radius() == 0 &&
              window.button_search->theme.get()[tokotachi::ColorRole::ACCENT] == tokotachi::Color(0x05, 0x96, 0x69) &&
              window.line_edit_search->theme.get()[tokotachi::ColorRole::ACCENT] != tokotachi::Color(0x05, 0x96, 0x69),
          "theme override");
    check(window.action_open->shortcut.get() == tokotachi::Shortcut("Ctrl+O"), "shortcut");
    check(window.action_open->icon.get().width() == 16 && window.action_open->icon.get().height() == 16,
          "embedded image");
    check(window.action_details->checkable.get() && window.action_details->checked.get(), "action flags");
    check(window.timer_refresh->interval.get() == 5000, "timer");
    // 子を入れた後に設定するプロパティ(Splitter::sizes)。区画が無いと効かない
    check(window.splitter_main->panes.get().size() == 2 && window.splitter_main->sizes.get().size() == 2, "splitter panes");
    check(window.splitter_main->panes.get()[0]->collapsible.get() && window.splitter_main->panes.get()[0]->stretch.get() == 0,
          "splitter pane properties");
    check(window.tab_widget_files->tabs.get().size() == 2 && !window.tab_detail->enabled.get(), "tabs");
    check(window.list_view_files->columns.get().size() == 3 &&
              window.list_view_files->columns.get()[0]->width.get() == 200 &&
              window.list_view_files->columns.get()[1]->alignment.get() == tokotachi::Align::END &&
              window.column_modified->title.get() == "更新日時",
          "list columns");
    // メニュー・ツールバー・ステータスバー
    check(window.menu_bar.get() != nullptr && window.menu_bar.get()->menus.get().size() == 2, "menu bar");
    // actions は区切りとサブメニューを除く
    check(window.menu_file->actions.get() ==
              std::vector<tokotachi::Action*>{window.action_open, window.action_quit},
          "menu items");
    check(window.tool_bar.get() != nullptr && window.status_bar.get() != nullptr, "tool bar and status bar");
    check(window.status_bar.get()->text.get() == "準備ができました", "status bar text");
    check(window.label_count->parent.get() != nullptr, "status bar widget");
    // ハンドラ
    window.action_open->trigger();
    check(called("action_open_trigger"), "action handler");
    window.button_search->on_click.get()(window.button_search);
    check(called("search"), "button handler");
    // ドラッグ&ドロップ(docs/adr/0047)のハンドラ
    check(window.list_view_files->drag_enabled.get() && window.list_view_files->accepts_drops.get(), "drag and drop properties");
    {
        tokotachi::DragData dropped;
        dropped.text = "x";
        tokotachi::DragEvent drop_event(tokotachi::Point(), dropped, tokotachi::MODIFIER_NONE, tokotachi::DropAction::COPY);
        window.list_view_files->on_drop.get()(window.list_view_files, drop_event);
        check(called("list_view_files_drop"), "drop handler");
    }
    window.timer_refresh->on_timer.get()(window.timer_refresh);
    check(called("refresh"), "timer handler");
    bool can_close = true;
    window.on_close.get()(&window, can_close);
    check(called("main_window_close"), "window close handler");
    window.tab_widget_files->on_change.get()(window.tab_widget_files);
    check(called("tab_widget_files_change"), "tab handler");
    // Variable との結びつけ(結んだ時点で Variable の値になり、その後も双方向に伝わる)
    check(window.line_edit_name->text.get() == "メモ.txt" && !window.check_box_hidden->checked.get(),
          "bound initial values");
    window.line_edit_name->text = "a.txt";
    check(window.file_name->value.get() == "a.txt" && called("file_name_change"), "binding to variable");
    window.show_hidden->value = true;
    check(window.check_box_hidden->checked.get(), "binding from variable");

    // ダイアログ(Dialog の継承・ボタン・PlaceLayout・自由な配置・ScrollArea の中身・値の型)
    SettingsDialog* dialog = new SettingsDialog(&window);
    dialog->open();
    dialog->save_png(argc > 2 ? argv[2] : "settings_dialog.png");
    check(dialog->title.get() == "設定", "dialog title");
    check(dialog->minimum_size.get().width == 400, "size value");
    check(dialog->label_heading->font.get().size == 18 && dialog->label_heading->font.get().bold, "font value");
    check(!dialog->label_heading->color.get().is_role() &&
              dialog->label_heading->color.get().color().r == 0x33 && dialog->label_heading->color.get().color().b == 0xCC,
          "color value");
    check(dialog->scroll_area_badges->content.get() == dialog->area_badges && dialog->scroll_area_badges->fit_width.get(),
          "scroll area content");
    const tokotachi::Rect area = dialog->area_badges->bounds;
    const tokotachi::Rect label_badge = dialog->label_badge->bounds;
    check(label_badge.x + label_badge.width / 2 > area.width / 2 - 1 && label_badge.x + label_badge.width / 2 < area.width / 2 + 1,
          "place layout (center)");
    const tokotachi::Rect corner = dialog->check_box_corner->bounds;
    check(corner.x + corner.width > area.width - 9 && corner.x + corner.width < area.width - 7 && corner.y == 8,
          "place layout (top right)");
    const tokotachi::Rect slider = dialog->slider_zoom->bounds;
    check(slider.x == 12 && slider.y == 28 && slider.width == 200 && slider.height == 38 &&
              dialog->slider_zoom->parent.get() == dialog->group_box_zoom,
          "free placement (group box)");
    check(dialog->slider_zoom->maximum.get() == 200 && dialog->slider_zoom->value.get() == 100, "slider range");
    dialog->slider_zoom->on_change.get()(dialog->slider_zoom);
    check(called("slider_zoom_change"), "dialog widget handler");
    dialog->on_finish.get()(dialog);
    check(called("settings_finish"), "dialog handler");
    // 部品ファイル(docs/adr/0043): 部品ファイルのクラスを置き、置いた側のプロパティを後から設定する
    check(dialog->panel_user->parent.get() == dialog && dialog->panel_user->tooltip.get() == "表示する名前",
          "component instance");
    check(dialog->panel_user->label_caption->text.get() == "名前" &&
              dialog->panel_user->line_edit_value->parent.get() == dialog->panel_user,
          "component content");
    check(dialog->panel_user->bounds.get().width > 300, "component in layout");
    dialog->panel_user->line_edit_value->on_change.get()(dialog->panel_user->line_edit_value);
    check(called("value_change"), "component handler");
    // 自作部品(docs/adr/0044): 自作のクラスを作り、基底と自作のプロパティを設定する
    samples::LevelBar* level_bar = dialog->level_bar_zoom;
    check(level_bar->maximum.get() == 200 && level_bar->warning_level.get() == 150 && !level_bar->is_warning(),
          "custom widget");
    std::printf("\n  ],\n  \"rects\": [\n");
    bool first = true;
    print_rects(&window, 0, 0, first);
    std::printf("\n  ]\n}\n");
    return g_failures == 0 ? 0 : 1;
}
