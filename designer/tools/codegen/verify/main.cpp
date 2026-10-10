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
    check(window.search_edit->placeholder_text.get() == "検索する言葉", "string property");
    check(window.close_button->style.get() == tokotachi::ButtonStyle::SECONDARY, "enum property");
    check(window.view_combo->items.get().size() == 2 && window.view_combo->current_index.get() == 0,
          "array property before index");
    check(window.size_spin->maximum.get() == 1000 && window.size_spin->value.get() == 10, "range before value");
    check(window.size_label->color.get().is_role() &&
              window.size_label->color.get().role() == tokotachi::ColorRole::TEXT_SECONDARY,
          "theme color");
    check(window.open_action->shortcut.get() == tokotachi::Shortcut("Ctrl+O"), "shortcut");
    check(window.details_action->checkable.get() && window.details_action->checked.get(), "action flags");
    check(window.refresh_timer->interval.get() == 5000, "timer");
    // 子を入れた後に設定するプロパティ(Splitter::sizes)。区画が無いと効かない
    check(window.split->panes.get().size() == 2 && window.split->sizes.get().size() == 2, "splitter panes");
    check(window.split->panes.get()[0]->collapsible.get() && window.split->panes.get()[0]->stretch.get() == 0,
          "splitter pane properties");
    check(window.tabs->tabs.get().size() == 2 && !window.detail_tab->enabled.get(), "tabs");
    // メニュー・ツールバー・ステータスバー
    check(window.menu_bar.get() != nullptr && window.menu_bar.get()->menus.get().size() == 2, "menu bar");
    // actions は区切りとサブメニューを除く
    check(window.file_menu->actions.get() ==
              std::vector<tokotachi::Action*>{window.open_action, window.quit_action},
          "menu items");
    check(window.tool_bar.get() != nullptr && window.status_bar.get() != nullptr, "tool bar and status bar");
    check(window.status_bar.get()->text.get() == "準備ができました", "status bar text");
    check(window.count_label->parent.get() != nullptr, "status bar widget");
    // ハンドラ
    window.open_action->trigger();
    check(called("open_action_trigger"), "action handler");
    window.search_button->on_click.get()(window.search_button);
    check(called("search"), "button handler");
    window.refresh_timer->on_timer.get()(window.refresh_timer);
    check(called("refresh"), "timer handler");
    bool can_close = true;
    window.on_close.get()(&window, can_close);
    check(called("main_window_close"), "window close handler");
    window.tabs->on_change.get()(window.tabs);
    check(called("tabs_change"), "tab handler");

    // ダイアログ(Dialog の継承・ボタン・PlaceLayout・自由な配置・ScrollArea の中身・値の型)
    SettingsDialog* dialog = new SettingsDialog(&window);
    dialog->open();
    dialog->save_png(argc > 2 ? argv[2] : "settings_dialog.png");
    check(dialog->title.get() == "設定", "dialog title");
    check(dialog->minimum_size.get().width == 400, "size value");
    check(dialog->heading->font.get().size == 18, "font value");
    check(!dialog->heading->color.get().is_role() &&
              dialog->heading->color.get().color().r == 0x33 && dialog->heading->color.get().color().b == 0xCC,
          "color value");
    check(dialog->scroll->content.get() == dialog->badge_area && dialog->scroll->fit_width.get(),
          "scroll area content");
    const tokotachi::Rect area = dialog->badge_area->bounds;
    const tokotachi::Rect badge = dialog->badge->bounds;
    check(badge.x + badge.width / 2 > area.width / 2 - 1 && badge.x + badge.width / 2 < area.width / 2 + 1,
          "place layout (center)");
    const tokotachi::Rect corner = dialog->corner_check->bounds;
    check(corner.x + corner.width > area.width - 9 && corner.x + corner.width < area.width - 7 && corner.y == 8,
          "place layout (top right)");
    const tokotachi::Rect slider = dialog->zoom_slider->bounds;
    check(slider.x == 0 && slider.y == 8 && slider.width == 200 && slider.height == 32, "free placement");
    check(dialog->zoom_slider->maximum.get() == 200 && dialog->zoom_slider->value.get() == 100, "slider range");
    dialog->zoom_slider->on_change.get()(dialog->zoom_slider);
    check(called("zoom_change"), "dialog widget handler");
    dialog->on_finish.get()(dialog);
    check(called("settings_finish"), "dialog handler");
    std::printf("\n  ],\n  \"rects\": [\n");
    bool first = true;
    print_rects(&window, 0, 0, first);
    std::printf("\n  ]\n}\n");
    return g_failures == 0 ? 0 : 1;
}
