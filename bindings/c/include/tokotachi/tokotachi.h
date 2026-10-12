/* Tokotachi の C API(docs/adr/0041・docs/bindings/c-api.md)。
 * api.py から generate.py で作ったファイル。手で書き換えない(bindings/c/api.py・bindings/c/generate.py)。
 *
 * 決まり:
 * - 名前は tokotachi_<クラス>_<操作>。プロパティは _get_・_set_、ハンドラは _set_on_xxx、コンストラクタは _new。
 *   意味は C++ のヘッダ(include/tokotachi/ の各クラス)の説明のとおり。
 * - Object の派生(ウィジェット・レイアウト・Action など)は、どれも tokotachi_object へのポインタで指す
 *   (tokotachi_widget などはその別名)。基底のクラスの関数(tokotachi_widget_set_x など)に、そのまま派生のものを渡せる。
 *   型が違うものを渡すと何もせずに失敗する。
 * - 所有は C++ と同じ(docs/adr/0015)。所有者のあるものは所有者と一緒に破棄される。所有者の無いもの(Window・Application など)は
 *   tokotachi_object_delete で破棄する。Application は最後に破棄する。
 * - 文字列は UTF-8。返された char* は tokotachi_free で、char** は tokotachi_strv_free で解放する。
 * - 読み取りだけの並びは size_t f(self, out, capacity) の形で、全体の数を返し、out に capacity 個まで書く。
 * - ハンドラは関数・利用者のデータ・解放の関数(NULL でもよい)を渡す。ハンドラが外れる・置き換わる・Object が破棄されるときに
 *   destroy(user_data) を呼ぶ。fn に NULL を渡すとハンドラを外す。
 * - 失敗(型の違うハンドル・C++ の例外)は戻り値を既定の値(0・NULL・false)にし、tokotachi_last_error で説明を読める。
 * - Object・ウィジェットはメインスレッドだけで扱う(docs/adr/0029)。別のスレッドからは tokotachi_post で頼む。
 */
#ifndef TOKOTACHI_TOKOTACHI_H
#define TOKOTACHI_TOKOTACHI_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#if defined(_WIN32)
#if defined(TOKOTACHI_C_BUILD)
#define TOKOTACHI_C_API __declspec(dllexport)
#else
#define TOKOTACHI_C_API __declspec(dllimport)
#endif
#else
#define TOKOTACHI_C_API __attribute__((visibility("default")))
#endif

#ifdef __cplusplus
extern "C" {
#endif

/* C API の版(ライブラリの版と同じ) */
#define TOKOTACHI_C_VERSION_MAJOR 0
#define TOKOTACHI_C_VERSION_MINOR 1
#define TOKOTACHI_C_VERSION_PATCH 0

/* ---- 列挙(値は C++ の enum class と同じ順) ---- */
/* C++ の tokotachi::MouseButton */
typedef enum tokotachi_mouse_button
{
    TOKOTACHI_MOUSE_BUTTON_NONE = 0,
    TOKOTACHI_MOUSE_BUTTON_LEFT = 1,
    TOKOTACHI_MOUSE_BUTTON_MIDDLE = 2,
    TOKOTACHI_MOUSE_BUTTON_RIGHT = 3,
} tokotachi_mouse_button;

/* C++ の tokotachi::Key */
typedef enum tokotachi_key
{
    TOKOTACHI_KEY_UNKNOWN = 0,
    TOKOTACHI_KEY_BACKSPACE = 1,
    TOKOTACHI_KEY_TAB = 2,
    TOKOTACHI_KEY_RETURN = 3,
    TOKOTACHI_KEY_ESCAPE = 4,
    TOKOTACHI_KEY_SPACE = 5,
    TOKOTACHI_KEY_DEL = 6,
    TOKOTACHI_KEY_INSERT = 7,
    TOKOTACHI_KEY_LEFT = 8,
    TOKOTACHI_KEY_RIGHT = 9,
    TOKOTACHI_KEY_UP = 10,
    TOKOTACHI_KEY_DOWN = 11,
    TOKOTACHI_KEY_HOME = 12,
    TOKOTACHI_KEY_END = 13,
    TOKOTACHI_KEY_PAGE_UP = 14,
    TOKOTACHI_KEY_PAGE_DOWN = 15,
    TOKOTACHI_KEY_A = 16,
    TOKOTACHI_KEY_B = 17,
    TOKOTACHI_KEY_C = 18,
    TOKOTACHI_KEY_D = 19,
    TOKOTACHI_KEY_E = 20,
    TOKOTACHI_KEY_F = 21,
    TOKOTACHI_KEY_G = 22,
    TOKOTACHI_KEY_H = 23,
    TOKOTACHI_KEY_I = 24,
    TOKOTACHI_KEY_J = 25,
    TOKOTACHI_KEY_K = 26,
    TOKOTACHI_KEY_L = 27,
    TOKOTACHI_KEY_M = 28,
    TOKOTACHI_KEY_N = 29,
    TOKOTACHI_KEY_O = 30,
    TOKOTACHI_KEY_P = 31,
    TOKOTACHI_KEY_Q = 32,
    TOKOTACHI_KEY_R = 33,
    TOKOTACHI_KEY_S = 34,
    TOKOTACHI_KEY_T = 35,
    TOKOTACHI_KEY_U = 36,
    TOKOTACHI_KEY_V = 37,
    TOKOTACHI_KEY_W = 38,
    TOKOTACHI_KEY_X = 39,
    TOKOTACHI_KEY_Y = 40,
    TOKOTACHI_KEY_Z = 41,
    TOKOTACHI_KEY_DIGIT_0 = 42,
    TOKOTACHI_KEY_DIGIT_1 = 43,
    TOKOTACHI_KEY_DIGIT_2 = 44,
    TOKOTACHI_KEY_DIGIT_3 = 45,
    TOKOTACHI_KEY_DIGIT_4 = 46,
    TOKOTACHI_KEY_DIGIT_5 = 47,
    TOKOTACHI_KEY_DIGIT_6 = 48,
    TOKOTACHI_KEY_DIGIT_7 = 49,
    TOKOTACHI_KEY_DIGIT_8 = 50,
    TOKOTACHI_KEY_DIGIT_9 = 51,
    TOKOTACHI_KEY_F1 = 52,
    TOKOTACHI_KEY_F2 = 53,
    TOKOTACHI_KEY_F3 = 54,
    TOKOTACHI_KEY_F4 = 55,
    TOKOTACHI_KEY_F5 = 56,
    TOKOTACHI_KEY_F6 = 57,
    TOKOTACHI_KEY_F7 = 58,
    TOKOTACHI_KEY_F8 = 59,
    TOKOTACHI_KEY_F9 = 60,
    TOKOTACHI_KEY_F10 = 61,
    TOKOTACHI_KEY_F11 = 62,
    TOKOTACHI_KEY_F12 = 63,
    TOKOTACHI_KEY_SHIFT = 64,
    TOKOTACHI_KEY_CONTROL = 65,
    TOKOTACHI_KEY_ALT = 66,
    TOKOTACHI_KEY_META = 67,
} tokotachi_key;

/* C++ の tokotachi::FocusPolicy */
typedef enum tokotachi_focus_policy
{
    TOKOTACHI_FOCUS_POLICY_NONE = 0,
    TOKOTACHI_FOCUS_POLICY_TAB = 1,
    TOKOTACHI_FOCUS_POLICY_CLICK = 2,
    TOKOTACHI_FOCUS_POLICY_TAB_AND_CLICK = 3,
} tokotachi_focus_policy;

/* C++ の tokotachi::MouseCursor */
typedef enum tokotachi_mouse_cursor
{
    TOKOTACHI_MOUSE_CURSOR_ARROW = 0,
    TOKOTACHI_MOUSE_CURSOR_IBEAM = 1,
    TOKOTACHI_MOUSE_CURSOR_POINTING_HAND = 2,
    TOKOTACHI_MOUSE_CURSOR_WAIT = 3,
    TOKOTACHI_MOUSE_CURSOR_CROSSHAIR = 4,
    TOKOTACHI_MOUSE_CURSOR_SIZE_HORIZONTAL = 5,
    TOKOTACHI_MOUSE_CURSOR_SIZE_VERTICAL = 6,
} tokotachi_mouse_cursor;

/* C++ の tokotachi::Orientation */
typedef enum tokotachi_orientation
{
    TOKOTACHI_ORIENTATION_HORIZONTAL = 0,
    TOKOTACHI_ORIENTATION_VERTICAL = 1,
} tokotachi_orientation;

/* C++ の tokotachi::Align */
typedef enum tokotachi_align
{
    TOKOTACHI_ALIGN_START = 0,
    TOKOTACHI_ALIGN_CENTER = 1,
    TOKOTACHI_ALIGN_END = 2,
    TOKOTACHI_ALIGN_FILL = 3,
} tokotachi_align;

/* C++ の tokotachi::Anchor */
typedef enum tokotachi_anchor
{
    TOKOTACHI_ANCHOR_TOP_LEFT = 0,
    TOKOTACHI_ANCHOR_TOP = 1,
    TOKOTACHI_ANCHOR_TOP_RIGHT = 2,
    TOKOTACHI_ANCHOR_LEFT = 3,
    TOKOTACHI_ANCHOR_CENTER = 4,
    TOKOTACHI_ANCHOR_RIGHT = 5,
    TOKOTACHI_ANCHOR_BOTTOM_LEFT = 6,
    TOKOTACHI_ANCHOR_BOTTOM = 7,
    TOKOTACHI_ANCHOR_BOTTOM_RIGHT = 8,
} tokotachi_anchor;

/* C++ の tokotachi::ColorScheme */
typedef enum tokotachi_color_scheme
{
    TOKOTACHI_COLOR_SCHEME_SYSTEM = 0,
    TOKOTACHI_COLOR_SCHEME_LIGHT = 1,
    TOKOTACHI_COLOR_SCHEME_DARK = 2,
} tokotachi_color_scheme;

/* C++ の tokotachi::ColorRole */
typedef enum tokotachi_color_role
{
    TOKOTACHI_COLOR_ROLE_WINDOW = 0,
    TOKOTACHI_COLOR_ROLE_POPUP = 1,
    TOKOTACHI_COLOR_ROLE_TEXT = 2,
    TOKOTACHI_COLOR_ROLE_TEXT_SECONDARY = 3,
    TOKOTACHI_COLOR_ROLE_TEXT_DISABLED = 4,
    TOKOTACHI_COLOR_ROLE_PLACEHOLDER = 5,
    TOKOTACHI_COLOR_ROLE_ACCENT = 6,
    TOKOTACHI_COLOR_ROLE_ACCENT_HOVER = 7,
    TOKOTACHI_COLOR_ROLE_ACCENT_PRESSED = 8,
    TOKOTACHI_COLOR_ROLE_ACCENT_DISABLED = 9,
    TOKOTACHI_COLOR_ROLE_ON_ACCENT = 10,
    TOKOTACHI_COLOR_ROLE_FOCUS_RING = 11,
    TOKOTACHI_COLOR_ROLE_FIELD = 12,
    TOKOTACHI_COLOR_ROLE_FIELD_DISABLED = 13,
    TOKOTACHI_COLOR_ROLE_BORDER = 14,
    TOKOTACHI_COLOR_ROLE_SELECTION = 15,
    TOKOTACHI_COLOR_ROLE_SELECTION_INACTIVE = 16,
    TOKOTACHI_COLOR_ROLE_ON_SELECTION = 17,
    TOKOTACHI_COLOR_ROLE_PREEDIT_UNDERLINE = 18,
    TOKOTACHI_COLOR_ROLE_SCROLL_TRACK = 19,
    TOKOTACHI_COLOR_ROLE_SCROLL_THUMB = 20,
    TOKOTACHI_COLOR_ROLE_SCROLL_THUMB_ACTIVE = 21,
    TOKOTACHI_COLOR_ROLE_TOOLTIP = 22,
    TOKOTACHI_COLOR_ROLE_ON_TOOLTIP = 23,
} tokotachi_color_role;

/* C++ の tokotachi::ButtonStyle */
typedef enum tokotachi_button_style
{
    TOKOTACHI_BUTTON_STYLE_PRIMARY = 0,
    TOKOTACHI_BUTTON_STYLE_SECONDARY = 1,
} tokotachi_button_style;

/* C++ の tokotachi::DialogButton */
typedef enum tokotachi_dialog_button
{
    TOKOTACHI_DIALOG_BUTTON_NONE = 0,
    TOKOTACHI_DIALOG_BUTTON_OK = 1,
    TOKOTACHI_DIALOG_BUTTON_YES = 2,
    TOKOTACHI_DIALOG_BUTTON_SAVE = 3,
    TOKOTACHI_DIALOG_BUTTON_NO = 4,
    TOKOTACHI_DIALOG_BUTTON_DISCARD = 5,
    TOKOTACHI_DIALOG_BUTTON_CANCEL = 6,
    TOKOTACHI_DIALOG_BUTTON_CLOSE = 7,
} tokotachi_dialog_button;

/* C++ の tokotachi::FileDialogMode */
typedef enum tokotachi_file_dialog_mode
{
    TOKOTACHI_FILE_DIALOG_MODE_OPEN = 0,
    TOKOTACHI_FILE_DIALOG_MODE_SAVE = 1,
    TOKOTACHI_FILE_DIALOG_MODE_FOLDER = 2,
} tokotachi_file_dialog_mode;

/* C++ の tokotachi::MessageType */
typedef enum tokotachi_message_type
{
    TOKOTACHI_MESSAGE_TYPE_INFORMATION = 0,
    TOKOTACHI_MESSAGE_TYPE_QUESTION = 1,
    TOKOTACHI_MESSAGE_TYPE_WARNING = 2,
    TOKOTACHI_MESSAGE_TYPE_CRITICAL = 3,
} tokotachi_message_type;

/* C++ の tokotachi::PopupKind */
typedef enum tokotachi_popup_kind
{
    TOKOTACHI_POPUP_KIND_MENU = 0,
    TOKOTACHI_POPUP_KIND_TOOLTIP = 1,
} tokotachi_popup_kind;

/* C++ の tokotachi::PopupPlacement */
typedef enum tokotachi_popup_placement
{
    TOKOTACHI_POPUP_PLACEMENT_BELOW = 0,
    TOKOTACHI_POPUP_PLACEMENT_RIGHT = 1,
    TOKOTACHI_POPUP_PLACEMENT_AT = 2,
} tokotachi_popup_placement;

/* C++ の tokotachi::ToolButtonStyle */
typedef enum tokotachi_tool_button_style
{
    TOKOTACHI_TOOL_BUTTON_STYLE_ICON_ONLY = 0,
    TOKOTACHI_TOOL_BUTTON_STYLE_TEXT_BESIDE_ICON = 1,
    TOKOTACHI_TOOL_BUTTON_STYLE_TEXT_UNDER_ICON = 2,
} tokotachi_tool_button_style;

/* C++ の tokotachi::SelectionMode */
typedef enum tokotachi_selection_mode
{
    TOKOTACHI_SELECTION_MODE_NONE = 0,
    TOKOTACHI_SELECTION_MODE_SINGLE = 1,
    TOKOTACHI_SELECTION_MODE_MULTI = 2,
} tokotachi_selection_mode;

/* C++ の tokotachi::SortOrder */
typedef enum tokotachi_sort_order
{
    TOKOTACHI_SORT_ORDER_ASCENDING = 0,
    TOKOTACHI_SORT_ORDER_DESCENDING = 1,
} tokotachi_sort_order;

/* C++ の tokotachi::LineCap */
typedef enum tokotachi_line_cap
{
    TOKOTACHI_LINE_CAP_BUTT = 0,
    TOKOTACHI_LINE_CAP_ROUND = 1,
    TOKOTACHI_LINE_CAP_SQUARE = 2,
} tokotachi_line_cap;

/* C++ の tokotachi::LineJoin */
typedef enum tokotachi_line_join
{
    TOKOTACHI_LINE_JOIN_MITER = 0,
    TOKOTACHI_LINE_JOIN_ROUND = 1,
    TOKOTACHI_LINE_JOIN_BEVEL = 2,
} tokotachi_line_join;

/* 塗りの種類(C++ の Paint::Type) */
typedef enum tokotachi_paint_type
{
    TOKOTACHI_PAINT_TYPE_SOLID = 0,
    TOKOTACHI_PAINT_TYPE_LINEAR = 1,
    TOKOTACHI_PAINT_TYPE_RADIAL = 2,
} tokotachi_paint_type;

/* Variable の値の型(bool・int・float・double・std::string) */
typedef enum tokotachi_variable_type
{
    TOKOTACHI_VARIABLE_TYPE_BOOL = 0,
    TOKOTACHI_VARIABLE_TYPE_INT = 1,
    TOKOTACHI_VARIABLE_TYPE_FLOAT = 2,
    TOKOTACHI_VARIABLE_TYPE_DOUBLE = 3,
    TOKOTACHI_VARIABLE_TYPE_STRING = 4,
} tokotachi_variable_type;


/* ---- 修飾キー(ビットの組み合わせ) ---- */
typedef unsigned tokotachi_modifiers;
#define TOKOTACHI_MODIFIER_NONE 0u
#define TOKOTACHI_MODIFIER_SHIFT (1u << 0)
#define TOKOTACHI_MODIFIER_CONTROL (1u << 1)
#define TOKOTACHI_MODIFIER_ALT (1u << 2)
#define TOKOTACHI_MODIFIER_META (1u << 3)

/* ---- 値の型(C++ の同じ名前の型。座標はすべて dp) ---- */
typedef struct tokotachi_point
{
    float x, y;
} tokotachi_point;

typedef struct tokotachi_size
{
    float width, height;
} tokotachi_size;

typedef struct tokotachi_rect
{
    float x, y, width, height;
} tokotachi_rect;

typedef struct tokotachi_color
{
    uint8_t r, g, b, a;
} tokotachi_color;

typedef struct tokotachi_margins
{
    float left, top, right, bottom;
} tokotachi_margins;

typedef struct tokotachi_font
{
    float size; /* dp */
} tokotachi_font;

typedef struct tokotachi_shortcut
{
    tokotachi_key key;
    tokotachi_modifiers modifiers;
} tokotachi_shortcut;

/* テーマの役割(is_role が true)か、具体的な色 */
typedef struct tokotachi_theme_color
{
    bool is_role;
    tokotachi_color_role role;
    tokotachi_color color;
} tokotachi_theme_color;

/* モデルの中の項目の位置(row が負なら無効)。id はモデルが決める */
typedef struct tokotachi_model_index
{
    int row;
    void* id;
} tokotachi_model_index;

typedef struct tokotachi_placement
{
    float x, y;                           /* dp */
    float relative_x, relative_y;         /* 親の大きさに対する割合 */
    float width, height;                  /* dp */
    float relative_width, relative_height;
    tokotachi_anchor anchor;
} tokotachi_placement;

#define TOKOTACHI_COLOR_ROLE_COUNT 24

typedef struct tokotachi_theme
{
    tokotachi_color colors[TOKOTACHI_COLOR_ROLE_COUNT]; /* tokotachi_color_role の順 */
    bool dark_colors;
    float radius;
    float field_radius;
    float field_padding;
} tokotachi_theme;

/* ---- イベント(ハンドラが受け取る。accepted を false にすると親へ送る) ---- */
typedef struct tokotachi_mouse_event
{
    tokotachi_point pos;
    tokotachi_mouse_button button;
    tokotachi_modifiers modifiers;
    int clicks;
    bool accepted;
} tokotachi_mouse_event;

typedef struct tokotachi_key_event
{
    tokotachi_key key;
    tokotachi_modifiers modifiers;
    bool repeat;
    uint32_t native_key;
    bool accepted;
} tokotachi_key_event;

typedef struct tokotachi_wheel_event
{
    tokotachi_point pos;
    tokotachi_point delta;
    tokotachi_modifiers modifiers;
    bool accepted;
} tokotachi_wheel_event;

/* ---- 描画 ---- */
typedef struct tokotachi_gradient_stop
{
    float offset;
    tokotachi_color color;
} tokotachi_gradient_stop;

/* 塗り。LINEAR は p0 から p1 へ、RADIAL は p0 が中心・p1 が焦点 */
typedef struct tokotachi_paint
{
    tokotachi_paint_type type;
    tokotachi_color color;
    tokotachi_point p0, p1;
    float radius;
    const tokotachi_gradient_stop* stops;
    size_t stop_count;
} tokotachi_paint;

typedef struct tokotachi_stroke_style
{
    float width;
    tokotachi_line_cap cap;
    tokotachi_line_join join;
    const float* dashes; /* 線と隙間の長さを交互に(dash_count が 0 なら実線) */
    size_t dash_count;
    float dash_offset;
} tokotachi_stroke_style;

/* ---- ハンドル ---- */
/* Object の派生。下の別名はどれも同じ型 */
typedef struct tokotachi_object tokotachi_object;
typedef tokotachi_object tokotachi_application;
typedef tokotachi_object tokotachi_timer;
typedef tokotachi_object tokotachi_widget;
typedef tokotachi_object tokotachi_button_base;
typedef tokotachi_object tokotachi_button;
typedef tokotachi_object tokotachi_check_box;
typedef tokotachi_object tokotachi_radio_button;
typedef tokotachi_object tokotachi_combo_box;
typedef tokotachi_object tokotachi_label;
typedef tokotachi_object tokotachi_text_input_base;
typedef tokotachi_object tokotachi_line_edit;
typedef tokotachi_object tokotachi_text_edit;
typedef tokotachi_object tokotachi_paint_box;
typedef tokotachi_object tokotachi_progress_bar;
typedef tokotachi_object tokotachi_range_base;
typedef tokotachi_object tokotachi_slider;
typedef tokotachi_object tokotachi_spin_box;
typedef tokotachi_object tokotachi_scroll_area;
typedef tokotachi_object tokotachi_scroll_bar;
typedef tokotachi_object tokotachi_splitter_pane;
typedef tokotachi_object tokotachi_splitter;
typedef tokotachi_object tokotachi_tab;
typedef tokotachi_object tokotachi_tab_widget;
typedef tokotachi_object tokotachi_window;
typedef tokotachi_object tokotachi_dialog;
typedef tokotachi_object tokotachi_message_dialog;
typedef tokotachi_object tokotachi_file_dialog;
typedef tokotachi_object tokotachi_menu_bar;
typedef tokotachi_object tokotachi_popup;
typedef tokotachi_object tokotachi_status_bar;
typedef tokotachi_object tokotachi_tool_bar;
typedef tokotachi_object tokotachi_layout;
typedef tokotachi_object tokotachi_box_layout;
typedef tokotachi_object tokotachi_grid_layout;
typedef tokotachi_object tokotachi_place_layout;
typedef tokotachi_object tokotachi_action;
typedef tokotachi_object tokotachi_menu;
typedef tokotachi_object tokotachi_item_model;
typedef tokotachi_object tokotachi_item;
typedef tokotachi_object tokotachi_list_model;
typedef tokotachi_object tokotachi_tree_item;
typedef tokotachi_object tokotachi_tree_model;
typedef tokotachi_object tokotachi_column;
typedef tokotachi_object tokotachi_item_view;
typedef tokotachi_object tokotachi_list_view;
typedef tokotachi_object tokotachi_tree_view;
typedef tokotachi_object tokotachi_variable;
typedef tokotachi_object tokotachi_task;
typedef tokotachi_object tokotachi_custom_model;

/* 値・借りもの */
typedef struct tokotachi_image tokotachi_image;             /* 画像(tokotachi_image_free で解放する) */
typedef struct tokotachi_bitmap tokotachi_bitmap;           /* 書き換えられる画像(tokotachi_bitmap_free で解放する) */
typedef struct tokotachi_path tokotachi_path;               /* パス(tokotachi_path_free で解放する) */
typedef struct tokotachi_text_layout tokotachi_text_layout; /* 文字のレイアウト(tokotachi_text_layout_free で解放する) */
typedef struct tokotachi_canvas tokotachi_canvas;           /* 描く先(借りもの。解放しない) */
typedef struct tokotachi_task_context tokotachi_task_context; /* Task の仕事に渡す(借りもの) */

/* ---- 関数の型 ---- */
typedef void (*tokotachi_destroy_fn)(void* user_data);
typedef void (*tokotachi_callback_fn)(void* user_data);
typedef void (*tokotachi_error_fn)(const char* message, void* user_data);
typedef void (*tokotachi_notify_fn)(tokotachi_object* sender, void* user_data);
typedef void (*tokotachi_mouse_fn)(tokotachi_object* sender, tokotachi_mouse_event* e, void* user_data);
typedef void (*tokotachi_key_fn)(tokotachi_object* sender, tokotachi_key_event* e, void* user_data);
typedef void (*tokotachi_wheel_fn)(tokotachi_object* sender, tokotachi_wheel_event* e, void* user_data);
/* canvas はハンドラの中だけで使える */
typedef void (*tokotachi_paint_fn)(tokotachi_object* sender, tokotachi_canvas* canvas, void* user_data);
/* *can_close を false にすると閉じない */
typedef void (*tokotachi_close_fn)(tokotachi_object* sender, bool* can_close, void* user_data);
/* 別のスレッドで呼ぶ */
typedef void (*tokotachi_task_fn)(tokotachi_task_context* context, void* user_data);

/* 独自のモデル(tokotachi_custom_model_new)が中身を答える関数。NULL のものは既定の答え(C++ の ItemModel の既定の実装)。
 * index・parent はモデルの中の位置へのポインタ(呼んでいる間だけ有効)。 */
typedef struct tokotachi_model_callbacks
{
    /* parent の子の数(parent が無効なら一番上の数)。必須 */
    int (*row_count)(const tokotachi_model_index* parent, void* user_data);
    /* parent の row 番目の子の識別子(NULL なら一番上の行だけを識別子 NULL で返す一覧のモデル) */
    void* (*index_id)(int row, const tokotachi_model_index* parent, void* user_data);
    /* 親を *out に書いて true(一番上なら false) */
    bool (*parent_of)(const tokotachi_model_index* index, tokotachi_model_index* out, void* user_data);
    /* column 列目の文字。返した文字列は、このモデルの次の関数を呼ぶまで有効であればよい。必須 */
    const char* (*text)(const tokotachi_model_index* index, int column, void* user_data);
    /* 画像(NULL なら無し)。返したハンドルは、このモデルの次の関数を呼ぶまで有効であればよい */
    const tokotachi_image* (*icon)(const tokotachi_model_index* index, void* user_data);
    bool (*enabled)(const tokotachi_model_index* index, void* user_data);
    bool (*checkable)(const tokotachi_model_index* index, void* user_data);
    bool (*checked)(const tokotachi_model_index* index, void* user_data);
    void (*set_checked)(const tokotachi_model_index* index, bool checked, void* user_data);
    void (*sort)(int column, tokotachi_sort_order order, void* user_data);
    /* 並びが変わった後の index の今の位置を *out に書いて true */
    bool (*locate)(const tokotachi_model_index* index, tokotachi_model_index* out, void* user_data);
} tokotachi_model_callbacks;

/* ======== 全体 ======== */
/* 実行時に読み込まれたライブラリの版("0.1.0")。解放しない */
TOKOTACHI_C_API const char* tokotachi_version(void);
/* この API が返した文字列(char*)を解放する(NULL なら何もしない) */
TOKOTACHI_C_API void tokotachi_free(void* p);
/* この API が返した文字列の並び(NULL で終わる char**)を、中の文字列ごと解放する */
TOKOTACHI_C_API void tokotachi_strv_free(char** strv);
/* このスレッドで最後に起きた失敗の説明(無ければ NULL)。次の失敗か tokotachi_clear_error まで有効 */
TOKOTACHI_C_API const char* tokotachi_last_error(void);
TOKOTACHI_C_API void tokotachi_clear_error(void);
/* 失敗したときに、失敗したスレッドで handler を呼ぶ(NULL で外す)。バインディングが例外にするために使う */
TOKOTACHI_C_API void tokotachi_set_error_handler(tokotachi_error_fn handler, void* user_data);
/* 構造体の大きさ(name は "tokotachi_rect" など。知らない名前なら 0)。バインディングの確かめに使う */
TOKOTACHI_C_API size_t tokotachi_struct_size(const char* name);
/* Object を破棄する(所有しているものも一緒に破棄される)。NULL なら何もしない */
TOKOTACHI_C_API void tokotachi_object_delete(tokotachi_object* object);
/* 今のイベントの処理が終わった後に破棄する(自分のハンドラの中から自分を破棄するとき) */
TOKOTACHI_C_API void tokotachi_object_delete_later(tokotachi_object* object);
/* この API が知っている最も派生したクラスの名前("Label" など。解放しない) */
TOKOTACHI_C_API const char* tokotachi_object_class_name(tokotachi_object* object);
/* Object に利用者のデータを 1 つ付ける。Object が破棄されるとき・付け替えたときに destroy(data) を呼ぶ
 * (破棄の途中で呼ぶので、destroy の中で Object に触ってはいけない) */
TOKOTACHI_C_API void tokotachi_object_set_user_data(tokotachi_object* object, void* data, tokotachi_destroy_fn destroy);
TOKOTACHI_C_API void* tokotachi_object_get_user_data(tokotachi_object* object);
/* target のプロパティと source のプロパティを結ぶ(C++ の tokotachi::bind。docs/adr/0016)。名前で指す("text" など)。
 * 型が違う・名前が無い・target が読み取り専用なら false */
TOKOTACHI_C_API bool tokotachi_bind(tokotachi_object* target, const char* target_property, tokotachi_object* source, const char* source_property);
TOKOTACHI_C_API bool tokotachi_unbind(tokotachi_object* a, const char* a_property, tokotachi_object* b, const char* b_property);
TOKOTACHI_C_API bool tokotachi_unbind_all(tokotachi_object* object, const char* property);
TOKOTACHI_C_API bool tokotachi_is_main_thread(void);
/* どのスレッドからでも呼べる: fn(user_data) をメインスレッドで呼ぶよう頼む(docs/adr/0029)。
 * 呼んだ後・呼ばずに捨てたときに destroy(user_data) を呼ぶ。Application が無ければ頼まずに false */
TOKOTACHI_C_API bool tokotachi_post(tokotachi_callback_fn fn, void* user_data, tokotachi_destroy_fn destroy);
TOKOTACHI_C_API char* tokotachi_clipboard_text(void);
TOKOTACHI_C_API void tokotachi_clipboard_set_text(const char* text);
TOKOTACHI_C_API bool tokotachi_clipboard_has_text(void);
/* "Ctrl+S" などから作る */
TOKOTACHI_C_API tokotachi_shortcut tokotachi_shortcut_parse(const char* text);
TOKOTACHI_C_API char* tokotachi_shortcut_to_string(tokotachi_shortcut shortcut);
TOKOTACHI_C_API void tokotachi_theme_light(tokotachi_theme* out);
TOKOTACHI_C_API void tokotachi_theme_dark(tokotachi_theme* out);
/* 今のアプリケーションのテーマで具体的な色にする */
TOKOTACHI_C_API tokotachi_color tokotachi_theme_color_resolve(tokotachi_theme_color color);
/* PNG・JPEG・BMP・GIF を読む。読めなければ NULL */
TOKOTACHI_C_API tokotachi_image* tokotachi_image_load(const char* path);
TOKOTACHI_C_API tokotachi_image* tokotachi_image_load_from_data(const void* data, size_t size);
/* 画素(左上から行ごと。アルファは乗算していない)から作る。count が width × height でなければ NULL */
TOKOTACHI_C_API tokotachi_image* tokotachi_image_from_pixels(int width, int height, const tokotachi_color* pixels, size_t count);
/* 同じ画像を指す、別のハンドル */
TOKOTACHI_C_API tokotachi_image* tokotachi_image_copy(const tokotachi_image* image);
TOKOTACHI_C_API void tokotachi_image_free(tokotachi_image* image);
TOKOTACHI_C_API int tokotachi_image_width(const tokotachi_image* image);
TOKOTACHI_C_API int tokotachi_image_height(const tokotachi_image* image);
TOKOTACHI_C_API tokotachi_color tokotachi_image_pixel(const tokotachi_image* image, int x, int y);
/* 同じ画像か(C++ の Image の ==。中身ではなく、同じ読み込みから来たか) */
TOKOTACHI_C_API bool tokotachi_image_equal(const tokotachi_image* a, const tokotachi_image* b);
TOKOTACHI_C_API tokotachi_bitmap* tokotachi_bitmap_new(int width, int height, float scale);
TOKOTACHI_C_API tokotachi_bitmap* tokotachi_bitmap_from_image(const tokotachi_image* image, float scale);
TOKOTACHI_C_API void tokotachi_bitmap_free(tokotachi_bitmap* bitmap);
TOKOTACHI_C_API int tokotachi_bitmap_width(const tokotachi_bitmap* bitmap);
TOKOTACHI_C_API int tokotachi_bitmap_height(const tokotachi_bitmap* bitmap);
TOKOTACHI_C_API float tokotachi_bitmap_scale(const tokotachi_bitmap* bitmap);
/* この画像に描く Canvas(Bitmap が生きている間有効。解放しない) */
TOKOTACHI_C_API tokotachi_canvas* tokotachi_bitmap_canvas(tokotachi_bitmap* bitmap);
TOKOTACHI_C_API void tokotachi_bitmap_clear(tokotachi_bitmap* bitmap, tokotachi_color color);
TOKOTACHI_C_API tokotachi_color tokotachi_bitmap_pixel(const tokotachi_bitmap* bitmap, int x, int y);
TOKOTACHI_C_API void tokotachi_bitmap_set_pixel(tokotachi_bitmap* bitmap, int x, int y, tokotachi_color color);
/* 画素の並び(行ごとに width 個。アルファ乗算済みの 0xAARRGGBB) */
TOKOTACHI_C_API uint32_t* tokotachi_bitmap_pixels(tokotachi_bitmap* bitmap);
TOKOTACHI_C_API tokotachi_image* tokotachi_bitmap_to_image(const tokotachi_bitmap* bitmap);
/* PNG に保存する */
TOKOTACHI_C_API bool tokotachi_bitmap_save_png(const tokotachi_bitmap* bitmap, const char* path);
TOKOTACHI_C_API tokotachi_path* tokotachi_path_new(void);
TOKOTACHI_C_API void tokotachi_path_free(tokotachi_path* path);
TOKOTACHI_C_API void tokotachi_path_move_to(tokotachi_path* path, float x, float y);
TOKOTACHI_C_API void tokotachi_path_line_to(tokotachi_path* path, float x, float y);
TOKOTACHI_C_API void tokotachi_path_quad_to(tokotachi_path* path, float x1, float y1, float x2, float y2);
TOKOTACHI_C_API void tokotachi_path_cubic_to(tokotachi_path* path, float x1, float y1, float x2, float y2, float x3, float y3);
TOKOTACHI_C_API void tokotachi_path_close(tokotachi_path* path);
TOKOTACHI_C_API void tokotachi_path_add_rect(tokotachi_path* path, tokotachi_rect rect);
TOKOTACHI_C_API void tokotachi_path_add_round_rect(tokotachi_path* path, tokotachi_rect rect, float radius);
TOKOTACHI_C_API void tokotachi_path_add_ellipse(tokotachi_path* path, tokotachi_point center, float rx, float ry);
TOKOTACHI_C_API void tokotachi_path_add_arc(tokotachi_path* path, tokotachi_point center, float rx, float ry, float start_angle, float sweep_angle, bool new_figure);
TOKOTACHI_C_API void tokotachi_path_add_polyline(tokotachi_path* path, const tokotachi_point* points, size_t count, bool close);
TOKOTACHI_C_API bool tokotachi_path_empty(const tokotachi_path* path);
TOKOTACHI_C_API tokotachi_text_layout* tokotachi_text_layout_new(const char* text, tokotachi_font font, float scale, float wrap_width, tokotachi_align alignment);
TOKOTACHI_C_API void tokotachi_text_layout_free(tokotachi_text_layout* layout);
TOKOTACHI_C_API float tokotachi_text_layout_width(const tokotachi_text_layout* layout);
TOKOTACHI_C_API float tokotachi_text_layout_height(const tokotachi_text_layout* layout);
TOKOTACHI_C_API float tokotachi_text_layout_ascent(const tokotachi_text_layout* layout);
TOKOTACHI_C_API float tokotachi_text_layout_descent(const tokotachi_text_layout* layout);
TOKOTACHI_C_API float tokotachi_text_layout_line_height(const tokotachi_text_layout* layout);
TOKOTACHI_C_API size_t tokotachi_text_layout_line_count(const tokotachi_text_layout* layout);
TOKOTACHI_C_API tokotachi_rect tokotachi_text_layout_line_rect(const tokotachi_text_layout* layout, size_t line);
TOKOTACHI_C_API tokotachi_point tokotachi_text_layout_caret_position(const tokotachi_text_layout* layout, size_t offset);
TOKOTACHI_C_API size_t tokotachi_text_layout_offset_at(const tokotachi_text_layout* layout, tokotachi_point p);
TOKOTACHI_C_API float tokotachi_canvas_scale(tokotachi_canvas* canvas);
TOKOTACHI_C_API void tokotachi_canvas_save(tokotachi_canvas* canvas);
TOKOTACHI_C_API void tokotachi_canvas_restore(tokotachi_canvas* canvas);
TOKOTACHI_C_API void tokotachi_canvas_translate(tokotachi_canvas* canvas, float dx, float dy);
TOKOTACHI_C_API void tokotachi_canvas_scale_by(tokotachi_canvas* canvas, float sx, float sy);
TOKOTACHI_C_API void tokotachi_canvas_rotate(tokotachi_canvas* canvas, float radians);
TOKOTACHI_C_API void tokotachi_canvas_clip_rect(tokotachi_canvas* canvas, tokotachi_rect rect);
TOKOTACHI_C_API void tokotachi_canvas_clip_path(tokotachi_canvas* canvas, const tokotachi_path* path);
TOKOTACHI_C_API tokotachi_rect tokotachi_canvas_clip_bounds(tokotachi_canvas* canvas);
TOKOTACHI_C_API void tokotachi_canvas_begin_layer(tokotachi_canvas* canvas, float opacity);
TOKOTACHI_C_API void tokotachi_canvas_end_layer(tokotachi_canvas* canvas);
TOKOTACHI_C_API void tokotachi_canvas_fill_rect(tokotachi_canvas* canvas, tokotachi_rect rect, tokotachi_color color);
TOKOTACHI_C_API void tokotachi_canvas_fill_round_rect(tokotachi_canvas* canvas, tokotachi_rect rect, float radius, const tokotachi_paint* paint);
TOKOTACHI_C_API void tokotachi_canvas_stroke_round_rect(tokotachi_canvas* canvas, tokotachi_rect rect, float radius, float width, tokotachi_color color);
TOKOTACHI_C_API void tokotachi_canvas_stroke_rect(tokotachi_canvas* canvas, tokotachi_rect rect, float width, tokotachi_color color);
TOKOTACHI_C_API void tokotachi_canvas_fill_path(tokotachi_canvas* canvas, const tokotachi_path* path, const tokotachi_paint* paint);
TOKOTACHI_C_API void tokotachi_canvas_stroke_path(tokotachi_canvas* canvas, const tokotachi_path* path, const tokotachi_stroke_style* style, const tokotachi_paint* paint);
TOKOTACHI_C_API void tokotachi_canvas_draw_line(tokotachi_canvas* canvas, tokotachi_point from, tokotachi_point to, const tokotachi_stroke_style* style, const tokotachi_paint* paint);
TOKOTACHI_C_API void tokotachi_canvas_fill_ellipse(tokotachi_canvas* canvas, tokotachi_rect rect, const tokotachi_paint* paint);
TOKOTACHI_C_API void tokotachi_canvas_stroke_ellipse(tokotachi_canvas* canvas, tokotachi_rect rect, const tokotachi_stroke_style* style, const tokotachi_paint* paint);
/* 1 行の文字(pos は左上) */
TOKOTACHI_C_API void tokotachi_canvas_draw_text(tokotachi_canvas* canvas, const char* text, tokotachi_point pos, tokotachi_color color, tokotachi_font font);
TOKOTACHI_C_API void tokotachi_canvas_draw_text_layout(tokotachi_canvas* canvas, const tokotachi_text_layout* layout, tokotachi_point pos, tokotachi_color color);
TOKOTACHI_C_API void tokotachi_canvas_draw_image(tokotachi_canvas* canvas, const tokotachi_image* image, tokotachi_rect dst, float opacity);

/* ======== Object(include/tokotachi/core/property.hpp) ======== */
/* すべてのクラスの基底 */
TOKOTACHI_C_API tokotachi_object* tokotachi_object_get_owner(tokotachi_object* self);

/* ======== Application(include/tokotachi/core/application.hpp。基底は Object) ======== */
/* 1 つのプロセスに 1 つだけ作る */
TOKOTACHI_C_API tokotachi_application* tokotachi_application_new(int argc, char** argv);
TOKOTACHI_C_API tokotachi_color_scheme tokotachi_application_get_color_scheme(tokotachi_application* self);
TOKOTACHI_C_API void tokotachi_application_set_color_scheme(tokotachi_application* self, tokotachi_color_scheme value);
TOKOTACHI_C_API void tokotachi_application_get_light_theme(tokotachi_application* self, tokotachi_theme* out);
TOKOTACHI_C_API void tokotachi_application_set_light_theme(tokotachi_application* self, const tokotachi_theme* value);
TOKOTACHI_C_API void tokotachi_application_get_dark_theme(tokotachi_application* self, tokotachi_theme* out);
TOKOTACHI_C_API void tokotachi_application_set_dark_theme(tokotachi_application* self, const tokotachi_theme* value);
TOKOTACHI_C_API bool tokotachi_application_get_dark(tokotachi_application* self);
TOKOTACHI_C_API void tokotachi_application_get_theme(tokotachi_application* self, tokotachi_theme* out);
TOKOTACHI_C_API void tokotachi_application_set_on_theme_change(tokotachi_application* self, tokotachi_notify_fn fn, void* user_data, tokotachi_destroy_fn destroy);
/* イベントループを回し、quit で抜ける。戻り値は quit に渡した値 */
TOKOTACHI_C_API int tokotachi_application_exec(tokotachi_application* self);
TOKOTACHI_C_API void tokotachi_application_quit(tokotachi_application* self, int code);
TOKOTACHI_C_API tokotachi_application* tokotachi_application_instance(void);

/* ======== Timer(include/tokotachi/core/timer.hpp。基底は Object) ======== */
TOKOTACHI_C_API tokotachi_timer* tokotachi_timer_new(tokotachi_object* owner);
TOKOTACHI_C_API int tokotachi_timer_get_interval(tokotachi_timer* self);
TOKOTACHI_C_API void tokotachi_timer_set_interval(tokotachi_timer* self, int value);
TOKOTACHI_C_API bool tokotachi_timer_get_repeat(tokotachi_timer* self);
TOKOTACHI_C_API void tokotachi_timer_set_repeat(tokotachi_timer* self, bool value);
TOKOTACHI_C_API bool tokotachi_timer_get_enabled(tokotachi_timer* self);
TOKOTACHI_C_API void tokotachi_timer_set_enabled(tokotachi_timer* self, bool value);
TOKOTACHI_C_API void tokotachi_timer_set_on_timer(tokotachi_timer* self, tokotachi_notify_fn fn, void* user_data, tokotachi_destroy_fn destroy);

/* ======== Widget(include/tokotachi/widgets/widget.hpp。基底は Object) ======== */
/* ウィジェットの基底。tokotachi_widget_new は中身の無い入れ物を作る */
TOKOTACHI_C_API tokotachi_widget* tokotachi_widget_new(tokotachi_widget* parent);
TOKOTACHI_C_API tokotachi_widget* tokotachi_widget_get_parent(tokotachi_widget* self);
TOKOTACHI_C_API void tokotachi_widget_set_parent(tokotachi_widget* self, tokotachi_widget* value);
TOKOTACHI_C_API size_t tokotachi_widget_get_children(tokotachi_widget* self, tokotachi_widget** out, size_t capacity);
TOKOTACHI_C_API tokotachi_window* tokotachi_widget_get_window(tokotachi_widget* self);
TOKOTACHI_C_API float tokotachi_widget_get_x(tokotachi_widget* self);
TOKOTACHI_C_API void tokotachi_widget_set_x(tokotachi_widget* self, float value);
TOKOTACHI_C_API float tokotachi_widget_get_y(tokotachi_widget* self);
TOKOTACHI_C_API void tokotachi_widget_set_y(tokotachi_widget* self, float value);
TOKOTACHI_C_API float tokotachi_widget_get_width(tokotachi_widget* self);
TOKOTACHI_C_API void tokotachi_widget_set_width(tokotachi_widget* self, float value);
TOKOTACHI_C_API float tokotachi_widget_get_height(tokotachi_widget* self);
TOKOTACHI_C_API void tokotachi_widget_set_height(tokotachi_widget* self, float value);
TOKOTACHI_C_API tokotachi_rect tokotachi_widget_get_bounds(tokotachi_widget* self);
TOKOTACHI_C_API void tokotachi_widget_set_bounds(tokotachi_widget* self, tokotachi_rect value);
TOKOTACHI_C_API tokotachi_rect tokotachi_widget_get_local_rect(tokotachi_widget* self);
TOKOTACHI_C_API tokotachi_size tokotachi_widget_get_preferred_size(tokotachi_widget* self);
TOKOTACHI_C_API tokotachi_size tokotachi_widget_get_minimum_size(tokotachi_widget* self);
TOKOTACHI_C_API void tokotachi_widget_set_minimum_size(tokotachi_widget* self, tokotachi_size value);
TOKOTACHI_C_API tokotachi_size tokotachi_widget_get_maximum_size(tokotachi_widget* self);
TOKOTACHI_C_API void tokotachi_widget_set_maximum_size(tokotachi_widget* self, tokotachi_size value);
TOKOTACHI_C_API tokotachi_size tokotachi_widget_get_minimum_hint(tokotachi_widget* self);
TOKOTACHI_C_API tokotachi_layout* tokotachi_widget_get_layout(tokotachi_widget* self);
TOKOTACHI_C_API bool tokotachi_widget_get_visible(tokotachi_widget* self);
TOKOTACHI_C_API void tokotachi_widget_set_visible(tokotachi_widget* self, bool value);
TOKOTACHI_C_API bool tokotachi_widget_get_showing(tokotachi_widget* self);
TOKOTACHI_C_API bool tokotachi_widget_get_enabled(tokotachi_widget* self);
TOKOTACHI_C_API void tokotachi_widget_set_enabled(tokotachi_widget* self, bool value);
TOKOTACHI_C_API bool tokotachi_widget_get_effectively_enabled(tokotachi_widget* self);
TOKOTACHI_C_API tokotachi_focus_policy tokotachi_widget_get_focus_policy(tokotachi_widget* self);
TOKOTACHI_C_API void tokotachi_widget_set_focus_policy(tokotachi_widget* self, tokotachi_focus_policy value);
TOKOTACHI_C_API bool tokotachi_widget_get_focused(tokotachi_widget* self);
TOKOTACHI_C_API void tokotachi_widget_set_focused(tokotachi_widget* self, bool value);
TOKOTACHI_C_API tokotachi_mouse_cursor tokotachi_widget_get_cursor(tokotachi_widget* self);
TOKOTACHI_C_API void tokotachi_widget_set_cursor(tokotachi_widget* self, tokotachi_mouse_cursor value);
TOKOTACHI_C_API bool tokotachi_widget_get_accepts_text_input(tokotachi_widget* self);
TOKOTACHI_C_API void tokotachi_widget_set_accepts_text_input(tokotachi_widget* self, bool value);
TOKOTACHI_C_API float tokotachi_widget_get_display_scale(tokotachi_widget* self);
TOKOTACHI_C_API char* tokotachi_widget_get_tooltip(tokotachi_widget* self);
TOKOTACHI_C_API void tokotachi_widget_set_tooltip(tokotachi_widget* self, const char* value);
TOKOTACHI_C_API size_t tokotachi_widget_get_actions(tokotachi_widget* self, tokotachi_action** out, size_t capacity);
TOKOTACHI_C_API void tokotachi_widget_add_action(tokotachi_widget* self, tokotachi_action* action);
TOKOTACHI_C_API void tokotachi_widget_remove_action(tokotachi_widget* self, tokotachi_action* action);
TOKOTACHI_C_API void tokotachi_widget_invalidate(tokotachi_widget* self);
TOKOTACHI_C_API void tokotachi_widget_invalidate_rect(tokotachi_widget* self, tokotachi_rect rect);
TOKOTACHI_C_API void tokotachi_widget_request_layout(tokotachi_widget* self);
TOKOTACHI_C_API void tokotachi_widget_apply_layout(tokotachi_widget* self);
TOKOTACHI_C_API tokotachi_point tokotachi_widget_map_to_window(tokotachi_widget* self, tokotachi_point p);
TOKOTACHI_C_API tokotachi_point tokotachi_widget_map_from_window(tokotachi_widget* self, tokotachi_point p);
TOKOTACHI_C_API tokotachi_widget* tokotachi_widget_child_at(tokotachi_widget* self, tokotachi_point p);
/* 自分の描画の後に呼ぶ(子の描画の前) */
TOKOTACHI_C_API void tokotachi_widget_set_on_paint(tokotachi_widget* self, tokotachi_paint_fn fn, void* user_data, tokotachi_destroy_fn destroy);
TOKOTACHI_C_API void tokotachi_widget_set_on_resize(tokotachi_widget* self, tokotachi_notify_fn fn, void* user_data, tokotachi_destroy_fn destroy);
TOKOTACHI_C_API void tokotachi_widget_set_on_mouse_down(tokotachi_widget* self, tokotachi_mouse_fn fn, void* user_data, tokotachi_destroy_fn destroy);
TOKOTACHI_C_API void tokotachi_widget_set_on_mouse_up(tokotachi_widget* self, tokotachi_mouse_fn fn, void* user_data, tokotachi_destroy_fn destroy);
TOKOTACHI_C_API void tokotachi_widget_set_on_mouse_move(tokotachi_widget* self, tokotachi_mouse_fn fn, void* user_data, tokotachi_destroy_fn destroy);
TOKOTACHI_C_API void tokotachi_widget_set_on_mouse_enter(tokotachi_widget* self, tokotachi_notify_fn fn, void* user_data, tokotachi_destroy_fn destroy);
TOKOTACHI_C_API void tokotachi_widget_set_on_mouse_leave(tokotachi_widget* self, tokotachi_notify_fn fn, void* user_data, tokotachi_destroy_fn destroy);
TOKOTACHI_C_API void tokotachi_widget_set_on_mouse_wheel(tokotachi_widget* self, tokotachi_wheel_fn fn, void* user_data, tokotachi_destroy_fn destroy);
TOKOTACHI_C_API void tokotachi_widget_set_on_key_down(tokotachi_widget* self, tokotachi_key_fn fn, void* user_data, tokotachi_destroy_fn destroy);
TOKOTACHI_C_API void tokotachi_widget_set_on_key_up(tokotachi_widget* self, tokotachi_key_fn fn, void* user_data, tokotachi_destroy_fn destroy);
TOKOTACHI_C_API void tokotachi_widget_set_on_focus_in(tokotachi_widget* self, tokotachi_notify_fn fn, void* user_data, tokotachi_destroy_fn destroy);
TOKOTACHI_C_API void tokotachi_widget_set_on_focus_out(tokotachi_widget* self, tokotachi_notify_fn fn, void* user_data, tokotachi_destroy_fn destroy);

/* ======== ButtonBase(include/tokotachi/widgets/button_base.hpp。基底は Widget) ======== */
TOKOTACHI_C_API char* tokotachi_button_base_get_text(tokotachi_button_base* self);
TOKOTACHI_C_API void tokotachi_button_base_set_text(tokotachi_button_base* self, const char* value);
TOKOTACHI_C_API tokotachi_font tokotachi_button_base_get_font(tokotachi_button_base* self);
TOKOTACHI_C_API void tokotachi_button_base_set_font(tokotachi_button_base* self, tokotachi_font value);
TOKOTACHI_C_API void tokotachi_button_base_set_on_click(tokotachi_button_base* self, tokotachi_notify_fn fn, void* user_data, tokotachi_destroy_fn destroy);

/* ======== Button(include/tokotachi/widgets/button.hpp。基底は ButtonBase) ======== */
TOKOTACHI_C_API tokotachi_button* tokotachi_button_new(tokotachi_widget* parent, const char* text);
TOKOTACHI_C_API tokotachi_button_style tokotachi_button_get_style(tokotachi_button* self);
TOKOTACHI_C_API void tokotachi_button_set_style(tokotachi_button* self, tokotachi_button_style value);

/* ======== CheckBox(include/tokotachi/widgets/check_box.hpp。基底は ButtonBase) ======== */
TOKOTACHI_C_API tokotachi_check_box* tokotachi_check_box_new(tokotachi_widget* parent, const char* text);
TOKOTACHI_C_API bool tokotachi_check_box_get_checked(tokotachi_check_box* self);
TOKOTACHI_C_API void tokotachi_check_box_set_checked(tokotachi_check_box* self, bool value);
TOKOTACHI_C_API void tokotachi_check_box_set_on_change(tokotachi_check_box* self, tokotachi_notify_fn fn, void* user_data, tokotachi_destroy_fn destroy);

/* ======== RadioButton(include/tokotachi/widgets/radio_button.hpp。基底は ButtonBase) ======== */
TOKOTACHI_C_API tokotachi_radio_button* tokotachi_radio_button_new(tokotachi_widget* parent, const char* text, int value);
TOKOTACHI_C_API int tokotachi_radio_button_get_value(tokotachi_radio_button* self);
TOKOTACHI_C_API void tokotachi_radio_button_set_value(tokotachi_radio_button* self, int value);
TOKOTACHI_C_API int tokotachi_radio_button_get_group_value(tokotachi_radio_button* self);
TOKOTACHI_C_API void tokotachi_radio_button_set_group_value(tokotachi_radio_button* self, int value);
TOKOTACHI_C_API bool tokotachi_radio_button_get_checked(tokotachi_radio_button* self);
TOKOTACHI_C_API void tokotachi_radio_button_set_on_change(tokotachi_radio_button* self, tokotachi_notify_fn fn, void* user_data, tokotachi_destroy_fn destroy);

/* ======== ComboBox(include/tokotachi/widgets/combo_box.hpp。基底は Widget) ======== */
TOKOTACHI_C_API tokotachi_combo_box* tokotachi_combo_box_new(tokotachi_widget* parent);
TOKOTACHI_C_API char** tokotachi_combo_box_get_items(tokotachi_combo_box* self);
TOKOTACHI_C_API void tokotachi_combo_box_set_items(tokotachi_combo_box* self, const char* const* value, size_t value_count);
TOKOTACHI_C_API int tokotachi_combo_box_get_current_index(tokotachi_combo_box* self);
TOKOTACHI_C_API void tokotachi_combo_box_set_current_index(tokotachi_combo_box* self, int value);
TOKOTACHI_C_API char* tokotachi_combo_box_get_current_text(tokotachi_combo_box* self);
TOKOTACHI_C_API char* tokotachi_combo_box_get_placeholder_text(tokotachi_combo_box* self);
TOKOTACHI_C_API void tokotachi_combo_box_set_placeholder_text(tokotachi_combo_box* self, const char* value);
TOKOTACHI_C_API tokotachi_font tokotachi_combo_box_get_font(tokotachi_combo_box* self);
TOKOTACHI_C_API void tokotachi_combo_box_set_font(tokotachi_combo_box* self, tokotachi_font value);
TOKOTACHI_C_API void tokotachi_combo_box_set_on_change(tokotachi_combo_box* self, tokotachi_notify_fn fn, void* user_data, tokotachi_destroy_fn destroy);
TOKOTACHI_C_API void tokotachi_combo_box_open_popup(tokotachi_combo_box* self);
TOKOTACHI_C_API void tokotachi_combo_box_close_popup(tokotachi_combo_box* self);
TOKOTACHI_C_API bool tokotachi_combo_box_is_popup_open(tokotachi_combo_box* self);

/* ======== Label(include/tokotachi/widgets/label.hpp。基底は Widget) ======== */
TOKOTACHI_C_API tokotachi_label* tokotachi_label_new(tokotachi_widget* parent, const char* text);
TOKOTACHI_C_API char* tokotachi_label_get_text(tokotachi_label* self);
TOKOTACHI_C_API void tokotachi_label_set_text(tokotachi_label* self, const char* value);
TOKOTACHI_C_API tokotachi_font tokotachi_label_get_font(tokotachi_label* self);
TOKOTACHI_C_API void tokotachi_label_set_font(tokotachi_label* self, tokotachi_font value);
TOKOTACHI_C_API tokotachi_theme_color tokotachi_label_get_color(tokotachi_label* self);
TOKOTACHI_C_API void tokotachi_label_set_color(tokotachi_label* self, tokotachi_theme_color value);
TOKOTACHI_C_API tokotachi_align tokotachi_label_get_alignment(tokotachi_label* self);
TOKOTACHI_C_API void tokotachi_label_set_alignment(tokotachi_label* self, tokotachi_align value);
TOKOTACHI_C_API bool tokotachi_label_get_word_wrap(tokotachi_label* self);
TOKOTACHI_C_API void tokotachi_label_set_word_wrap(tokotachi_label* self, bool value);

/* ======== TextInputBase(include/tokotachi/widgets/text_input_base.hpp。基底は Widget) ======== */
TOKOTACHI_C_API char* tokotachi_text_input_base_get_text(tokotachi_text_input_base* self);
TOKOTACHI_C_API void tokotachi_text_input_base_set_text(tokotachi_text_input_base* self, const char* value);
TOKOTACHI_C_API char* tokotachi_text_input_base_get_placeholder_text(tokotachi_text_input_base* self);
TOKOTACHI_C_API void tokotachi_text_input_base_set_placeholder_text(tokotachi_text_input_base* self, const char* value);
TOKOTACHI_C_API tokotachi_font tokotachi_text_input_base_get_font(tokotachi_text_input_base* self);
TOKOTACHI_C_API void tokotachi_text_input_base_set_font(tokotachi_text_input_base* self, tokotachi_font value);
TOKOTACHI_C_API size_t tokotachi_text_input_base_get_cursor_position(tokotachi_text_input_base* self);
TOKOTACHI_C_API void tokotachi_text_input_base_set_cursor_position(tokotachi_text_input_base* self, size_t value);
TOKOTACHI_C_API bool tokotachi_text_input_base_get_has_selection(tokotachi_text_input_base* self);
TOKOTACHI_C_API char* tokotachi_text_input_base_get_selected_text(tokotachi_text_input_base* self);
TOKOTACHI_C_API void tokotachi_text_input_base_set_on_change(tokotachi_text_input_base* self, tokotachi_notify_fn fn, void* user_data, tokotachi_destroy_fn destroy);
TOKOTACHI_C_API void tokotachi_text_input_base_select_all(tokotachi_text_input_base* self);
TOKOTACHI_C_API void tokotachi_text_input_base_deselect(tokotachi_text_input_base* self);
TOKOTACHI_C_API void tokotachi_text_input_base_cut(tokotachi_text_input_base* self);
TOKOTACHI_C_API void tokotachi_text_input_base_copy(tokotachi_text_input_base* self);
TOKOTACHI_C_API void tokotachi_text_input_base_paste(tokotachi_text_input_base* self);
TOKOTACHI_C_API void tokotachi_text_input_base_insert(tokotachi_text_input_base* self, const char* s);

/* ======== LineEdit(include/tokotachi/widgets/line_edit.hpp。基底は TextInputBase) ======== */
TOKOTACHI_C_API tokotachi_line_edit* tokotachi_line_edit_new(tokotachi_widget* parent, const char* text);
TOKOTACHI_C_API void tokotachi_line_edit_set_on_submit(tokotachi_line_edit* self, tokotachi_notify_fn fn, void* user_data, tokotachi_destroy_fn destroy);

/* ======== TextEdit(include/tokotachi/widgets/text_edit.hpp。基底は TextInputBase) ======== */
TOKOTACHI_C_API tokotachi_text_edit* tokotachi_text_edit_new(tokotachi_widget* parent, const char* text);
TOKOTACHI_C_API bool tokotachi_text_edit_get_word_wrap(tokotachi_text_edit* self);
TOKOTACHI_C_API void tokotachi_text_edit_set_word_wrap(tokotachi_text_edit* self, bool value);

/* ======== PaintBox(include/tokotachi/widgets/paint_box.hpp。基底は Widget) ======== */
TOKOTACHI_C_API tokotachi_paint_box* tokotachi_paint_box_new(tokotachi_widget* parent);
TOKOTACHI_C_API bool tokotachi_paint_box_get_cached(tokotachi_paint_box* self);
TOKOTACHI_C_API void tokotachi_paint_box_set_cached(tokotachi_paint_box* self, bool value);
TOKOTACHI_C_API void tokotachi_paint_box_refresh(tokotachi_paint_box* self);

/* ======== ProgressBar(include/tokotachi/widgets/progress_bar.hpp。基底は Widget) ======== */
TOKOTACHI_C_API tokotachi_progress_bar* tokotachi_progress_bar_new(tokotachi_widget* parent);
TOKOTACHI_C_API double tokotachi_progress_bar_get_minimum(tokotachi_progress_bar* self);
TOKOTACHI_C_API void tokotachi_progress_bar_set_minimum(tokotachi_progress_bar* self, double value);
TOKOTACHI_C_API double tokotachi_progress_bar_get_maximum(tokotachi_progress_bar* self);
TOKOTACHI_C_API void tokotachi_progress_bar_set_maximum(tokotachi_progress_bar* self, double value);
TOKOTACHI_C_API double tokotachi_progress_bar_get_value(tokotachi_progress_bar* self);
TOKOTACHI_C_API void tokotachi_progress_bar_set_value(tokotachi_progress_bar* self, double value);
TOKOTACHI_C_API bool tokotachi_progress_bar_get_text_visible(tokotachi_progress_bar* self);
TOKOTACHI_C_API void tokotachi_progress_bar_set_text_visible(tokotachi_progress_bar* self, bool value);

/* ======== RangeBase(include/tokotachi/widgets/range_base.hpp。基底は Widget) ======== */
TOKOTACHI_C_API double tokotachi_range_base_get_minimum(tokotachi_range_base* self);
TOKOTACHI_C_API void tokotachi_range_base_set_minimum(tokotachi_range_base* self, double value);
TOKOTACHI_C_API double tokotachi_range_base_get_maximum(tokotachi_range_base* self);
TOKOTACHI_C_API void tokotachi_range_base_set_maximum(tokotachi_range_base* self, double value);
TOKOTACHI_C_API double tokotachi_range_base_get_value(tokotachi_range_base* self);
TOKOTACHI_C_API void tokotachi_range_base_set_value(tokotachi_range_base* self, double value);
TOKOTACHI_C_API double tokotachi_range_base_get_single_step(tokotachi_range_base* self);
TOKOTACHI_C_API void tokotachi_range_base_set_single_step(tokotachi_range_base* self, double value);
TOKOTACHI_C_API double tokotachi_range_base_get_page_step(tokotachi_range_base* self);
TOKOTACHI_C_API void tokotachi_range_base_set_page_step(tokotachi_range_base* self, double value);
TOKOTACHI_C_API void tokotachi_range_base_set_on_change(tokotachi_range_base* self, tokotachi_notify_fn fn, void* user_data, tokotachi_destroy_fn destroy);

/* ======== Slider(include/tokotachi/widgets/slider.hpp。基底は RangeBase) ======== */
TOKOTACHI_C_API tokotachi_slider* tokotachi_slider_new(tokotachi_widget* parent, tokotachi_orientation orientation);
TOKOTACHI_C_API tokotachi_orientation tokotachi_slider_get_orientation(tokotachi_slider* self);
TOKOTACHI_C_API void tokotachi_slider_set_orientation(tokotachi_slider* self, tokotachi_orientation value);

/* ======== SpinBox(include/tokotachi/widgets/spin_box.hpp。基底は RangeBase) ======== */
TOKOTACHI_C_API tokotachi_spin_box* tokotachi_spin_box_new(tokotachi_widget* parent);
TOKOTACHI_C_API int tokotachi_spin_box_get_decimals(tokotachi_spin_box* self);
TOKOTACHI_C_API void tokotachi_spin_box_set_decimals(tokotachi_spin_box* self, int value);
TOKOTACHI_C_API char* tokotachi_spin_box_get_text(tokotachi_spin_box* self);
TOKOTACHI_C_API void tokotachi_spin_box_commit(tokotachi_spin_box* self);

/* ======== ScrollArea(include/tokotachi/widgets/scroll_area.hpp。基底は Widget) ======== */
TOKOTACHI_C_API tokotachi_scroll_area* tokotachi_scroll_area_new(tokotachi_widget* parent);
TOKOTACHI_C_API tokotachi_widget* tokotachi_scroll_area_get_content(tokotachi_scroll_area* self);
TOKOTACHI_C_API void tokotachi_scroll_area_set_content(tokotachi_scroll_area* self, tokotachi_widget* value);
TOKOTACHI_C_API bool tokotachi_scroll_area_get_fit_width(tokotachi_scroll_area* self);
TOKOTACHI_C_API void tokotachi_scroll_area_set_fit_width(tokotachi_scroll_area* self, bool value);
TOKOTACHI_C_API tokotachi_point tokotachi_scroll_area_get_scroll_position(tokotachi_scroll_area* self);
TOKOTACHI_C_API void tokotachi_scroll_area_set_scroll_position(tokotachi_scroll_area* self, tokotachi_point value);
TOKOTACHI_C_API void tokotachi_scroll_area_scroll_to_visible(tokotachi_scroll_area* self, tokotachi_rect rect);

/* ======== ScrollBar(include/tokotachi/widgets/scroll_bar.hpp。基底は Widget) ======== */
TOKOTACHI_C_API tokotachi_scroll_bar* tokotachi_scroll_bar_new(tokotachi_widget* parent, tokotachi_orientation orientation);
TOKOTACHI_C_API tokotachi_orientation tokotachi_scroll_bar_get_orientation(tokotachi_scroll_bar* self);
TOKOTACHI_C_API void tokotachi_scroll_bar_set_orientation(tokotachi_scroll_bar* self, tokotachi_orientation value);
TOKOTACHI_C_API float tokotachi_scroll_bar_get_value(tokotachi_scroll_bar* self);
TOKOTACHI_C_API void tokotachi_scroll_bar_set_value(tokotachi_scroll_bar* self, float value);
TOKOTACHI_C_API float tokotachi_scroll_bar_get_maximum(tokotachi_scroll_bar* self);
TOKOTACHI_C_API void tokotachi_scroll_bar_set_maximum(tokotachi_scroll_bar* self, float value);
TOKOTACHI_C_API float tokotachi_scroll_bar_get_page_step(tokotachi_scroll_bar* self);
TOKOTACHI_C_API void tokotachi_scroll_bar_set_page_step(tokotachi_scroll_bar* self, float value);
TOKOTACHI_C_API float tokotachi_scroll_bar_get_single_step(tokotachi_scroll_bar* self);
TOKOTACHI_C_API void tokotachi_scroll_bar_set_single_step(tokotachi_scroll_bar* self, float value);
TOKOTACHI_C_API void tokotachi_scroll_bar_set_on_change(tokotachi_scroll_bar* self, tokotachi_notify_fn fn, void* user_data, tokotachi_destroy_fn destroy);

/* ======== SplitterPane(include/tokotachi/widgets/splitter.hpp。基底は Object) ======== */
/* Splitter が作り、所有する */
TOKOTACHI_C_API int tokotachi_splitter_pane_get_stretch(tokotachi_splitter_pane* self);
TOKOTACHI_C_API void tokotachi_splitter_pane_set_stretch(tokotachi_splitter_pane* self, int value);
TOKOTACHI_C_API bool tokotachi_splitter_pane_get_collapsible(tokotachi_splitter_pane* self);
TOKOTACHI_C_API void tokotachi_splitter_pane_set_collapsible(tokotachi_splitter_pane* self, bool value);
TOKOTACHI_C_API bool tokotachi_splitter_pane_get_collapsed(tokotachi_splitter_pane* self);
TOKOTACHI_C_API tokotachi_widget* tokotachi_splitter_pane_get_widget(tokotachi_splitter_pane* self);

/* ======== Splitter(include/tokotachi/widgets/splitter.hpp。基底は Widget) ======== */
TOKOTACHI_C_API tokotachi_splitter* tokotachi_splitter_new(tokotachi_widget* parent, tokotachi_orientation orientation);
TOKOTACHI_C_API tokotachi_orientation tokotachi_splitter_get_orientation(tokotachi_splitter* self);
TOKOTACHI_C_API void tokotachi_splitter_set_orientation(tokotachi_splitter* self, tokotachi_orientation value);
TOKOTACHI_C_API size_t tokotachi_splitter_get_panes(tokotachi_splitter* self, tokotachi_splitter_pane** out, size_t capacity);
TOKOTACHI_C_API size_t tokotachi_splitter_get_sizes(tokotachi_splitter* self, float* out, size_t capacity);
TOKOTACHI_C_API void tokotachi_splitter_set_sizes(tokotachi_splitter* self, const float* value, size_t value_count);
TOKOTACHI_C_API void tokotachi_splitter_set_on_change(tokotachi_splitter* self, tokotachi_notify_fn fn, void* user_data, tokotachi_destroy_fn destroy);
TOKOTACHI_C_API tokotachi_splitter_pane* tokotachi_splitter_add_widget(tokotachi_splitter* self, tokotachi_widget* widget, int stretch);
TOKOTACHI_C_API tokotachi_splitter_pane* tokotachi_splitter_insert_widget(tokotachi_splitter* self, int index, tokotachi_widget* widget, int stretch);

/* ======== Tab(include/tokotachi/widgets/tab_widget.hpp。基底は Object) ======== */
/* TabWidget が作り、所有する */
TOKOTACHI_C_API char* tokotachi_tab_get_title(tokotachi_tab* self);
TOKOTACHI_C_API void tokotachi_tab_set_title(tokotachi_tab* self, const char* value);
TOKOTACHI_C_API bool tokotachi_tab_get_enabled(tokotachi_tab* self);
TOKOTACHI_C_API void tokotachi_tab_set_enabled(tokotachi_tab* self, bool value);
TOKOTACHI_C_API tokotachi_widget* tokotachi_tab_get_page(tokotachi_tab* self);

/* ======== TabWidget(include/tokotachi/widgets/tab_widget.hpp。基底は Widget) ======== */
TOKOTACHI_C_API tokotachi_tab_widget* tokotachi_tab_widget_new(tokotachi_widget* parent);
TOKOTACHI_C_API size_t tokotachi_tab_widget_get_tabs(tokotachi_tab_widget* self, tokotachi_tab** out, size_t capacity);
TOKOTACHI_C_API int tokotachi_tab_widget_get_current_index(tokotachi_tab_widget* self);
TOKOTACHI_C_API void tokotachi_tab_widget_set_current_index(tokotachi_tab_widget* self, int value);
TOKOTACHI_C_API tokotachi_tab* tokotachi_tab_widget_get_current_tab(tokotachi_tab_widget* self);
TOKOTACHI_C_API void tokotachi_tab_widget_set_on_change(tokotachi_tab_widget* self, tokotachi_notify_fn fn, void* user_data, tokotachi_destroy_fn destroy);
TOKOTACHI_C_API tokotachi_tab* tokotachi_tab_widget_add_tab(tokotachi_tab_widget* self, tokotachi_widget* page, const char* title);
TOKOTACHI_C_API tokotachi_tab* tokotachi_tab_widget_insert_tab(tokotachi_tab_widget* self, int index, tokotachi_widget* page, const char* title);
/* タブを除き、ページを返す(ページは親が無くなり、呼んだ側が破棄する) */
TOKOTACHI_C_API tokotachi_widget* tokotachi_tab_widget_remove_tab(tokotachi_tab_widget* self, tokotachi_tab* tab);

/* ======== Window(include/tokotachi/windows/window.hpp。基底は Widget) ======== */
/* Application を作った後に作る。所有者を持たない(利用者が破棄する) */
TOKOTACHI_C_API tokotachi_window* tokotachi_window_new(const char* title, float width, float height);
TOKOTACHI_C_API char* tokotachi_window_get_title(tokotachi_window* self);
TOKOTACHI_C_API void tokotachi_window_set_title(tokotachi_window* self, const char* value);
TOKOTACHI_C_API tokotachi_theme_color tokotachi_window_get_background(tokotachi_window* self);
TOKOTACHI_C_API void tokotachi_window_set_background(tokotachi_window* self, tokotachi_theme_color value);
TOKOTACHI_C_API tokotachi_widget* tokotachi_window_get_focused_widget(tokotachi_window* self);
TOKOTACHI_C_API tokotachi_menu_bar* tokotachi_window_get_menu_bar(tokotachi_window* self);
TOKOTACHI_C_API void tokotachi_window_set_menu_bar(tokotachi_window* self, tokotachi_menu_bar* value);
TOKOTACHI_C_API tokotachi_tool_bar* tokotachi_window_get_tool_bar(tokotachi_window* self);
TOKOTACHI_C_API void tokotachi_window_set_tool_bar(tokotachi_window* self, tokotachi_tool_bar* value);
TOKOTACHI_C_API tokotachi_status_bar* tokotachi_window_get_status_bar(tokotachi_window* self);
TOKOTACHI_C_API void tokotachi_window_set_status_bar(tokotachi_window* self, tokotachi_status_bar* value);
TOKOTACHI_C_API void tokotachi_window_set_on_close(tokotachi_window* self, tokotachi_close_fn fn, void* user_data, tokotachi_destroy_fn destroy);
TOKOTACHI_C_API void tokotachi_window_close(tokotachi_window* self);
TOKOTACHI_C_API bool tokotachi_window_save_png(tokotachi_window* self, const char* path);
/* 全体を描いて Bitmap にする(tokotachi_bitmap_free で解放する) */
TOKOTACHI_C_API tokotachi_bitmap* tokotachi_window_capture(tokotachi_window* self);

/* ======== Dialog(include/tokotachi/windows/dialog.hpp。基底は Window) ======== */
/* parent が所有する */
TOKOTACHI_C_API tokotachi_dialog* tokotachi_dialog_new(tokotachi_window* parent, const char* title);
TOKOTACHI_C_API tokotachi_dialog_button tokotachi_dialog_get_result(tokotachi_dialog* self);
TOKOTACHI_C_API void tokotachi_dialog_set_on_finish(tokotachi_dialog* self, tokotachi_notify_fn fn, void* user_data, tokotachi_destroy_fn destroy);
/* text が空なら、言語に合わせた標準の文字 */
TOKOTACHI_C_API tokotachi_button* tokotachi_dialog_add_button(tokotachi_dialog* self, tokotachi_dialog_button button, const char* text);
TOKOTACHI_C_API void tokotachi_dialog_open(tokotachi_dialog* self);
TOKOTACHI_C_API void tokotachi_dialog_finish(tokotachi_dialog* self, tokotachi_dialog_button result);

/* ======== MessageDialog(include/tokotachi/windows/message_dialog.hpp。基底は Dialog) ======== */
TOKOTACHI_C_API tokotachi_message_dialog* tokotachi_message_dialog_new(tokotachi_window* parent, const char* title, const char* text, tokotachi_message_type type);
TOKOTACHI_C_API char* tokotachi_message_dialog_get_text(tokotachi_message_dialog* self);
TOKOTACHI_C_API void tokotachi_message_dialog_set_text(tokotachi_message_dialog* self, const char* value);
TOKOTACHI_C_API tokotachi_message_type tokotachi_message_dialog_get_type(tokotachi_message_dialog* self);
TOKOTACHI_C_API void tokotachi_message_dialog_set_type(tokotachi_message_dialog* self, tokotachi_message_type value);
/* 作ってボタンを加えて開き、閉じたら on_finish を呼んで破棄する(button_count が 0 なら OK だけ) */
TOKOTACHI_C_API tokotachi_message_dialog* tokotachi_message_dialog_show(tokotachi_window* parent, const char* title, const char* text, const tokotachi_dialog_button* buttons, size_t button_count, tokotachi_notify_fn on_finish, void* user_data, tokotachi_destroy_fn destroy, tokotachi_message_type type);

/* ======== FileDialog(include/tokotachi/windows/file_dialog.hpp。基底は Object) ======== */
TOKOTACHI_C_API tokotachi_file_dialog* tokotachi_file_dialog_new(tokotachi_window* parent, tokotachi_file_dialog_mode mode);
TOKOTACHI_C_API tokotachi_file_dialog_mode tokotachi_file_dialog_get_mode(tokotachi_file_dialog* self);
TOKOTACHI_C_API void tokotachi_file_dialog_set_mode(tokotachi_file_dialog* self, tokotachi_file_dialog_mode value);
TOKOTACHI_C_API char* tokotachi_file_dialog_get_title(tokotachi_file_dialog* self);
TOKOTACHI_C_API void tokotachi_file_dialog_set_title(tokotachi_file_dialog* self, const char* value);
/* 名前と拡張子(";" でつなぐ)を交互に並べたもの(["画像", "png;jpg"]) */
TOKOTACHI_C_API char** tokotachi_file_dialog_get_filters(tokotachi_file_dialog* self);
TOKOTACHI_C_API void tokotachi_file_dialog_set_filters(tokotachi_file_dialog* self, const char* const* value, size_t value_count);
TOKOTACHI_C_API char* tokotachi_file_dialog_get_location(tokotachi_file_dialog* self);
TOKOTACHI_C_API void tokotachi_file_dialog_set_location(tokotachi_file_dialog* self, const char* value);
TOKOTACHI_C_API bool tokotachi_file_dialog_get_allow_many(tokotachi_file_dialog* self);
TOKOTACHI_C_API void tokotachi_file_dialog_set_allow_many(tokotachi_file_dialog* self, bool value);
TOKOTACHI_C_API char** tokotachi_file_dialog_get_paths(tokotachi_file_dialog* self);
TOKOTACHI_C_API char* tokotachi_file_dialog_get_error(tokotachi_file_dialog* self);
TOKOTACHI_C_API int tokotachi_file_dialog_get_selected_filter(tokotachi_file_dialog* self);
TOKOTACHI_C_API bool tokotachi_file_dialog_get_running(tokotachi_file_dialog* self);
TOKOTACHI_C_API void tokotachi_file_dialog_set_on_finish(tokotachi_file_dialog* self, tokotachi_notify_fn fn, void* user_data, tokotachi_destroy_fn destroy);
TOKOTACHI_C_API void tokotachi_file_dialog_open(tokotachi_file_dialog* self);

/* ======== MenuBar(include/tokotachi/windows/menu_bar.hpp。基底は Widget) ======== */
TOKOTACHI_C_API tokotachi_menu_bar* tokotachi_menu_bar_new(tokotachi_widget* parent);
TOKOTACHI_C_API size_t tokotachi_menu_bar_get_menus(tokotachi_menu_bar* self, tokotachi_menu** out, size_t capacity);
TOKOTACHI_C_API tokotachi_menu* tokotachi_menu_bar_add_menu(tokotachi_menu_bar* self, const char* title);
TOKOTACHI_C_API void tokotachi_menu_bar_remove_menu(tokotachi_menu_bar* self, tokotachi_menu* menu);

/* ======== Popup(include/tokotachi/windows/popup.hpp。基底は Window) ======== */
TOKOTACHI_C_API tokotachi_popup* tokotachi_popup_new(tokotachi_widget* owner_widget, tokotachi_popup_kind kind, tokotachi_object* owner_object);
TOKOTACHI_C_API tokotachi_widget* tokotachi_popup_get_owner_widget(tokotachi_popup* self);
TOKOTACHI_C_API tokotachi_popup_kind tokotachi_popup_get_kind(tokotachi_popup* self);
TOKOTACHI_C_API void tokotachi_popup_set_on_closed(tokotachi_popup* self, tokotachi_notify_fn fn, void* user_data, tokotachi_destroy_fn destroy);
TOKOTACHI_C_API void tokotachi_popup_open(tokotachi_popup* self, tokotachi_rect anchor, tokotachi_size size, tokotachi_popup_placement placement);

/* ======== StatusBar(include/tokotachi/windows/status_bar.hpp。基底は Widget) ======== */
TOKOTACHI_C_API tokotachi_status_bar* tokotachi_status_bar_new(tokotachi_widget* parent);
TOKOTACHI_C_API char* tokotachi_status_bar_get_text(tokotachi_status_bar* self);
TOKOTACHI_C_API void tokotachi_status_bar_set_text(tokotachi_status_bar* self, const char* value);
TOKOTACHI_C_API char* tokotachi_status_bar_get_message(tokotachi_status_bar* self);
TOKOTACHI_C_API void tokotachi_status_bar_show_message(tokotachi_status_bar* self, const char* message, int timeout_ms);
TOKOTACHI_C_API void tokotachi_status_bar_clear_message(tokotachi_status_bar* self);
TOKOTACHI_C_API void tokotachi_status_bar_add_widget(tokotachi_status_bar* self, tokotachi_widget* widget);

/* ======== ToolBar(include/tokotachi/windows/tool_bar.hpp。基底は Widget) ======== */
TOKOTACHI_C_API tokotachi_tool_bar* tokotachi_tool_bar_new(tokotachi_widget* parent);
TOKOTACHI_C_API tokotachi_tool_button_style tokotachi_tool_bar_get_button_style(tokotachi_tool_bar* self);
TOKOTACHI_C_API void tokotachi_tool_bar_set_button_style(tokotachi_tool_bar* self, tokotachi_tool_button_style value);
/* Action を作ってボタンとして加える(ツールバーが所有する) */
TOKOTACHI_C_API tokotachi_action* tokotachi_tool_bar_add_item(tokotachi_tool_bar* self, const char* text);
TOKOTACHI_C_API void tokotachi_tool_bar_add_separator(tokotachi_tool_bar* self);
TOKOTACHI_C_API void tokotachi_tool_bar_add_widget(tokotachi_tool_bar* self, tokotachi_widget* widget);

/* ======== Layout(include/tokotachi/layout/layout.hpp。基底は Object) ======== */
/* target が所有する(target が前のレイアウトを持っていれば、それは破棄される) */
TOKOTACHI_C_API tokotachi_widget* tokotachi_layout_get_widget(tokotachi_layout* self);
TOKOTACHI_C_API tokotachi_margins tokotachi_layout_get_margins(tokotachi_layout* self);
TOKOTACHI_C_API void tokotachi_layout_set_margins(tokotachi_layout* self, tokotachi_margins value);
TOKOTACHI_C_API void tokotachi_layout_remove(tokotachi_layout* self, tokotachi_widget* widget);

/* ======== BoxLayout(include/tokotachi/layout/box_layout.hpp。基底は Layout) ======== */
TOKOTACHI_C_API tokotachi_box_layout* tokotachi_box_layout_new(tokotachi_widget* target, tokotachi_orientation orientation);
TOKOTACHI_C_API tokotachi_orientation tokotachi_box_layout_get_orientation(tokotachi_box_layout* self);
TOKOTACHI_C_API void tokotachi_box_layout_set_orientation(tokotachi_box_layout* self, tokotachi_orientation value);
TOKOTACHI_C_API float tokotachi_box_layout_get_spacing(tokotachi_box_layout* self);
TOKOTACHI_C_API void tokotachi_box_layout_set_spacing(tokotachi_box_layout* self, float value);
TOKOTACHI_C_API void tokotachi_box_layout_add(tokotachi_box_layout* self, tokotachi_widget* widget, int stretch, tokotachi_align cross_alignment);
TOKOTACHI_C_API void tokotachi_box_layout_add_spacing(tokotachi_box_layout* self, float size);
TOKOTACHI_C_API void tokotachi_box_layout_add_stretch(tokotachi_box_layout* self, int stretch);
TOKOTACHI_C_API void tokotachi_box_layout_set_stretch(tokotachi_box_layout* self, tokotachi_widget* widget, int stretch);
TOKOTACHI_C_API void tokotachi_box_layout_set_alignment(tokotachi_box_layout* self, tokotachi_widget* widget, tokotachi_align cross_alignment);

/* ======== GridLayout(include/tokotachi/layout/grid_layout.hpp。基底は Layout) ======== */
TOKOTACHI_C_API tokotachi_grid_layout* tokotachi_grid_layout_new(tokotachi_widget* target);
TOKOTACHI_C_API float tokotachi_grid_layout_get_row_spacing(tokotachi_grid_layout* self);
TOKOTACHI_C_API void tokotachi_grid_layout_set_row_spacing(tokotachi_grid_layout* self, float value);
TOKOTACHI_C_API float tokotachi_grid_layout_get_column_spacing(tokotachi_grid_layout* self);
TOKOTACHI_C_API void tokotachi_grid_layout_set_column_spacing(tokotachi_grid_layout* self, float value);
TOKOTACHI_C_API void tokotachi_grid_layout_add(tokotachi_grid_layout* self, tokotachi_widget* widget, int row, int column, int row_span, int column_span, tokotachi_align horizontal, tokotachi_align vertical);
TOKOTACHI_C_API void tokotachi_grid_layout_set_row_stretch(tokotachi_grid_layout* self, int row, int stretch);
TOKOTACHI_C_API void tokotachi_grid_layout_set_column_stretch(tokotachi_grid_layout* self, int column, int stretch);
TOKOTACHI_C_API void tokotachi_grid_layout_set_row_minimum(tokotachi_grid_layout* self, int row, float size);
TOKOTACHI_C_API void tokotachi_grid_layout_set_column_minimum(tokotachi_grid_layout* self, int column, float size);

/* ======== PlaceLayout(include/tokotachi/layout/place_layout.hpp。基底は Layout) ======== */
TOKOTACHI_C_API tokotachi_place_layout* tokotachi_place_layout_new(tokotachi_widget* target);
TOKOTACHI_C_API void tokotachi_place_layout_add(tokotachi_place_layout* self, tokotachi_widget* widget, tokotachi_placement placement);

/* ======== Action(include/tokotachi/actions/action.hpp。基底は Object) ======== */
TOKOTACHI_C_API tokotachi_action* tokotachi_action_new(tokotachi_object* owner, const char* text);
TOKOTACHI_C_API char* tokotachi_action_get_text(tokotachi_action* self);
TOKOTACHI_C_API void tokotachi_action_set_text(tokotachi_action* self, const char* value);
TOKOTACHI_C_API tokotachi_shortcut tokotachi_action_get_shortcut(tokotachi_action* self);
TOKOTACHI_C_API void tokotachi_action_set_shortcut(tokotachi_action* self, tokotachi_shortcut value);
TOKOTACHI_C_API tokotachi_image* tokotachi_action_get_icon(tokotachi_action* self);
TOKOTACHI_C_API void tokotachi_action_set_icon(tokotachi_action* self, const tokotachi_image* value);
TOKOTACHI_C_API char* tokotachi_action_get_tooltip(tokotachi_action* self);
TOKOTACHI_C_API void tokotachi_action_set_tooltip(tokotachi_action* self, const char* value);
TOKOTACHI_C_API bool tokotachi_action_get_enabled(tokotachi_action* self);
TOKOTACHI_C_API void tokotachi_action_set_enabled(tokotachi_action* self, bool value);
TOKOTACHI_C_API bool tokotachi_action_get_checkable(tokotachi_action* self);
TOKOTACHI_C_API void tokotachi_action_set_checkable(tokotachi_action* self, bool value);
TOKOTACHI_C_API bool tokotachi_action_get_checked(tokotachi_action* self);
TOKOTACHI_C_API void tokotachi_action_set_checked(tokotachi_action* self, bool value);
TOKOTACHI_C_API void tokotachi_action_set_on_trigger(tokotachi_action* self, tokotachi_notify_fn fn, void* user_data, tokotachi_destroy_fn destroy);
TOKOTACHI_C_API void tokotachi_action_trigger(tokotachi_action* self);

/* ======== Menu(include/tokotachi/actions/menu.hpp。基底は Object) ======== */
TOKOTACHI_C_API tokotachi_menu* tokotachi_menu_new(tokotachi_object* owner, const char* title);
TOKOTACHI_C_API char* tokotachi_menu_get_title(tokotachi_menu* self);
TOKOTACHI_C_API void tokotachi_menu_set_title(tokotachi_menu* self, const char* value);
TOKOTACHI_C_API size_t tokotachi_menu_get_actions(tokotachi_menu* self, tokotachi_action** out, size_t capacity);
/* Action を作って最後に加える(メニューが所有する) */
TOKOTACHI_C_API tokotachi_action* tokotachi_menu_add_item(tokotachi_menu* self, const char* text);
TOKOTACHI_C_API void tokotachi_menu_add_action(tokotachi_menu* self, tokotachi_action* action);
TOKOTACHI_C_API void tokotachi_menu_remove_action(tokotachi_menu* self, tokotachi_action* action);
TOKOTACHI_C_API void tokotachi_menu_add_separator(tokotachi_menu* self);
TOKOTACHI_C_API tokotachi_menu* tokotachi_menu_add_submenu(tokotachi_menu* self, const char* title);
TOKOTACHI_C_API void tokotachi_menu_popup(tokotachi_menu* self, tokotachi_widget* anchor_widget, tokotachi_point at);
TOKOTACHI_C_API void tokotachi_menu_close(tokotachi_menu* self);
TOKOTACHI_C_API bool tokotachi_menu_is_open(tokotachi_menu* self);

/* ======== ItemModel(include/tokotachi/views/item_model.hpp。基底は Object) ======== */
/* 一覧・木のモデルの基底。独自のモデルは tokotachi_custom_model_new で作る */
TOKOTACHI_C_API int tokotachi_item_model_row_count(tokotachi_item_model* self, tokotachi_model_index parent);
TOKOTACHI_C_API tokotachi_model_index tokotachi_item_model_index(tokotachi_item_model* self, int row, tokotachi_model_index parent);
TOKOTACHI_C_API tokotachi_model_index tokotachi_item_model_parent_of(tokotachi_item_model* self, tokotachi_model_index index);
TOKOTACHI_C_API char* tokotachi_item_model_text(tokotachi_item_model* self, tokotachi_model_index index, int column);
TOKOTACHI_C_API tokotachi_image* tokotachi_item_model_icon(tokotachi_item_model* self, tokotachi_model_index index);
TOKOTACHI_C_API bool tokotachi_item_model_enabled(tokotachi_item_model* self, tokotachi_model_index index);
TOKOTACHI_C_API bool tokotachi_item_model_checkable(tokotachi_item_model* self, tokotachi_model_index index);
TOKOTACHI_C_API bool tokotachi_item_model_checked(tokotachi_item_model* self, tokotachi_model_index index);
TOKOTACHI_C_API void tokotachi_item_model_set_checked(tokotachi_item_model* self, tokotachi_model_index index, bool checked);
TOKOTACHI_C_API void tokotachi_item_model_sort(tokotachi_item_model* self, int column, tokotachi_sort_order order);
TOKOTACHI_C_API tokotachi_model_index tokotachi_item_model_locate(tokotachi_item_model* self, tokotachi_model_index index);

/* ======== Item(include/tokotachi/views/item.hpp。基底は Object) ======== */
/* ListModel・TreeModel の項目(モデルが所有する) */
TOKOTACHI_C_API char* tokotachi_item_get_text(tokotachi_item* self);
TOKOTACHI_C_API void tokotachi_item_set_text(tokotachi_item* self, const char* value);
TOKOTACHI_C_API char** tokotachi_item_get_column_texts(tokotachi_item* self);
TOKOTACHI_C_API void tokotachi_item_set_column_texts(tokotachi_item* self, const char* const* value, size_t value_count);
TOKOTACHI_C_API tokotachi_image* tokotachi_item_get_icon(tokotachi_item* self);
TOKOTACHI_C_API void tokotachi_item_set_icon(tokotachi_item* self, const tokotachi_image* value);
TOKOTACHI_C_API bool tokotachi_item_get_enabled(tokotachi_item* self);
TOKOTACHI_C_API void tokotachi_item_set_enabled(tokotachi_item* self, bool value);
TOKOTACHI_C_API bool tokotachi_item_get_checkable(tokotachi_item* self);
TOKOTACHI_C_API void tokotachi_item_set_checkable(tokotachi_item* self, bool value);
TOKOTACHI_C_API bool tokotachi_item_get_checked(tokotachi_item* self);
TOKOTACHI_C_API void tokotachi_item_set_checked(tokotachi_item* self, bool value);
TOKOTACHI_C_API char* tokotachi_item_column_text(tokotachi_item* self, int column);

/* ======== ListModel(include/tokotachi/views/list_model.hpp。基底は ItemModel) ======== */
TOKOTACHI_C_API tokotachi_list_model* tokotachi_list_model_new(tokotachi_object* owner);
TOKOTACHI_C_API size_t tokotachi_list_model_get_items(tokotachi_list_model* self, tokotachi_item** out, size_t capacity);
TOKOTACHI_C_API tokotachi_item* tokotachi_list_model_add_item(tokotachi_list_model* self, const char* text);
TOKOTACHI_C_API tokotachi_item* tokotachi_list_model_insert_item(tokotachi_list_model* self, int index, const char* text);
TOKOTACHI_C_API void tokotachi_list_model_remove_item(tokotachi_list_model* self, tokotachi_item* item);
TOKOTACHI_C_API void tokotachi_list_model_clear(tokotachi_list_model* self);
TOKOTACHI_C_API tokotachi_model_index tokotachi_list_model_index_of(tokotachi_list_model* self, tokotachi_item* item);
TOKOTACHI_C_API tokotachi_item* tokotachi_list_model_item_at(tokotachi_list_model* self, tokotachi_model_index index);

/* ======== TreeItem(include/tokotachi/views/tree_model.hpp。基底は Item) ======== */
TOKOTACHI_C_API tokotachi_tree_item* tokotachi_tree_item_get_parent_item(tokotachi_tree_item* self);
TOKOTACHI_C_API size_t tokotachi_tree_item_get_children(tokotachi_tree_item* self, tokotachi_tree_item** out, size_t capacity);
TOKOTACHI_C_API tokotachi_tree_item* tokotachi_tree_item_add_child(tokotachi_tree_item* self, const char* text);
TOKOTACHI_C_API tokotachi_tree_item* tokotachi_tree_item_insert_child(tokotachi_tree_item* self, int index, const char* text);

/* ======== TreeModel(include/tokotachi/views/tree_model.hpp。基底は ItemModel) ======== */
TOKOTACHI_C_API tokotachi_tree_model* tokotachi_tree_model_new(tokotachi_object* owner);
TOKOTACHI_C_API size_t tokotachi_tree_model_get_items(tokotachi_tree_model* self, tokotachi_tree_item** out, size_t capacity);
TOKOTACHI_C_API tokotachi_tree_item* tokotachi_tree_model_add_item(tokotachi_tree_model* self, const char* text);
TOKOTACHI_C_API tokotachi_tree_item* tokotachi_tree_model_insert_item(tokotachi_tree_model* self, int index, const char* text);
TOKOTACHI_C_API void tokotachi_tree_model_remove_item(tokotachi_tree_model* self, tokotachi_tree_item* item);
TOKOTACHI_C_API void tokotachi_tree_model_clear(tokotachi_tree_model* self);
TOKOTACHI_C_API tokotachi_model_index tokotachi_tree_model_index_of(tokotachi_tree_model* self, tokotachi_tree_item* item);
TOKOTACHI_C_API tokotachi_tree_item* tokotachi_tree_model_item_at(tokotachi_tree_model* self, tokotachi_model_index index);

/* ======== Column(include/tokotachi/views/item_view.hpp。基底は Object) ======== */
/* ItemView が作り、所有する */
TOKOTACHI_C_API char* tokotachi_column_get_title(tokotachi_column* self);
TOKOTACHI_C_API void tokotachi_column_set_title(tokotachi_column* self, const char* value);
TOKOTACHI_C_API float tokotachi_column_get_width(tokotachi_column* self);
TOKOTACHI_C_API void tokotachi_column_set_width(tokotachi_column* self, float value);
TOKOTACHI_C_API tokotachi_align tokotachi_column_get_alignment(tokotachi_column* self);
TOKOTACHI_C_API void tokotachi_column_set_alignment(tokotachi_column* self, tokotachi_align value);

/* ======== ItemView(include/tokotachi/views/item_view.hpp。基底は Widget) ======== */
/* ListView・TreeView の基底(モデルは所有しない) */
TOKOTACHI_C_API tokotachi_item_model* tokotachi_item_view_get_model(tokotachi_item_view* self);
TOKOTACHI_C_API void tokotachi_item_view_set_model(tokotachi_item_view* self, tokotachi_item_model* value);
TOKOTACHI_C_API size_t tokotachi_item_view_get_columns(tokotachi_item_view* self, tokotachi_column** out, size_t capacity);
TOKOTACHI_C_API bool tokotachi_item_view_get_header_visible(tokotachi_item_view* self);
TOKOTACHI_C_API void tokotachi_item_view_set_header_visible(tokotachi_item_view* self, bool value);
TOKOTACHI_C_API bool tokotachi_item_view_get_sorting_enabled(tokotachi_item_view* self);
TOKOTACHI_C_API void tokotachi_item_view_set_sorting_enabled(tokotachi_item_view* self, bool value);
TOKOTACHI_C_API int tokotachi_item_view_get_sort_column(tokotachi_item_view* self);
TOKOTACHI_C_API void tokotachi_item_view_set_sort_column(tokotachi_item_view* self, int value);
TOKOTACHI_C_API tokotachi_sort_order tokotachi_item_view_get_sort_order(tokotachi_item_view* self);
TOKOTACHI_C_API void tokotachi_item_view_set_sort_order(tokotachi_item_view* self, tokotachi_sort_order value);
TOKOTACHI_C_API tokotachi_selection_mode tokotachi_item_view_get_selection_mode(tokotachi_item_view* self);
TOKOTACHI_C_API void tokotachi_item_view_set_selection_mode(tokotachi_item_view* self, tokotachi_selection_mode value);
TOKOTACHI_C_API tokotachi_model_index tokotachi_item_view_get_current(tokotachi_item_view* self);
TOKOTACHI_C_API void tokotachi_item_view_set_current(tokotachi_item_view* self, tokotachi_model_index value);
TOKOTACHI_C_API size_t tokotachi_item_view_get_selection(tokotachi_item_view* self, tokotachi_model_index* out, size_t capacity);
TOKOTACHI_C_API void tokotachi_item_view_set_on_selection_change(tokotachi_item_view* self, tokotachi_notify_fn fn, void* user_data, tokotachi_destroy_fn destroy);
TOKOTACHI_C_API void tokotachi_item_view_set_on_activate(tokotachi_item_view* self, tokotachi_notify_fn fn, void* user_data, tokotachi_destroy_fn destroy);
TOKOTACHI_C_API tokotachi_column* tokotachi_item_view_add_column(tokotachi_item_view* self, const char* title, float width);
TOKOTACHI_C_API void tokotachi_item_view_remove_column(tokotachi_item_view* self, tokotachi_column* column);
TOKOTACHI_C_API bool tokotachi_item_view_is_selected(tokotachi_item_view* self, tokotachi_model_index index);
TOKOTACHI_C_API void tokotachi_item_view_select(tokotachi_item_view* self, tokotachi_model_index index, bool selected);
TOKOTACHI_C_API void tokotachi_item_view_select_all(tokotachi_item_view* self);
TOKOTACHI_C_API void tokotachi_item_view_clear_selection(tokotachi_item_view* self);
TOKOTACHI_C_API void tokotachi_item_view_scroll_to(tokotachi_item_view* self, tokotachi_model_index index);
TOKOTACHI_C_API tokotachi_model_index tokotachi_item_view_index_at(tokotachi_item_view* self, tokotachi_point p);

/* ======== ListView(include/tokotachi/views/list_view.hpp。基底は ItemView) ======== */
TOKOTACHI_C_API tokotachi_list_view* tokotachi_list_view_new(tokotachi_widget* parent);

/* ======== TreeView(include/tokotachi/views/tree_view.hpp。基底は ItemView) ======== */
TOKOTACHI_C_API tokotachi_tree_view* tokotachi_tree_view_new(tokotachi_widget* parent);
TOKOTACHI_C_API float tokotachi_tree_view_get_indentation(tokotachi_tree_view* self);
TOKOTACHI_C_API void tokotachi_tree_view_set_indentation(tokotachi_tree_view* self, float value);
TOKOTACHI_C_API void tokotachi_tree_view_expand(tokotachi_tree_view* self, tokotachi_model_index index);
TOKOTACHI_C_API void tokotachi_tree_view_collapse(tokotachi_tree_view* self, tokotachi_model_index index);
TOKOTACHI_C_API bool tokotachi_tree_view_is_expanded(tokotachi_tree_view* self, tokotachi_model_index index);
TOKOTACHI_C_API void tokotachi_tree_view_expand_all(tokotachi_tree_view* self);
TOKOTACHI_C_API void tokotachi_tree_view_collapse_all(tokotachi_tree_view* self);

/* ======== Variable(include/tokotachi/core/variable.hpp。基底は Object) ======== */
/* 値を 1 つ持つオブジェクト(docs/adr/0016)。プロパティ "value" を tokotachi_bind で結ぶ。
 * 型と合わない get・set は失敗する */
/* 値の型が type の Variable(C++ の Variable<T>)を作る。値は T() */
TOKOTACHI_C_API tokotachi_variable* tokotachi_variable_new(tokotachi_object* owner, tokotachi_variable_type type);
TOKOTACHI_C_API tokotachi_variable_type tokotachi_variable_get_type(tokotachi_variable* variable);
TOKOTACHI_C_API bool tokotachi_variable_get_bool(tokotachi_variable* variable);
TOKOTACHI_C_API void tokotachi_variable_set_bool(tokotachi_variable* variable, bool value);
TOKOTACHI_C_API int tokotachi_variable_get_int(tokotachi_variable* variable);
TOKOTACHI_C_API void tokotachi_variable_set_int(tokotachi_variable* variable, int value);
TOKOTACHI_C_API float tokotachi_variable_get_float(tokotachi_variable* variable);
TOKOTACHI_C_API void tokotachi_variable_set_float(tokotachi_variable* variable, float value);
TOKOTACHI_C_API double tokotachi_variable_get_double(tokotachi_variable* variable);
TOKOTACHI_C_API void tokotachi_variable_set_double(tokotachi_variable* variable, double value);
TOKOTACHI_C_API char* tokotachi_variable_get_string(tokotachi_variable* variable);
TOKOTACHI_C_API void tokotachi_variable_set_string(tokotachi_variable* variable, const char* value);
TOKOTACHI_C_API void tokotachi_variable_set_on_set(tokotachi_variable* variable, tokotachi_notify_fn fn, void* user_data, tokotachi_destroy_fn destroy);
TOKOTACHI_C_API void tokotachi_variable_set_on_change(tokotachi_variable* variable, tokotachi_notify_fn fn, void* user_data, tokotachi_destroy_fn destroy);

/* ======== Task(include/tokotachi/core/task.hpp。基底は Object) ======== */
/* 別のスレッドで行う、終わりのある仕事(docs/adr/0029) */
TOKOTACHI_C_API bool tokotachi_task_get_running(tokotachi_task* self);
TOKOTACHI_C_API double tokotachi_task_get_progress(tokotachi_task* self);
TOKOTACHI_C_API bool tokotachi_task_get_cancelled(tokotachi_task* self);
TOKOTACHI_C_API char* tokotachi_task_get_error(tokotachi_task* self);
TOKOTACHI_C_API void tokotachi_task_set_on_progress(tokotachi_task* self, tokotachi_notify_fn fn, void* user_data, tokotachi_destroy_fn destroy);
TOKOTACHI_C_API void tokotachi_task_set_on_finish(tokotachi_task* self, tokotachi_notify_fn fn, void* user_data, tokotachi_destroy_fn destroy);
TOKOTACHI_C_API void tokotachi_task_cancel(tokotachi_task* self);
/* 結果を持たない Task(C++ の Task<void>) */
TOKOTACHI_C_API tokotachi_task* tokotachi_task_new(tokotachi_object* owner);
/* work(context, user_data) を別のスレッドで呼ぶ。動いていれば何もせず false。
 * destroy(user_data) は仕事が終わった後に呼ぶ(どのスレッドかは決まっていない) */
TOKOTACHI_C_API bool tokotachi_task_start(tokotachi_task* task, tokotachi_task_fn work, void* user_data, tokotachi_destroy_fn destroy);
TOKOTACHI_C_API bool tokotachi_task_context_cancel_requested(tokotachi_task_context* context);
TOKOTACHI_C_API void tokotachi_task_context_report_progress(tokotachi_task_context* context, double value);
/* 仕事を失敗として終える(work が戻った後、error に message が入る) */
TOKOTACHI_C_API void tokotachi_task_context_fail(tokotachi_task_context* context, const char* message);

/* ======== CustomModel(include/tokotachi/views/item_model.hpp。基底は ItemModel) ======== */
/* 関数で中身を答えるモデル(C++ では ItemModel を継承して作るもの) */
/* 関数で中身を答えるモデル。callbacks は写す。モデルが破棄されるときに destroy(user_data) を呼ぶ */
TOKOTACHI_C_API tokotachi_custom_model* tokotachi_custom_model_new(tokotachi_object* owner, const tokotachi_model_callbacks* callbacks, void* user_data, tokotachi_destroy_fn destroy);
/* 加えた後に呼ぶ */
TOKOTACHI_C_API void tokotachi_custom_model_notify_rows_inserted(tokotachi_custom_model* model, tokotachi_model_index parent, int first, int count);
/* 除く前に呼ぶ */
TOKOTACHI_C_API void tokotachi_custom_model_notify_rows_removing(tokotachi_custom_model* model, tokotachi_model_index parent, int first, int count);
TOKOTACHI_C_API void tokotachi_custom_model_notify_data_changed(tokotachi_custom_model* model, tokotachi_model_index index);
TOKOTACHI_C_API void tokotachi_custom_model_notify_layout_changed(tokotachi_custom_model* model);
TOKOTACHI_C_API void tokotachi_custom_model_notify_reset(tokotachi_custom_model* model);

#ifdef __cplusplus
}
#endif

#endif
