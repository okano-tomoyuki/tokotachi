#!/usr/bin/env python3
"""C API の生成器(docs/adr/0041)。api.py から次の 3 つを作る:

  bindings/c/include/tokotachi/tokotachi.h   C のヘッダ
  bindings/c/src/generated.cpp               プロパティ・メソッドの C の関数(C++ の実装)
  bindings/python/src/tokotachi/_api.py      Python の列挙・ctypes の宣言・クラス

  python3 bindings/c/generate.py           作る
  python3 bindings/c/generate.py --check   作ったものが api.py と合っているか確かめる(CI)
"""

import os
import re
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.dirname(os.path.dirname(HERE))
sys.path.insert(0, HERE)

import api  # noqa: E402

HEADER = os.path.join(HERE, 'include', 'tokotachi', 'tokotachi.h')
SOURCE = os.path.join(HERE, 'src', 'generated.cpp')
PYTHON = os.path.join(ROOT, 'bindings', 'python', 'src', 'tokotachi', '_api.py')

NOTICE = 'api.py から generate.py で作ったファイル。手で書き換えない'


def snake(name):
    """PascalCase → snake_case(TextInputBase → text_input_base)。"""
    return re.sub(r'(?<!^)(?=[A-Z])', '_', name).lower()


ENUMS = {e.name: e for e in api.ENUMS}
CLASSES = {c.name: c for c in api.CLASSES}


def c_class(name):
    return 'tokotachi_' + snake(name)


# ---------------------------------------------------------------------------------------------------------------------
# 型
# ---------------------------------------------------------------------------------------------------------------------

class Type:
    """C・C++・Python の間の値の受け渡し。"""

    shape = 'value'  # value: 戻り値で返す / buffer: 呼んだ側の配列に書く / out: 呼んだ側の構造体に書く
    bindable = True

    def __init__(self, c, ct, cpp_in='{}', cpp_out='{}', err='{}', ct_ret=None, c_ret=None, py_in='{}',
                 py_out='{}'):
        self.c = c
        self.ct = ct
        self._cpp_in = cpp_in
        self._cpp_out = cpp_out
        self.err = err
        self.ct_ret = ct_ret or ct
        self.c_ret = c_ret or c
        self._py_in = py_in
        self._py_out = py_out

    def c_params(self, name):
        return [(self.c, name)]

    def ct_params(self):
        return [self.ct]

    def cpp_in(self, name):
        return self._cpp_in.format(name)

    def cpp_out(self, expr):
        return self._cpp_out.format(expr)

    def py_in(self, expr):
        return self._py_in.format(expr)

    def py_out(self, expr):
        return self._py_out.format(expr)


class StringsType(Type):
    """文字列の並び: 返すときは NULL で終わる char**、渡すときは配列と数。"""

    def __init__(self, cpp_in='c::strv({0}, {0}_count)', cpp_out='c::dup_strv({})', py_in='_in_strings({})',
                 py_out='_out_strings({})'):
        super().__init__('const char* const*', 'POINTER(c_char_p)', cpp_in, cpp_out, 'nullptr', 'c_void_p', 'char**',
                         py_in, py_out)

    def c_params(self, name):
        return [('const char* const*', name), ('size_t', name + '_count')]

    def ct_params(self):
        return ['POINTER(c_char_p)', 'c_size_t']

    def py_in(self, expr):
        return '*' + self._py_in.format(expr)


class BufferType(Type):
    """読み取りだけの並び: size_t f(self, T* out, size_t capacity) で、全体の数を返す。"""

    shape = 'buffer'

    def __init__(self, element, ct_element, fill, py_get, cpp_in=None):
        super().__init__('const %s*' % element, 'POINTER(%s)' % ct_element)
        self.element = element
        self.ct_element = ct_element
        self.fill = fill
        self.py_get = py_get
        self.buffer_in = cpp_in

    def c_params(self, name):
        return [('const %s*' % self.element, name), ('size_t', name + '_count')]

    def ct_params(self):
        return ['POINTER(%s)' % self.ct_element, 'c_size_t']

    def cpp_in(self, name):
        return self.buffer_in.format(name)

    def py_in(self, expr):
        return '*_in_floats(%s)' % expr


def simple(c, ct):
    return Type(c, ct)


def struct(c, ct, key):
    return Type(c, ct, 'c::from_c({})', 'c::to_c({})', c + '()', py_in='_in_%s({})' % key,
                py_out='_out_%s({})' % key)


TYPES = {
    'void': Type('void', 'None'),
    'bool': simple('bool', 'c_bool'),
    'int': simple('int', 'c_int'),
    'float': simple('float', 'c_float'),
    'double': simple('double', 'c_double'),
    'size_t': simple('size_t', 'c_size_t'),
    'string': Type('const char*', 'c_char_p', 'c::str({})', 'c::dup({})', 'nullptr', 'c_void_p', 'char*',
                   '_in_string({})', '_out_string({})'),
    'strings': StringsType(),
    'file_filters': StringsType('c::file_filters_in({0}, {0}_count)', 'c::file_filters_out({})'),
    'floats': BufferType('float', 'c_float', 'c::fill_floats', '_get_floats', 'c::floats({0}, {0}_count)'),
    'ModelIndexes': BufferType('tokotachi_model_index', 'CModelIndex', 'c::fill_indexes', '_get_indexes'),
    'Point': struct('tokotachi_point', 'CPoint', 'Point'),
    'Size': struct('tokotachi_size', 'CSize', 'Size'),
    'Rect': struct('tokotachi_rect', 'CRect', 'Rect'),
    'Color': struct('tokotachi_color', 'CColor', 'Color'),
    'Margins': struct('tokotachi_margins', 'CMargins', 'Margins'),
    'Font': struct('tokotachi_font', 'CFont', 'Font'),
    'Shortcut': struct('tokotachi_shortcut', 'CShortcut', 'Shortcut'),
    'ThemeColor': struct('tokotachi_theme_color', 'CThemeColor', 'ThemeColor'),
    'ModelIndex': struct('tokotachi_model_index', 'CModelIndex', 'ModelIndex'),
    'Placement': struct('tokotachi_placement', 'CPlacement', 'Placement'),
    'Image': Type('const tokotachi_image*', 'c_void_p', 'c::from_c({})', 'c::to_c_image({})', 'nullptr', 'c_void_p',
                  'tokotachi_image*', '_in_Image({})', '_out_Image({})'),
    'Bitmap': Type('tokotachi_bitmap*', 'c_void_p', '', 'c::to_c_bitmap({})', 'nullptr', py_out='_out_Bitmap({})'),
}


class ThemeType(Type):
    """Theme: 大きいので、呼んだ側の構造体に書く・構造体へのポインタで渡す。"""

    shape = 'out'

    def __init__(self):
        super().__init__('const tokotachi_theme*', 'POINTER(CTheme)', 'c::from_c({})', '', '', py_in='_in_Theme({})')


TYPES['Theme'] = ThemeType()

RAW_CTYPES = {
    'int': 'c_int',
    'char**': 'c_void_p',
    'void*': 'c_void_p',
    'const void*': 'c_void_p',
    'const char*': 'c_char_p',
    'uint32_t*': 'POINTER(c_uint32)',
    'tokotachi_theme*': 'POINTER(CTheme)',
    'const tokotachi_color*': 'POINTER(CColor)',
    'const tokotachi_point*': 'POINTER(CPoint)',
    'const tokotachi_paint*': 'POINTER(CPaint)',
    'const tokotachi_stroke_style*': 'POINTER(CStrokeStyle)',
    'const tokotachi_dialog_button*': 'POINTER(c_int)',
    'const tokotachi_model_callbacks*': 'POINTER(CModelCallbacks)',
    'tokotachi_error_fn': 'c_void_p',
    'tokotachi_destroy_fn': 'c_void_p',
    'tokotachi_callback_fn': 'c_void_p',
    'tokotachi_notify_fn': 'c_void_p',
    'tokotachi_task_fn': 'c_void_p',
}
for handle in ['image', 'bitmap', 'canvas', 'path', 'text_layout', 'task_context']:
    RAW_CTYPES['tokotachi_%s*' % handle] = 'c_void_p'
    RAW_CTYPES['const tokotachi_%s*' % handle] = 'c_void_p'


def type_of(name):
    if name in TYPES:
        return TYPES[name]
    if name in ENUMS:
        e = ENUMS[name]
        c = c_class(name)
        cpp = e.cpp or 'int'
        t = Type(c, 'c_int', 'c::enum_in<%s>({}, %d)' % (cpp, len(e.values)), 'static_cast<%s>({})' % c,
                 'static_cast<%s>(0)' % c, py_in='int({})', py_out='%s({})' % name)
        return t
    if name.startswith('obj:'):
        cls = name[4:]
        cpp = CLASSES[cls].cpp
        t = Type(c_class(cls) + '*', 'c_void_p', 'c::arg_as<%s>({}, __func__)' % cpp, 'c::handle({})', 'nullptr',
                 py_in="_in_obj({}, '%s')" % cls, py_out='_wrap({})')
        t.bindable = False
        return t
    if name.startswith('objs:'):
        cls = name[5:]
        t = BufferType(c_class(cls) + '*', 'c_void_p', 'c::fill_objects', '_get_objects')
        t.c = c_class(cls) + '**'
        t.bindable = False
        return t
    if name.startswith('raw:'):
        c = name[4:]
        return Type(c, RAW_CTYPES[c])
    raise KeyError(name)


# ---------------------------------------------------------------------------------------------------------------------
# 関数の並び
# ---------------------------------------------------------------------------------------------------------------------

class Function:
    def __init__(self, name, ret, params, doc='', body=None, kind='manual'):
        self.name = name
        self.ret = ret  # 型の名前
        self.params = params  # [(C の型, 名前)]
        self.ctypes = []  # ctypes の型
        self.doc = doc
        self.body = body  # C++ の本体(None なら手で書く)
        self.kind = kind

    def c_ret(self):
        t = type_of(self.ret)
        if t.shape == 'buffer':
            return 'size_t'
        if t.shape == 'out':
            return 'void'
        return t.c_ret

    def ct_ret(self):
        t = type_of(self.ret)
        if t.shape == 'buffer':
            return 'c_size_t'
        if t.shape == 'out':
            return 'None'
        return t.ct_ret

    def declaration(self):
        params = ', '.join(join_decl(c, n) for c, n in self.params) or 'void'
        return '%s %s(%s)' % (self.c_ret(), self.name, params)


def join_decl(c, n):
    return c + ' ' + n


def handler_c_type(kind):
    return 'tokotachi_%s_fn' % kind


HANDLER_MAKERS = {
    'notify': 'c::notify_handler',
    'mouse': 'c::mouse_handler',
    'key': 'c::key_handler',
    'wheel': 'c::wheel_handler',
    'paint': 'c::paint_handler',
    'close': 'c::close_handler',
}



def param_list(params):
    """(型, 名前[, 既定値]) → C の引数と ctypes の型。"""
    c, ct = [], []
    for p in params:
        t = type_of(p[0])
        c += t.c_params(p[1])
        ct += t.ct_params()
    return c, ct


def self_expr(cls):
    return 'c::self_as<%s>(self, __func__)' % cls.cpp


def catch(ret_type):
    t = type_of(ret_type)
    if ret_type == 'void' or t.shape == 'out':
        return 'TOKOTACHI_C_CATCH()'
    if t.shape == 'buffer':
        return 'TOKOTACHI_C_CATCH(0)'
    return 'TOKOTACHI_C_CATCH(%s)' % t.err


def class_functions(cls):
    """クラスの C の関数(生成するもの・手で書くもの)。"""
    prefix = c_class(cls.name)
    self_param = (prefix + '*', 'self')
    out = []

    if cls.new is not None:
        params, ct = param_list(cls.new)
        call = cls.new_cpp or 'new %s(%s)' % (cls.cpp, ', '.join(type_of(p[0]).cpp_in(p[1]) for p in cls.new))
        f = Function(prefix + '_new', 'obj:' + cls.name, params, kind='new',
                     body='return c::handle(%s);' % call)
        f.ctypes = ct
        out.append(f)

    for m in cls.members:
        if isinstance(m, api.P):
            t = type_of(m.type)
            get = '%s->%s.get()' % (self_expr(cls), m.name)
            if t.shape == 'buffer':
                f = Function('%s_get_%s' % (prefix, m.name), m.type,
                             [self_param, (t.c if t.c.endswith('**') else t.element + '*', 'out'),
                              ('size_t', 'capacity')],
                             m.doc, 'return %s(%s, out, capacity);' % (t.fill, get), kind='get')
                f.ctypes = ['c_void_p', 'POINTER(%s)' % t.ct_element, 'c_size_t']
            elif t.shape == 'out':
                f = Function('%s_get_%s' % (prefix, m.name), m.type, [self_param, ('tokotachi_theme*', 'out')],
                             m.doc, '*out = c::to_c(%s);' % get, kind='get')
                f.ctypes = ['c_void_p', 'POINTER(CTheme)']
            else:
                f = Function('%s_get_%s' % (prefix, m.name), m.type, [self_param], m.doc,
                             'return %s;' % t.cpp_out(get), kind='get')
                f.ctypes = ['c_void_p']
            f.prop = m
            out.append(f)
            if not m.ro:
                params = [self_param] + t.c_params('value')
                f = Function('%s_set_%s' % (prefix, m.name), 'void', params, '',
                             '%s->%s = %s;' % (self_expr(cls), m.name, t.cpp_in('value')), kind='set')
                f.ctypes = ['c_void_p'] + t.ct_params()
                f.prop = m
                out.append(f)
        elif isinstance(m, api.H):
            fn_type = handler_c_type(m.kind)
            params = [self_param, (fn_type, 'fn'), ('void*', 'user_data'), ('tokotachi_destroy_fn', 'destroy')]
            body = ('%s handler = %s(fn, user_data, destroy);\n'
                    '%s->%s = handler;' % (HANDLER_TYPES[m.kind], HANDLER_MAKERS[m.kind], self_expr(cls), m.name))
            f = Function('%s_set_%s' % (prefix, m.name), 'void', params, m.doc, body, kind='handler')
            f.ctypes = ['c_void_p', 'c_void_p', 'c_void_p', 'c_void_p']
            f.handler = m
            out.append(f)
        elif isinstance(m, api.M):
            params, ct = param_list(m.params)
            args = {p[1]: type_of(p[0]).cpp_in(p[1]) for p in m.params}
            if m.cpp:
                call = m.cpp.format(self=self_expr(cls), **args)
            else:
                call = '%s->%s(%s)' % (self_expr(cls), m.name, ', '.join(args[p[1]] for p in m.params))
            rt = type_of(m.ret)
            body = call + ';' if m.ret == 'void' else 'return %s;' % rt.cpp_out(call)
            f = Function('%s_%s' % (prefix, m.c_name or m.name), m.ret, [self_param] + params, m.doc, body,
                         kind='method')
            f.ctypes = ['c_void_p'] + ct
            f.method = m
            out.append(f)
        elif isinstance(m, api.F):
            out.append(manual_function(m))
        else:
            raise TypeError(m)
    return out


HANDLER_TYPES = {
    'notify': 'tokotachi::NotifyHandler',
    'mouse': 'tokotachi::MouseHandler',
    'key': 'tokotachi::KeyHandler',
    'wheel': 'tokotachi::WheelHandler',
    'paint': 'tokotachi::PaintHandler',
    'close': 'tokotachi::CloseHandler',
}


def manual_function(m):
    params, ct = param_list(m.params)
    f = Function(m.name, m.ret, params, m.doc)
    f.ctypes = ct
    return f


# ---------------------------------------------------------------------------------------------------------------------
# C のヘッダ
# ---------------------------------------------------------------------------------------------------------------------

HEADER_TOP = r'''/* Tokotachi の C API(docs/adr/0041・docs/bindings/c-api.md)。
 * %(notice)s(bindings/c/api.py・bindings/c/generate.py)。
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
#define TOKOTACHI_C_VERSION_MAJOR %(major)s
#define TOKOTACHI_C_VERSION_MINOR %(minor)s
#define TOKOTACHI_C_VERSION_PATCH %(patch)s
'''

HEADER_STRUCTS = r'''
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

#define TOKOTACHI_COLOR_ROLE_COUNT %(role_count)d

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
%(aliases)s
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
'''

HEADER_BOTTOM = r'''
#ifdef __cplusplus
}
#endif

#endif
'''


def doc_comment(doc, indent=''):
    if not doc:
        return ''
    lines = doc.split('\n')
    if len(lines) == 1:
        return '%s/* %s */\n' % (indent, lines[0])
    return '%s/* %s\n%s */\n' % (indent, lines[0], '\n'.join('%s * %s' % (indent, line) for line in lines[1:]))


def version():
    text = open(os.path.join(ROOT, 'include', 'tokotachi', 'core', 'version.hpp'), encoding='utf-8').read()
    return {k: re.search(r'#define TOKOTACHI_VERSION_%s (\d+)' % k.upper(), text).group(1)
            for k in ('major', 'minor', 'patch')}


def all_functions():
    """(見出し, 関数の並び)の並び。"""
    groups = [('全体', [manual_function(m) for m in api.CORE])]
    for cls in api.CLASSES:
        title = '%s(include/tokotachi/%s' % (cls.name, cls.header)
        title += '。基底は %s)' % cls.base if cls.base else ')'
        groups.append((title, class_functions(cls), cls))
    return groups


def generate_header():
    out = [HEADER_TOP % dict(notice=NOTICE, **version())]
    out.append('\n/* ---- 列挙(値は C++ の enum class と同じ順) ---- */\n')
    for e in api.ENUMS:
        name = c_class(e.name)
        out.append(doc_comment(e.doc or ('C++ の %s' % e.cpp)))
        out.append('typedef enum %s\n{\n' % name)
        prefix = 'TOKOTACHI_' + snake(e.name).upper()
        for i, v in enumerate(e.values):
            out.append('    %s_%s = %d,\n' % (prefix, v, i))
        out.append('} %s;\n\n' % name)
    aliases = ''.join('typedef tokotachi_object %s;\n' % c_class(c.name) for c in api.CLASSES if c.name != 'Object')
    out.append(HEADER_STRUCTS % dict(role_count=len(ENUMS['ColorRole'].values), aliases=aliases))
    for group in all_functions():
        title, functions = group[0], group[1]
        out.append('\n/* ======== %s ======== */\n' % title)
        if len(group) > 2 and group[2].doc:
            out.append(doc_comment(group[2].doc))
        for f in functions:
            out.append(doc_comment(f.doc))
            out.append('TOKOTACHI_C_API %s;\n' % f.declaration())
    out.append(HEADER_BOTTOM)
    return ''.join(out)


# ---------------------------------------------------------------------------------------------------------------------
# C++ の実装
# ---------------------------------------------------------------------------------------------------------------------

def depth(cls):
    d = 0
    while cls.base:
        cls = CLASSES[cls.base]
        d += 1
    return d


DETECT = {
    'Variable': 'c::is_variable(o)',
    'CustomModel': 'dynamic_cast<c::CallbackModel*>(o)',
}


def generate_source():
    out = ['// %s。\n' % NOTICE,
           '#include "common.hpp"\n\n',
           'namespace c = tokotachi::c;\n\n',
           'extern "C" {\n']
    for group in all_functions():
        for f in group[1]:
            if f.body is None:
                continue
            out.append('\nTOKOTACHI_C_API %s\n{\n    try\n    {\n' % f.declaration())
            for line in f.body.split('\n'):
                out.append('        %s\n' % line)
            out.append('    }\n    %s\n}\n' % catch(f.ret))

    # tokotachi_object_class_name: 深いクラスから順に確かめる
    out.append('\nTOKOTACHI_C_API const char* tokotachi_object_class_name(tokotachi_object* object)\n{\n')
    out.append('    tokotachi::Object* o = reinterpret_cast<tokotachi::Object*>(object);\n')
    out.append('    if (!o)\n    {\n        return nullptr;\n    }\n')
    ordered = sorted((c for c in api.CLASSES if c.name != 'Object'), key=lambda c: -depth(c))
    for cls in ordered:
        test = DETECT.get(cls.name, 'dynamic_cast<%s*>(o)' % cls.cpp)
        out.append('    if (%s)\n    {\n        return "%s";\n    }\n' % (test, cls.name))
    out.append('    return "Object";\n}\n')
    out.append('\n}  // extern "C"\n')

    # tokotachi_bind のための、名前からプロパティへの表
    out.append('\nvoid tokotachi::c::register_generated_properties()\n{\n')
    for cls in api.CLASSES:
        for m in cls.members:
            if isinstance(m, api.P) and m.bindable and type_of(m.type).bindable:
                out.append('    TOKOTACHI_C_ADD_PROPERTY("%s", &%s::%s);\n' % (m.name, cls.cpp, m.name))
    out.append('}\n')

    # 列挙の値が C++ と同じか
    out.append('\n// 列挙の値が C++ の enum class と同じ順であること。\n')
    for e in api.ENUMS:
        if e.cpp:
            prefix = 'TOKOTACHI_' + snake(e.name).upper()
            for i, v in enumerate(e.values):
                cpp_value = v
                out.append('static_assert(static_cast<int>(%s::%s) == %s_%s, "%s");\n' %
                           (e.cpp, cpp_value, prefix, v, e.name))
            out.append('static_assert(static_cast<int>(%s::%s) == %d, "%s の数");\n' %
                       (e.cpp, e.values[-1], len(e.values) - 1, e.name))
    return ''.join(out)


# ---------------------------------------------------------------------------------------------------------------------
# Python
# ---------------------------------------------------------------------------------------------------------------------

def py_default(p):
    return p[2] if len(p) > 2 and p[2] is not None else None


def generate_python():
    out = ['# %s。\n' % NOTICE,
           '# flake8: noqa\n',
           'from ctypes import POINTER, c_bool, c_char_p, c_double, c_float, c_int, c_size_t, c_uint32, c_void_p\n',
           'import enum\n\n',
           'from ._core import *  # noqa: F401,F403\n',
           'from ._core import _lib, _ObjectBase, _Property, _Handler, _proto, _register, _wrap, _repin\n',
           'from ._core import (_in_string, _out_string, _in_strings, _out_strings, _in_floats, _get_floats,\n'
           '                    _get_indexes, _get_objects, _get_out, _in_obj, _in_Image, _out_Image, _out_Bitmap,\n'
           '                    _in_Theme, _out_Theme)\n',
           'from ._core import (_in_Point, _out_Point, _in_Size, _out_Size, _in_Rect, _out_Rect, _in_Color, _out_Color,\n'
           '                    _in_Margins, _out_Margins, _in_Font, _out_Font, _in_Shortcut, _out_Shortcut,\n'
           '                    _in_ThemeColor, _out_ThemeColor, _in_ModelIndex, _out_ModelIndex, _in_Placement,\n'
           '                    _out_Placement)\n',
           'from ._core import (_ApplicationMixin, _MessageDialogMixin, _VariableMixin, _TaskMixin, _CustomModelMixin)\n',
           ]
    names = []
    out.append('\n# ---- 列挙 ----\n')
    for e in api.ENUMS:
        out.append('\n\nclass %s(enum.IntEnum):\n' % e.name)
        out.append('    """%s"""\n\n' % (e.doc or ('C++ の %s' % e.cpp)))
        for i, v in enumerate(e.values):
            out.append('    %s = %d\n' % (v, i))
        names.append(e.name)

    out.append('\n\n# ---- 関数の宣言(ctypes) ----\n\n')
    for group in all_functions():
        for f in group[1]:
            out.append("_proto('%s', %s, [%s])\n" % (f.name, f.ct_ret(), ', '.join(f.ctypes)))
    # tokotachi_object_class_name は CORE にある

    out.append('\n\n# ---- クラス ----\n')
    for cls in api.CLASSES:
        bases = []
        if cls.py_mixin:
            bases.append(cls.py_mixin)
        bases.append(cls.base if cls.base else '_ObjectBase')
        out.append('\n\nclass %s(%s):\n' % (cls.name, ', '.join(bases)))
        doc = cls.doc.replace('\n', ' ') if cls.doc else ''
        out.append('    """C++ の %s(include/tokotachi/%s)。%s"""\n' % (cls.cpp, cls.header, doc))
        functions = class_functions(cls)
        body = []
        if cls.new is not None and cls.py_init:
            params = []
            for p in cls.new:
                d = py_default(p)
                params.append(p[1] if d is None else '%s=%s' % (p[1], d))
            args = ', '.join(type_of(p[0]).py_in(p[1]) for p in cls.new)
            body.append('\n    def __init__(self%s):\n' % ''.join(', ' + p for p in params))
            body.append('        self._attach(_lib.%s_new(%s), True)\n' % (c_class(cls.name), args))
            for p in cls.new:
                if p[0].startswith('obj:'):
                    body.append('        _repin(%s)\n' % p[1])
            body.append('\n')
        if cls.new is None and cls.base and not cls.py_mixin:
            body.append('    __init__ = _ObjectBase.__init__  # 直接は作れない\n\n')
        props = {}
        for f in functions:
            if f.kind in ('get', 'set'):
                props.setdefault(f.prop.name, {})[f.kind] = f
        for name, fs in props.items():
            get = fs['get']
            t = type_of(get.prop.type)
            if t.shape == 'buffer':
                getter = "lambda h: %s(_lib.%s, h)" % (t.py_get, get.name)
            elif t.shape == 'out':
                getter = "lambda h: _out_Theme(_get_out(_lib.%s, h, CTheme))" % get.name
            else:
                getter = 'lambda h: %s' % t.py_out('_lib.%s(h)' % get.name)
            setter = 'None'
            if 'set' in fs:
                setter = 'lambda h, v: _lib.%s(h, %s)' % (fs['set'].name, t.py_in('v'))
            repin = ', repin=True' if get.prop.type.startswith('obj:') and 'set' in fs else ''
            body.append('    %s = _Property(%s, %s%s)\n' % (name, getter, setter, repin))
        for f in functions:
            if f.kind == 'handler':
                body.append("    %s = _Handler('%s', '%s')\n" % (f.handler.name, f.handler.kind, f.name))
        for f in functions:
            if f.kind != 'method':
                continue
            m = f.method
            params = []
            for p in m.params:
                d = py_default(p)
                params.append(p[1] if d is None else '%s=%s' % (p[1], d))
            args = ''.join(', ' + type_of(p[0]).py_in(p[1]) for p in m.params)
            call = '_lib.%s(self._h%s)' % (f.name, args)
            body.append('\n    def %s(self%s):\n' % (m.name, ''.join(', ' + p for p in params)))
            if m.doc:
                body.append('        """%s"""\n' % m.doc.replace('\n', ' '))
            rt = type_of(m.ret)
            obj_params = [p[1] for p in m.params if p[0].startswith('obj:')]
            if m.ret == 'void':
                body.append('        %s\n' % call)
                for p in obj_params:
                    body.append('        _repin(%s)\n' % p)
            else:
                body.append('        result = %s\n' % rt.py_out(call))
                for p in obj_params:
                    body.append('        _repin(%s)\n' % p)
                if m.ret.startswith('obj:'):
                    body.append('        _repin(result)\n')
                body.append('        return result\n')
        if not body:
            body.append('    pass\n')
        out.extend(body)
        out.append('\n\n_register(%s)\n' % cls.name)
        names.append(cls.name)
    out.append('\n__all__ = [%s]\n' % ', '.join("'%s'" % n for n in names))
    return ''.join(out)


# ---------------------------------------------------------------------------------------------------------------------

def main():
    check = '--check' in sys.argv
    outputs = [(HEADER, generate_header()), (SOURCE, generate_source()), (PYTHON, generate_python())]
    stale = []
    for path, text in outputs:
        old = open(path, encoding='utf-8').read() if os.path.exists(path) else None
        if old == text:
            continue
        if check:
            stale.append(os.path.relpath(path, ROOT))
        else:
            os.makedirs(os.path.dirname(path), exist_ok=True)
            with open(path, 'w', encoding='utf-8', newline='\n') as f:
                f.write(text)
            print('generated', os.path.relpath(path, ROOT))
    if stale:
        print('api.py と合っていないファイル(python3 bindings/c/generate.py で作り直す):', *stale, sep='\n  ')
        return 1
    return 0


if __name__ == '__main__':
    sys.exit(main())
