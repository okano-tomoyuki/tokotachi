/* C API のテスト(docs/adr/0041)。C99 としてコンパイルし、画面なし(SDL の offscreen)で動かす。 */
#include <tokotachi/tokotachi.h>

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int failures = 0;

#define CHECK(cond)                                                                                                    \
    do                                                                                                                 \
    {                                                                                                                  \
        if (!(cond))                                                                                                   \
        {                                                                                                              \
            fprintf(stderr, "%s:%d: CHECK(%s) failed\n", __FILE__, __LINE__, #cond);                                   \
            ++failures;                                                                                                \
        }                                                                                                              \
    } while (0)

/* 返された文字列を比べて解放する。 */
static int take_equal(char* s, const char* expected)
{
    const int ok = s && strcmp(s, expected) == 0;
    if (!ok)
    {
        fprintf(stderr, "  got \"%s\", expected \"%s\"\n", s ? s : "(null)", expected);
    }
    tokotachi_free(s);
    return ok;
}

/* ---- ハンドラ ---- */

static int clicks = 0;
static int destroyed = 0;

static void on_click(tokotachi_object* sender, void* user_data)
{
    (void)sender;
    clicks += *(int*)user_data;
}

static void count_destroy(void* user_data)
{
    (void)user_data;
    ++destroyed;
}

static void test_handlers(tokotachi_window* window)
{
    static int step = 1;
    tokotachi_button* button = tokotachi_button_new(window, "OK");
    clicks = 0;
    destroyed = 0;
    tokotachi_button_base_set_on_click(button, on_click, &step, count_destroy);
    /* Button の on_click は ButtonBase のもの。基底の関数にそのまま渡せる */
    tokotachi_check_box* check_box = tokotachi_check_box_new(window, "check");
    tokotachi_check_box_set_checked(check_box, true);
    CHECK(tokotachi_check_box_get_checked(check_box));

    /* 置き換えると前のものを解放する */
    tokotachi_button_base_set_on_click(button, on_click, &step, count_destroy);
    CHECK(destroyed == 1);
    /* Object を破棄するとハンドラも解放する */
    tokotachi_object_delete(button);
    CHECK(destroyed == 2);
    /* NULL を渡すと外し、user_data はすぐに解放する */
    tokotachi_check_box_set_on_change(check_box, NULL, &step, count_destroy);
    CHECK(destroyed == 3);
    tokotachi_object_delete(check_box);
}

/* ---- 失敗 ---- */

static int error_calls = 0;

static void on_error(const char* message, void* user_data)
{
    (void)user_data;
    CHECK(message != NULL);
    ++error_calls;
}

static void test_errors(tokotachi_window* window)
{
    tokotachi_clear_error();
    CHECK(tokotachi_last_error() == NULL);
    tokotachi_set_error_handler(on_error, NULL);
    /* Window は Label ではない */
    char* text = tokotachi_label_get_text(window);
    CHECK(text == NULL);
    CHECK(tokotachi_last_error() != NULL && strstr(tokotachi_last_error(), "tokotachi_label_get_text") != NULL);
    CHECK(error_calls == 1);
    /* NULL のハンドル */
    tokotachi_widget_set_x(NULL, 1);
    CHECK(error_calls == 2);
    /* 範囲の外の列挙 */
    tokotachi_label* label = tokotachi_label_new(window, "");
    tokotachi_label_set_alignment(label, (tokotachi_align)99);
    CHECK(error_calls == 3);
    CHECK(tokotachi_label_get_alignment(label) == TOKOTACHI_ALIGN_START);
    tokotachi_set_error_handler(NULL, NULL);
    tokotachi_clear_error();
    tokotachi_object_delete(label);
}

/* ---- プロパティ ---- */

static void test_properties(tokotachi_window* window)
{
    tokotachi_label* label = tokotachi_label_new(window, "こんにちは");
    CHECK(take_equal(tokotachi_label_get_text(label), "こんにちは"));
    tokotachi_label_set_text(label, "abc");
    CHECK(take_equal(tokotachi_label_get_text(label), "abc"));
    CHECK(strcmp(tokotachi_object_class_name(label), "Label") == 0);
    CHECK(tokotachi_widget_get_parent(label) == window);
    CHECK(tokotachi_object_get_owner(label) == window);

    tokotachi_rect r = {10, 20, 100, 30};
    tokotachi_widget_set_bounds(label, r);
    CHECK(tokotachi_widget_get_x(label) == 10 && tokotachi_widget_get_width(label) == 100);
    tokotachi_widget_set_y(label, 25);
    CHECK(tokotachi_widget_get_bounds(label).y == 25);

    tokotachi_theme_color color = {true, TOKOTACHI_COLOR_ROLE_TEXT_SECONDARY, {0, 0, 0, 0}};
    tokotachi_label_set_color(label, color);
    CHECK(tokotachi_label_get_color(label).is_role);
    CHECK(tokotachi_label_get_color(label).role == TOKOTACHI_COLOR_ROLE_TEXT_SECONDARY);
    color.is_role = false;
    color.color.r = 200;
    color.color.a = 255;
    tokotachi_label_set_color(label, color);
    CHECK(!tokotachi_label_get_color(label).is_role && tokotachi_label_get_color(label).color.r == 200);

    tokotachi_font font = {22};
    tokotachi_label_set_font(label, font);
    CHECK(tokotachi_label_get_font(label).size == 22);

    /* 子の並び(全体の数を返し、capacity 個まで書く) */
    tokotachi_widget* children[16];
    size_t n = tokotachi_widget_get_children(window, children, 16);
    CHECK(n >= 1);
    int found = 0;
    for (size_t i = 0; i < n && i < 16; ++i)
    {
        found |= children[i] == label;
    }
    CHECK(found);
    CHECK(tokotachi_widget_get_children(window, NULL, 0) == n);

    /* 文字列の並び */
    tokotachi_combo_box* combo = tokotachi_combo_box_new(window);
    const char* items[] = {"赤", "緑", "青"};
    tokotachi_combo_box_set_items(combo, items, 3);
    tokotachi_combo_box_set_current_index(combo, 1);
    CHECK(take_equal(tokotachi_combo_box_get_current_text(combo), "緑"));
    char** got = tokotachi_combo_box_get_items(combo);
    CHECK(got && strcmp(got[2], "青") == 0 && got[3] == NULL);
    tokotachi_strv_free(got);

    /* float の並び */
    tokotachi_splitter* splitter = tokotachi_splitter_new(window, TOKOTACHI_ORIENTATION_HORIZONTAL);
    tokotachi_splitter_add_widget(splitter, tokotachi_widget_new(NULL), 1);
    tokotachi_splitter_add_widget(splitter, tokotachi_widget_new(NULL), 1);
    CHECK(tokotachi_splitter_get_panes(splitter, NULL, 0) == 2);
    float sizes[2];
    CHECK(tokotachi_splitter_get_sizes(splitter, sizes, 2) == 2);

    /* ショートカット */
    tokotachi_action* action = tokotachi_action_new(window, "保存");
    tokotachi_action_set_shortcut(action, tokotachi_shortcut_parse("Ctrl+S"));
    CHECK(tokotachi_action_get_shortcut(action).key == TOKOTACHI_KEY_S);
    CHECK(tokotachi_action_get_shortcut(action).modifiers == TOKOTACHI_MODIFIER_CONTROL);
    CHECK(take_equal(tokotachi_shortcut_to_string(tokotachi_action_get_shortcut(action)), "Ctrl+S"));

    tokotachi_object_delete(label);
    tokotachi_object_delete(combo);
    tokotachi_object_delete(splitter);
    tokotachi_object_delete(action);
}

/* ---- 利用者のデータ ---- */

static int user_data_destroyed = 0;

static void on_user_data_destroyed(void* data)
{
    CHECK(data == &user_data_destroyed);
    ++user_data_destroyed;
}

static void test_user_data(void)
{
    tokotachi_window* window = tokotachi_window_new("user data", 200, 100);
    tokotachi_label* label = tokotachi_label_new(window, "x");
    tokotachi_object_set_user_data(label, &user_data_destroyed, on_user_data_destroyed);
    CHECK(tokotachi_object_get_user_data(label) == &user_data_destroyed);
    /* 所有者を破棄すると、所有されているものも破棄され、destroy が呼ばれる */
    tokotachi_object_delete(window);
    CHECK(user_data_destroyed == 1);
}

/* ---- 結びつけ ---- */

static void test_bind(tokotachi_window* window)
{
    tokotachi_variable* name = tokotachi_variable_new(window, TOKOTACHI_VARIABLE_TYPE_STRING);
    tokotachi_line_edit* edit = tokotachi_line_edit_new(window, "");
    tokotachi_label* label = tokotachi_label_new(window, "");
    CHECK(strcmp(tokotachi_object_class_name(name), "Variable") == 0);
    CHECK(tokotachi_variable_get_type(name) == TOKOTACHI_VARIABLE_TYPE_STRING);
    CHECK(tokotachi_bind(edit, "text", name, "value"));
    CHECK(tokotachi_bind(label, "text", name, "value"));
    tokotachi_variable_set_string(name, "ゆき");
    CHECK(take_equal(tokotachi_text_input_base_get_text(edit), "ゆき"));
    CHECK(take_equal(tokotachi_label_get_text(label), "ゆき"));
    tokotachi_text_input_base_set_text(edit, "はな");
    CHECK(take_equal(tokotachi_label_get_text(label), "はな"));
    CHECK(take_equal(tokotachi_variable_get_string(name), "はな"));

    /* 型が違う・名前が無い・読み取り専用へは結べない */
    tokotachi_variable* flag = tokotachi_variable_new(window, TOKOTACHI_VARIABLE_TYPE_BOOL);
    CHECK(!tokotachi_bind(label, "text", flag, "value"));
    CHECK(!tokotachi_bind(label, "no_such", name, "value"));
    CHECK(!tokotachi_bind(edit, "selected_text", name, "value"));
    tokotachi_variable_set_int(flag, 1); /* 型が違う */
    CHECK(tokotachi_last_error() != NULL);
    tokotachi_clear_error();

    /* 読み取り専用のプロパティからは一方向 */
    tokotachi_check_box* check_box = tokotachi_check_box_new(window, "");
    CHECK(tokotachi_bind(check_box, "checked", flag, "value"));
    tokotachi_variable_set_bool(flag, true);
    CHECK(tokotachi_check_box_get_checked(check_box));

    CHECK(tokotachi_unbind_all(label, "text"));
    tokotachi_variable_set_string(name, "つき");
    CHECK(take_equal(tokotachi_label_get_text(label), "はな"));
    CHECK(tokotachi_unbind(edit, "text", name, "value"));
    tokotachi_variable_set_string(name, "そら");
    CHECK(take_equal(tokotachi_text_input_base_get_text(edit), "つき"));

    tokotachi_object_delete(edit);
    tokotachi_object_delete(label);
    tokotachi_object_delete(check_box);
    tokotachi_object_delete(name);
    tokotachi_object_delete(flag);
}

/* ---- モデルとビュー ---- */

static const char* rows[] = {"りんご", "みかん", "ぶどう"};

static int custom_row_count(const tokotachi_model_index* parent, void* user_data)
{
    (void)user_data;
    return parent->row < 0 ? 3 : 0;
}

static const char* custom_text(const tokotachi_model_index* index, int column, void* user_data)
{
    (void)user_data;
    return column == 0 ? rows[index->row] : "";
}

static void test_models(tokotachi_window* window)
{
    tokotachi_list_model* model = tokotachi_list_model_new(window);
    tokotachi_item* a = tokotachi_list_model_add_item(model, "a");
    tokotachi_list_model_add_item(model, "b");
    const char* columns[] = {"1 KB"};
    tokotachi_item_set_column_texts(a, columns, 1);
    CHECK(take_equal(tokotachi_item_column_text(a, 1), "1 KB"));
    CHECK(strcmp(tokotachi_object_class_name(a), "Item") == 0);

    tokotachi_list_view* view = tokotachi_list_view_new(window);
    tokotachi_item_view_set_model(view, model);
    tokotachi_item_view_add_column(view, "名前", 200);
    tokotachi_item_view_add_column(view, "大きさ", 80);
    CHECK(tokotachi_item_view_get_columns(view, NULL, 0) == 2);
    tokotachi_model_index none = {-1, NULL};
    CHECK(tokotachi_item_model_row_count(model, none) == 2);
    tokotachi_model_index second = tokotachi_item_model_index(model, 1, none);
    CHECK(second.row == 1);
    CHECK(take_equal(tokotachi_item_model_text(model, second, 0), "b"));
    tokotachi_item_view_set_current(view, second);
    tokotachi_model_index selection[4];
    CHECK(tokotachi_item_view_get_selection(view, selection, 4) == 1);
    CHECK(selection[0].row == 1 && selection[0].id == second.id);
    CHECK(tokotachi_list_model_item_at(model, second) != a);

    tokotachi_tree_model* tree = tokotachi_tree_model_new(window);
    tokotachi_tree_item* root = tokotachi_tree_model_add_item(tree, "root");
    tokotachi_tree_item_add_child(root, "child");
    CHECK(tokotachi_tree_item_get_children(root, NULL, 0) == 1);
    tokotachi_model_index root_index = tokotachi_tree_model_index_of(tree, root);
    CHECK(tokotachi_item_model_row_count(tree, root_index) == 1);

    /* 関数で答えるモデル */
    tokotachi_model_callbacks callbacks;
    memset(&callbacks, 0, sizeof(callbacks));
    callbacks.row_count = custom_row_count;
    callbacks.text = custom_text;
    destroyed = 0;
    tokotachi_custom_model* custom = tokotachi_custom_model_new(window, &callbacks, NULL, count_destroy);
    CHECK(strcmp(tokotachi_object_class_name(custom), "CustomModel") == 0);
    CHECK(tokotachi_item_model_row_count(custom, none) == 3);
    tokotachi_model_index third = tokotachi_item_model_index(custom, 2, none);
    CHECK(take_equal(tokotachi_item_model_text(custom, third, 0), "ぶどう"));
    tokotachi_item_view_set_model(view, custom);
    tokotachi_custom_model_notify_reset(custom);
    tokotachi_object_delete(custom);
    CHECK(destroyed == 1);
    CHECK(tokotachi_item_view_get_model(view) == NULL);

    /* row_count・text の無いモデルは作れない */
    memset(&callbacks, 0, sizeof(callbacks));
    CHECK(tokotachi_custom_model_new(window, &callbacks, NULL, count_destroy) == NULL);
    CHECK(destroyed == 2);
    tokotachi_clear_error();

    tokotachi_object_delete(view);
    tokotachi_object_delete(model);
    tokotachi_object_delete(tree);
}

/* ---- 描画 ---- */

static int paints = 0;

static void on_paint(tokotachi_object* sender, tokotachi_canvas* canvas, void* user_data)
{
    (void)sender;
    (void)user_data;
    ++paints;
    tokotachi_rect r = {0, 0, 40, 40};
    tokotachi_color red = {255, 0, 0, 255};
    tokotachi_canvas_fill_rect(canvas, r, red);
}

static void test_drawing(void)
{
    tokotachi_window* window = tokotachi_window_new("drawing", 120, 80);
    tokotachi_paint_box* box = tokotachi_paint_box_new(window);
    tokotachi_rect r = {10, 10, 40, 40};
    tokotachi_widget_set_bounds(box, r);
    tokotachi_widget_set_on_paint(box, on_paint, NULL, NULL);
    tokotachi_widget_set_visible(window, true);
    tokotachi_bitmap* shot = tokotachi_window_capture(window);
    CHECK(shot != NULL);
    CHECK(paints >= 1);
    const float s = tokotachi_bitmap_scale(shot);
    tokotachi_color c = tokotachi_bitmap_pixel(shot, (int)(30 * s), (int)(30 * s));
    CHECK(c.r == 255 && c.g == 0 && c.b == 0);
    tokotachi_bitmap_free(shot);

    /* Bitmap に描く */
    tokotachi_bitmap* bitmap = tokotachi_bitmap_new(64, 32, 1);
    tokotachi_canvas* canvas = tokotachi_bitmap_canvas(bitmap);
    tokotachi_color white = {255, 255, 255, 255};
    tokotachi_bitmap_clear(bitmap, white);
    tokotachi_gradient_stop stops[2] = {{0, {0, 0, 255, 255}}, {1, {0, 255, 0, 255}}};
    tokotachi_paint paint;
    memset(&paint, 0, sizeof(paint));
    paint.type = TOKOTACHI_PAINT_TYPE_LINEAR;
    paint.p1.x = 64;
    paint.stops = stops;
    paint.stop_count = 2;
    tokotachi_rect all = {0, 0, 64, 32};
    tokotachi_canvas_fill_round_rect(canvas, all, 0, &paint);
    tokotachi_color left = tokotachi_bitmap_pixel(bitmap, 1, 16);
    tokotachi_color right = tokotachi_bitmap_pixel(bitmap, 62, 16);
    CHECK(left.b > 200 && right.g > 200);

    tokotachi_path* path = tokotachi_path_new();
    tokotachi_point center = {32, 16};
    tokotachi_path_add_ellipse(path, center, 10, 10);
    CHECK(!tokotachi_path_empty(path));
    tokotachi_paint black;
    memset(&black, 0, sizeof(black));
    black.color.a = 255;
    tokotachi_canvas_fill_path(canvas, path, &black);
    CHECK(tokotachi_bitmap_pixel(bitmap, 32, 16).r == 0 && tokotachi_bitmap_pixel(bitmap, 32, 16).g == 0);
    tokotachi_stroke_style style = {2, TOKOTACHI_LINE_CAP_ROUND, TOKOTACHI_LINE_JOIN_ROUND, NULL, 0, 0};
    tokotachi_canvas_stroke_path(canvas, path, &style, &black);
    tokotachi_path_free(path);

    tokotachi_font font = {14};
    tokotachi_text_layout* layout = tokotachi_text_layout_new("Tokotachi", font, 1, 0, TOKOTACHI_ALIGN_START);
    CHECK(tokotachi_text_layout_width(layout) > 10);
    CHECK(tokotachi_text_layout_line_count(layout) == 1);
    tokotachi_point origin = {0, 0};
    tokotachi_canvas_draw_text_layout(canvas, layout, origin, black.color);
    tokotachi_text_layout_free(layout);

    /* Image */
    tokotachi_image* image = tokotachi_bitmap_to_image(bitmap);
    CHECK(tokotachi_image_width(image) == 64 && tokotachi_image_height(image) == 32);
    tokotachi_image* copy = tokotachi_image_copy(image);
    CHECK(tokotachi_image_equal(image, copy));
    tokotachi_action* action = tokotachi_action_new(window, "a");
    tokotachi_action_set_icon(action, image);
    tokotachi_image* icon = tokotachi_action_get_icon(action);
    CHECK(tokotachi_image_equal(icon, image));
    tokotachi_image_free(icon);
    tokotachi_image_free(copy);
    tokotachi_image_free(image);
    tokotachi_color pixels[4] = {{255, 0, 0, 255}, {0, 255, 0, 255}, {0, 0, 255, 255}, {0, 0, 0, 0}};
    image = tokotachi_image_from_pixels(2, 2, pixels, 4);
    CHECK(tokotachi_image_pixel(image, 1, 0).g == 255);
    tokotachi_image_free(image);
    CHECK(tokotachi_image_from_pixels(2, 2, pixels, 3) == NULL);
    CHECK(tokotachi_image_load("/no/such/file.png") == NULL);

    tokotachi_bitmap_free(bitmap);
    tokotachi_object_delete(window);
}

/* ---- レイアウト ---- */

static void test_layout(void)
{
    tokotachi_window* window = tokotachi_window_new("layout", 300, 200);
    tokotachi_box_layout* box = tokotachi_box_layout_new(window, TOKOTACHI_ORIENTATION_VERTICAL);
    tokotachi_margins m = {10, 10, 10, 10};
    tokotachi_layout_set_margins(box, m);
    CHECK(tokotachi_widget_get_layout(window) == box);
    tokotachi_label* a = tokotachi_label_new(window, "a");
    tokotachi_label* b = tokotachi_label_new(window, "b");
    tokotachi_box_layout_add(box, a, 0, TOKOTACHI_ALIGN_FILL);
    tokotachi_box_layout_add(box, b, 1, TOKOTACHI_ALIGN_FILL);
    tokotachi_widget_apply_layout(window);
    CHECK(tokotachi_widget_get_x(a) == 10);
    CHECK(tokotachi_widget_get_y(b) > tokotachi_widget_get_y(a));
    CHECK(fabsf(tokotachi_widget_get_width(a) - 280) < 0.01f);
    tokotachi_object_delete(window);
}

/* ---- イベントループ・post・Task・Timer ---- */

static tokotachi_application* app = NULL;
static int posted = 0;
static int task_progress_seen = 0;

static void on_posted(void* user_data)
{
    (void)user_data;
    ++posted;
}

static void work(tokotachi_task_context* context, void* user_data)
{
    (void)user_data;
    tokotachi_task_context_report_progress(context, 50);
    tokotachi_task_context_fail(context, "失敗しました");
}

static void on_progress(tokotachi_object* sender, void* user_data)
{
    (void)sender;
    (void)user_data;
    task_progress_seen = 1;
}

static void on_finish(tokotachi_object* sender, void* user_data)
{
    (void)user_data;
    CHECK(take_equal(tokotachi_task_get_error(sender), "失敗しました"));
    CHECK(!tokotachi_task_get_running(sender));
    tokotachi_application_quit(app, 7);
}

static void on_timeout(tokotachi_object* sender, void* user_data)
{
    (void)sender;
    (void)user_data;
    fprintf(stderr, "the event loop did not finish in time\n");
    ++failures;
    tokotachi_application_quit(app, 1);
}

static void test_event_loop(void)
{
    tokotachi_window* window = tokotachi_window_new("loop", 100, 100);
    tokotachi_widget_set_visible(window, true);
    CHECK(tokotachi_is_main_thread());
    destroyed = 0;
    CHECK(tokotachi_post(on_posted, NULL, count_destroy));
    tokotachi_task* task = tokotachi_task_new(window);
    tokotachi_task_set_on_progress(task, on_progress, NULL, NULL);
    tokotachi_task_set_on_finish(task, on_finish, NULL, NULL);
    CHECK(tokotachi_task_start(task, work, NULL, count_destroy));
    tokotachi_timer* timer = tokotachi_timer_new(window);
    tokotachi_timer_set_interval(timer, 5000);
    tokotachi_timer_set_on_timer(timer, on_timeout, NULL, NULL);
    tokotachi_timer_set_enabled(timer, true);
    CHECK(tokotachi_application_exec(app) == 7);
    CHECK(posted == 1);
    CHECK(destroyed == 2); /* post と Task の user_data */
    (void)task_progress_seen; /* 間引かれて届かないこともある */
    tokotachi_object_delete(window);
}

int main(int argc, char** argv)
{
    CHECK(strcmp(tokotachi_version(), "0.0.0") != 0);
    CHECK(tokotachi_struct_size("tokotachi_rect") == sizeof(tokotachi_rect));
    CHECK(tokotachi_struct_size("no_such") == 0);
    CHECK(tokotachi_application_instance() == NULL);
    app = tokotachi_application_new(argc, argv);
    CHECK(app != NULL);
    CHECK(tokotachi_application_instance() == app);

    tokotachi_window* window = tokotachi_window_new("test", 400, 300);
    CHECK(strcmp(tokotachi_object_class_name(window), "Window") == 0);
    CHECK(take_equal(tokotachi_window_get_title(window), "test"));

    test_properties(window);
    test_handlers(window);
    test_errors(window);
    test_bind(window);
    test_models(window);
    test_user_data();
    test_drawing();
    test_layout();
    tokotachi_object_delete(window);
    test_event_loop();

    tokotachi_object_delete(app);
    if (failures)
    {
        fprintf(stderr, "%d failure(s)\n", failures);
        return 1;
    }
    printf("all C API tests passed\n");
    return 0;
}
