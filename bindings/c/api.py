"""C API の仕様(docs/adr/0065)。generate.py がここから、C のヘッダ・C++ の実装・Python の ctypes の宣言を作る。

型の名前(generate.py の TYPES):
  void・bool・int・float・double・size_t・string(UTF-8)・strings(文字列の並び)・floats・Point・Size・Rect・Color・Margins・
  Font・Shortcut・ThemeColor・ModelIndex・ModelIndexes・Placement・Image・Bitmap・Theme・列挙の名前(Align 等)・
  obj:クラス(Object の派生へのポインタ)・objs:クラス(その並び。読み取りだけ)・raw:C の型(変換しない)。

クラス(cls)の要素:
  P(名前, 型, ro=...)            プロパティ(C++ の Property・ReadOnlyProperty)。get・set の関数になる
  H(名前, 種類)                  ハンドラのプロパティ(notify・mouse・key・wheel・paint・close)。set の関数になる
  M(名前, 戻り値の型, 引数, ...) メソッド。引数は (型, 名前[, Python の既定値]) の並び
  new=引数                       コンストラクタ(tokotachi_<クラス>_new)
  manual=[...]                   手で書く関数(src/manual.cpp)の宣言(F(...))
"""

from dataclasses import dataclass, field
from typing import List, Optional, Tuple


@dataclass
class Enum:
    name: str  # Python の名前(Align)。C は tokotachi_align・TOKOTACHI_ALIGN_START
    cpp: Optional[str]  # C++ の型(None なら C だけのもの)
    values: List[str]
    doc: str = ''


@dataclass
class P:
    name: str
    type: str
    ro: bool = False
    doc: str = ''
    bindable: bool = True  # tokotachi_bind の名前で結べるか


@dataclass
class H:
    name: str
    kind: str
    doc: str = ''


@dataclass
class M:
    name: str
    ret: str
    params: List[Tuple] = field(default_factory=list)
    cpp: Optional[str] = None  # C++ の式({self} と引数の名前で書く)。無ければ {self}->名前(引数)
    doc: str = ''
    c_name: Optional[str] = None  # C の関数の名前の後ろ(既定は名前)


@dataclass
class F:
    """手で書く関数(名前は完全なもの)。"""

    name: str
    ret: str
    params: List[Tuple] = field(default_factory=list)
    doc: str = ''


@dataclass
class Cls:
    name: str  # Python・C の名前(Label → tokotachi_label)
    cpp: str  # C++ の型
    base: Optional[str]
    header: str
    members: list = field(default_factory=list)
    new: Optional[List[Tuple]] = None
    new_cpp: Optional[str] = None
    doc: str = ''
    py_mixin: Optional[str] = None  # Python のクラスに混ぜる手書きのクラス(_core.py)
    py_init: bool = True  # Python の __init__ を作るか(False なら混ぜるクラスが持つ)
    snake: Optional[str] = None


KEYS = (
    ['UNKNOWN', 'BACKSPACE', 'TAB', 'RETURN', 'ESCAPE', 'SPACE', 'DEL', 'INSERT', 'LEFT', 'RIGHT', 'UP', 'DOWN', 'HOME',
     'END', 'PAGE_UP', 'PAGE_DOWN']
    + [chr(c) for c in range(ord('A'), ord('Z') + 1)]
    + ['DIGIT_%d' % i for i in range(10)]
    + ['F%d' % i for i in range(1, 13)]
    + ['SHIFT', 'CONTROL', 'ALT', 'META']
)

ENUMS = [
    Enum('MouseButton', 'tokotachi::MouseButton', ['NONE', 'LEFT', 'MIDDLE', 'RIGHT']),
    Enum('Key', 'tokotachi::Key', KEYS),
    Enum('FocusPolicy', 'tokotachi::FocusPolicy', ['NONE', 'TAB', 'CLICK', 'TAB_AND_CLICK']),
    Enum('MouseCursor', 'tokotachi::MouseCursor',
         ['ARROW', 'IBEAM', 'POINTING_HAND', 'WAIT', 'CROSSHAIR', 'SIZE_HORIZONTAL', 'SIZE_VERTICAL']),
    Enum('Orientation', 'tokotachi::Orientation', ['HORIZONTAL', 'VERTICAL']),
    Enum('Align', 'tokotachi::Align', ['START', 'CENTER', 'END', 'FILL']),
    Enum('Anchor', 'tokotachi::Anchor',
         ['TOP_LEFT', 'TOP', 'TOP_RIGHT', 'LEFT', 'CENTER', 'RIGHT', 'BOTTOM_LEFT', 'BOTTOM', 'BOTTOM_RIGHT']),
    Enum('ColorScheme', 'tokotachi::ColorScheme', ['SYSTEM', 'LIGHT', 'DARK']),
    Enum('ColorRole', 'tokotachi::ColorRole',
         ['WINDOW', 'POPUP', 'TEXT', 'TEXT_SECONDARY', 'TEXT_DISABLED', 'PLACEHOLDER', 'ACCENT', 'ACCENT_HOVER',
          'ACCENT_PRESSED', 'ACCENT_DISABLED', 'ON_ACCENT', 'FOCUS_RING', 'FIELD', 'FIELD_DISABLED', 'BORDER',
          'SELECTION', 'SELECTION_INACTIVE', 'ON_SELECTION', 'PREEDIT_UNDERLINE', 'SCROLL_TRACK', 'SCROLL_THUMB',
          'SCROLL_THUMB_ACTIVE', 'TOOLTIP', 'ON_TOOLTIP']),
    Enum('ButtonStyle', 'tokotachi::ButtonStyle', ['PRIMARY', 'SECONDARY']),
    Enum('DialogButton', 'tokotachi::DialogButton', ['NONE', 'OK', 'YES', 'SAVE', 'NO', 'DISCARD', 'CANCEL', 'CLOSE']),
    Enum('FileDialogMode', 'tokotachi::FileDialogMode', ['OPEN', 'SAVE', 'FOLDER']),
    Enum('MessageType', 'tokotachi::MessageType', ['INFORMATION', 'QUESTION', 'WARNING', 'CRITICAL']),
    Enum('PopupKind', 'tokotachi::PopupKind', ['MENU', 'TOOLTIP']),
    Enum('PopupPlacement', 'tokotachi::PopupPlacement', ['BELOW', 'RIGHT', 'AT']),
    Enum('ToolButtonStyle', 'tokotachi::ToolButtonStyle', ['ICON_ONLY', 'TEXT_BESIDE_ICON', 'TEXT_UNDER_ICON']),
    Enum('SelectionMode', 'tokotachi::SelectionMode', ['NONE', 'SINGLE', 'MULTI']),
    Enum('SortOrder', 'tokotachi::SortOrder', ['ASCENDING', 'DESCENDING']),
    Enum('LineCap', 'tokotachi::LineCap', ['BUTT', 'ROUND', 'SQUARE']),
    Enum('LineJoin', 'tokotachi::LineJoin', ['MITER', 'ROUND', 'BEVEL']),
    Enum('PaintType', 'tokotachi::Paint::Type', ['SOLID', 'LINEAR', 'RADIAL'], doc='塗りの種類(C++ の Paint::Type)'),
    Enum('VariableType', None, ['BOOL', 'INT', 'FLOAT', 'DOUBLE', 'STRING'],
         doc='Variable の値の型(bool・int・float・double・std::string)'),
]

# ---- C API の全体(クラスに属さない関数。src/manual.cpp) ----
CORE = [
    F('tokotachi_version', 'raw:const char*', [], doc='実行時に読み込まれたライブラリの版("0.1.0")。解放しない'),
    F('tokotachi_free', 'void', [('raw:void*', 'p')], doc='この API が返した文字列(char*)を解放する(NULL なら何もしない)'),
    F('tokotachi_strv_free', 'void', [('raw:char**', 'strv')],
      doc='この API が返した文字列の並び(NULL で終わる char**)を、中の文字列ごと解放する'),
    F('tokotachi_last_error', 'raw:const char*', [],
      doc='このスレッドで最後に起きた失敗の説明(無ければ NULL)。次の失敗か tokotachi_clear_error まで有効'),
    F('tokotachi_clear_error', 'void', []),
    F('tokotachi_set_error_handler', 'void', [('raw:tokotachi_error_fn', 'handler'), ('raw:void*', 'user_data')],
      doc='失敗したときに、失敗したスレッドで handler を呼ぶ(NULL で外す)。バインディングが例外にするために使う'),
    F('tokotachi_struct_size', 'size_t', [('raw:const char*', 'name')],
      doc='構造体の大きさ(name は "tokotachi_rect" など。知らない名前なら 0)。バインディングの確かめに使う'),
    F('tokotachi_object_delete', 'void', [('obj:Object', 'object')],
      doc='Object を破棄する(所有しているものも一緒に破棄される)。NULL なら何もしない'),
    F('tokotachi_object_delete_later', 'void', [('obj:Object', 'object')],
      doc='今のイベントの処理が終わった後に破棄する(自分のハンドラの中から自分を破棄するとき)'),
    F('tokotachi_object_class_name', 'raw:const char*', [('obj:Object', 'object')],
      doc='この API が知っている最も派生したクラスの名前("Label" など。解放しない)'),
    F('tokotachi_object_set_user_data', 'void',
      [('obj:Object', 'object'), ('raw:void*', 'data'), ('raw:tokotachi_destroy_fn', 'destroy')],
      doc='Object に利用者のデータを 1 つ付ける。Object が破棄されるとき・付け替えたときに destroy(data) を呼ぶ\n'
          '(破棄の始め、所有しているものより先に呼ぶ。destroy の中で Object に触ってはいけない)'),
    F('tokotachi_object_get_user_data', 'raw:void*', [('obj:Object', 'object')]),
    F('tokotachi_bind', 'bool',
      [('obj:Object', 'target'), ('raw:const char*', 'target_property'), ('obj:Object', 'source'),
       ('raw:const char*', 'source_property')],
      doc='target のプロパティと source のプロパティを結ぶ(C++ の tokotachi::bind。docs/adr/0016)。名前で指す(\"text\" など)。\n'
          '型が違う・名前が無い・target が読み取り専用なら false'),
    F('tokotachi_unbind', 'bool',
      [('obj:Object', 'a'), ('raw:const char*', 'a_property'), ('obj:Object', 'b'), ('raw:const char*', 'b_property')]),
    F('tokotachi_unbind_all', 'bool', [('obj:Object', 'object'), ('raw:const char*', 'property')]),
    F('tokotachi_is_main_thread', 'bool', []),
    F('tokotachi_post', 'bool',
      [('raw:tokotachi_callback_fn', 'fn'), ('raw:void*', 'user_data'), ('raw:tokotachi_destroy_fn', 'destroy')],
      doc='どのスレッドからでも呼べる: fn(user_data) をメインスレッドで呼ぶよう頼む(docs/adr/0029)。\n'
          '呼んだ後・呼ばずに捨てたときに destroy(user_data) を呼ぶ。Application が無ければ頼まずに false'),
    F('tokotachi_clipboard_text', 'string', []),
    F('tokotachi_clipboard_set_text', 'void', [('string', 'text')]),
    F('tokotachi_clipboard_has_text', 'bool', []),
    F('tokotachi_shortcut_parse', 'Shortcut', [('string', 'text')], doc='"Ctrl+S" などから作る'),
    F('tokotachi_shortcut_to_string', 'string', [('Shortcut', 'shortcut')]),
    F('tokotachi_theme_light', 'void', [('raw:tokotachi_theme*', 'out')]),
    F('tokotachi_theme_dark', 'void', [('raw:tokotachi_theme*', 'out')]),
    F('tokotachi_theme_color_resolve', 'Color', [('ThemeColor', 'color')], doc='今のアプリケーションのテーマで具体的な色にする'),
    # ---- Image(値。参照を数えて共有する) ----
    F('tokotachi_image_load', 'Image', [('string', 'path')], doc='PNG・JPEG・BMP・GIF を読む。読めなければ NULL'),
    F('tokotachi_image_load_from_data', 'Image', [('raw:const void*', 'data'), ('size_t', 'size')]),
    F('tokotachi_image_from_pixels', 'Image',
      [('int', 'width'), ('int', 'height'), ('raw:const tokotachi_color*', 'pixels'), ('size_t', 'count')],
      doc='画素(左上から行ごと。アルファは乗算していない)から作る。count が width × height でなければ NULL'),
    F('tokotachi_image_copy', 'Image', [('raw:const tokotachi_image*', 'image')], doc='同じ画像を指す、別のハンドル'),
    F('tokotachi_image_free', 'void', [('raw:tokotachi_image*', 'image')]),
    F('tokotachi_image_width', 'int', [('raw:const tokotachi_image*', 'image')]),
    F('tokotachi_image_height', 'int', [('raw:const tokotachi_image*', 'image')]),
    F('tokotachi_image_pixel', 'Color', [('raw:const tokotachi_image*', 'image'), ('int', 'x'), ('int', 'y')]),
    F('tokotachi_image_equal', 'bool', [('raw:const tokotachi_image*', 'a'), ('raw:const tokotachi_image*', 'b')],
      doc='同じ画像か(C++ の Image の ==。中身ではなく、同じ読み込みから来たか)'),
    # ---- Bitmap(書き換えられる画像) ----
    F('tokotachi_bitmap_new', 'Bitmap', [('int', 'width'), ('int', 'height'), ('float', 'scale')]),
    F('tokotachi_bitmap_from_image', 'Bitmap', [('raw:const tokotachi_image*', 'image'), ('float', 'scale')]),
    F('tokotachi_bitmap_free', 'void', [('raw:tokotachi_bitmap*', 'bitmap')]),
    F('tokotachi_bitmap_width', 'int', [('raw:const tokotachi_bitmap*', 'bitmap')]),
    F('tokotachi_bitmap_height', 'int', [('raw:const tokotachi_bitmap*', 'bitmap')]),
    F('tokotachi_bitmap_scale', 'float', [('raw:const tokotachi_bitmap*', 'bitmap')]),
    F('tokotachi_bitmap_canvas', 'raw:tokotachi_canvas*', [('raw:tokotachi_bitmap*', 'bitmap')],
      doc='この画像に描く Canvas(Bitmap が生きている間有効。解放しない)'),
    F('tokotachi_bitmap_clear', 'void', [('raw:tokotachi_bitmap*', 'bitmap'), ('Color', 'color')]),
    F('tokotachi_bitmap_pixel', 'Color', [('raw:const tokotachi_bitmap*', 'bitmap'), ('int', 'x'), ('int', 'y')]),
    F('tokotachi_bitmap_set_pixel', 'void',
      [('raw:tokotachi_bitmap*', 'bitmap'), ('int', 'x'), ('int', 'y'), ('Color', 'color')]),
    F('tokotachi_bitmap_pixels', 'raw:uint32_t*', [('raw:tokotachi_bitmap*', 'bitmap')],
      doc='画素の並び(行ごとに width 個。アルファ乗算済みの 0xAARRGGBB)'),
    F('tokotachi_bitmap_to_image', 'Image', [('raw:const tokotachi_bitmap*', 'bitmap')]),
    F('tokotachi_bitmap_save_png', 'bool', [('raw:const tokotachi_bitmap*', 'bitmap'), ('string', 'path')],
      doc='PNG に保存する'),
    # ---- Path ----
    F('tokotachi_path_new', 'raw:tokotachi_path*', []),
    F('tokotachi_path_free', 'void', [('raw:tokotachi_path*', 'path')]),
    F('tokotachi_path_move_to', 'void', [('raw:tokotachi_path*', 'path'), ('float', 'x'), ('float', 'y')]),
    F('tokotachi_path_line_to', 'void', [('raw:tokotachi_path*', 'path'), ('float', 'x'), ('float', 'y')]),
    F('tokotachi_path_quad_to', 'void',
      [('raw:tokotachi_path*', 'path'), ('float', 'x1'), ('float', 'y1'), ('float', 'x2'), ('float', 'y2')]),
    F('tokotachi_path_cubic_to', 'void',
      [('raw:tokotachi_path*', 'path'), ('float', 'x1'), ('float', 'y1'), ('float', 'x2'), ('float', 'y2'),
       ('float', 'x3'), ('float', 'y3')]),
    F('tokotachi_path_close', 'void', [('raw:tokotachi_path*', 'path')]),
    F('tokotachi_path_add_rect', 'void', [('raw:tokotachi_path*', 'path'), ('Rect', 'rect')]),
    F('tokotachi_path_add_round_rect', 'void',
      [('raw:tokotachi_path*', 'path'), ('Rect', 'rect'), ('float', 'radius')]),
    F('tokotachi_path_add_ellipse', 'void',
      [('raw:tokotachi_path*', 'path'), ('Point', 'center'), ('float', 'rx'), ('float', 'ry')]),
    F('tokotachi_path_add_arc', 'void',
      [('raw:tokotachi_path*', 'path'), ('Point', 'center'), ('float', 'rx'), ('float', 'ry'),
       ('float', 'start_angle'), ('float', 'sweep_angle'), ('bool', 'new_figure')]),
    F('tokotachi_path_add_polyline', 'void',
      [('raw:tokotachi_path*', 'path'), ('raw:const tokotachi_point*', 'points'), ('size_t', 'count'),
       ('bool', 'close')]),
    F('tokotachi_path_is_empty', 'bool', [('raw:const tokotachi_path*', 'path')]),
    # ---- TextLayout ----
    F('tokotachi_text_layout_new', 'raw:tokotachi_text_layout*',
      [('string', 'text'), ('Font', 'font'), ('float', 'scale'), ('float', 'wrap_width'), ('Align', 'alignment')]),
    F('tokotachi_text_layout_free', 'void', [('raw:tokotachi_text_layout*', 'layout')]),
    F('tokotachi_text_layout_width', 'float', [('raw:const tokotachi_text_layout*', 'layout')]),
    F('tokotachi_text_layout_height', 'float', [('raw:const tokotachi_text_layout*', 'layout')]),
    F('tokotachi_text_layout_ascent', 'float', [('raw:const tokotachi_text_layout*', 'layout')]),
    F('tokotachi_text_layout_descent', 'float', [('raw:const tokotachi_text_layout*', 'layout')]),
    F('tokotachi_text_layout_line_height', 'float', [('raw:const tokotachi_text_layout*', 'layout')]),
    F('tokotachi_text_layout_line_count', 'size_t', [('raw:const tokotachi_text_layout*', 'layout')]),
    F('tokotachi_text_layout_line_rect', 'Rect', [('raw:const tokotachi_text_layout*', 'layout'), ('size_t', 'line')]),
    F('tokotachi_text_layout_caret_position', 'Point',
      [('raw:const tokotachi_text_layout*', 'layout'), ('size_t', 'offset')]),
    F('tokotachi_text_layout_offset_at', 'size_t', [('raw:const tokotachi_text_layout*', 'layout'), ('Point', 'p')]),
    # ---- Canvas(描くハンドラ・Bitmap から借りる。解放しない) ----
    F('tokotachi_canvas_scale', 'float', [('raw:tokotachi_canvas*', 'canvas')]),
    F('tokotachi_canvas_save', 'void', [('raw:tokotachi_canvas*', 'canvas')]),
    F('tokotachi_canvas_restore', 'void', [('raw:tokotachi_canvas*', 'canvas')]),
    F('tokotachi_canvas_translate', 'void', [('raw:tokotachi_canvas*', 'canvas'), ('float', 'dx'), ('float', 'dy')]),
    F('tokotachi_canvas_scale_by', 'void', [('raw:tokotachi_canvas*', 'canvas'), ('float', 'sx'), ('float', 'sy')]),
    F('tokotachi_canvas_rotate', 'void', [('raw:tokotachi_canvas*', 'canvas'), ('float', 'radians')]),
    F('tokotachi_canvas_clip_rect', 'void', [('raw:tokotachi_canvas*', 'canvas'), ('Rect', 'rect')]),
    F('tokotachi_canvas_clip_path', 'void', [('raw:tokotachi_canvas*', 'canvas'), ('raw:const tokotachi_path*', 'path')]),
    F('tokotachi_canvas_clip_bounds', 'Rect', [('raw:tokotachi_canvas*', 'canvas')]),
    F('tokotachi_canvas_begin_layer', 'void', [('raw:tokotachi_canvas*', 'canvas'), ('float', 'opacity')]),
    F('tokotachi_canvas_end_layer', 'void', [('raw:tokotachi_canvas*', 'canvas')]),
    F('tokotachi_canvas_fill_rect', 'void', [('raw:tokotachi_canvas*', 'canvas'), ('Rect', 'rect'), ('Color', 'color')]),
    F('tokotachi_canvas_fill_round_rect', 'void',
      [('raw:tokotachi_canvas*', 'canvas'), ('Rect', 'rect'), ('float', 'radius'),
       ('raw:const tokotachi_paint*', 'paint')]),
    F('tokotachi_canvas_stroke_round_rect', 'void',
      [('raw:tokotachi_canvas*', 'canvas'), ('Rect', 'rect'), ('float', 'radius'), ('float', 'width'),
       ('Color', 'color')]),
    F('tokotachi_canvas_stroke_rect', 'void',
      [('raw:tokotachi_canvas*', 'canvas'), ('Rect', 'rect'), ('float', 'width'), ('Color', 'color')]),
    F('tokotachi_canvas_fill_path', 'void',
      [('raw:tokotachi_canvas*', 'canvas'), ('raw:const tokotachi_path*', 'path'),
       ('raw:const tokotachi_paint*', 'paint')]),
    F('tokotachi_canvas_stroke_path', 'void',
      [('raw:tokotachi_canvas*', 'canvas'), ('raw:const tokotachi_path*', 'path'),
       ('raw:const tokotachi_stroke_style*', 'style'), ('raw:const tokotachi_paint*', 'paint')]),
    F('tokotachi_canvas_draw_line', 'void',
      [('raw:tokotachi_canvas*', 'canvas'), ('Point', 'from'), ('Point', 'to'),
       ('raw:const tokotachi_stroke_style*', 'style'), ('raw:const tokotachi_paint*', 'paint')]),
    F('tokotachi_canvas_fill_ellipse', 'void',
      [('raw:tokotachi_canvas*', 'canvas'), ('Rect', 'rect'), ('raw:const tokotachi_paint*', 'paint')]),
    F('tokotachi_canvas_stroke_ellipse', 'void',
      [('raw:tokotachi_canvas*', 'canvas'), ('Rect', 'rect'), ('raw:const tokotachi_stroke_style*', 'style'),
       ('raw:const tokotachi_paint*', 'paint')]),
    F('tokotachi_canvas_draw_text', 'void',
      [('raw:tokotachi_canvas*', 'canvas'), ('string', 'text'), ('Point', 'pos'), ('Color', 'color'),
       ('Font', 'font')], doc='1 行の文字(pos は左上)'),
    F('tokotachi_canvas_draw_text_layout', 'void',
      [('raw:tokotachi_canvas*', 'canvas'), ('raw:const tokotachi_text_layout*', 'layout'), ('Point', 'pos'),
       ('Color', 'color')]),
    F('tokotachi_canvas_draw_image', 'void',
      [('raw:tokotachi_canvas*', 'canvas'), ('raw:const tokotachi_image*', 'image'), ('Rect', 'dst'),
       ('float', 'opacity')]),
]

# ---- クラス ----
WIDGET_HANDLERS = [
    H('on_paint', 'paint', doc='自分の描画の後に呼ぶ(子の描画の前)'),
    H('on_resize', 'notify'),
    H('on_mouse_down', 'mouse'),
    H('on_mouse_up', 'mouse'),
    H('on_mouse_move', 'mouse'),
    H('on_mouse_enter', 'notify'),
    H('on_mouse_leave', 'notify'),
    H('on_mouse_wheel', 'wheel'),
    H('on_key_down', 'key'),
    H('on_key_up', 'key'),
    H('on_focus_in', 'notify'),
    H('on_focus_out', 'notify'),
]

PARENT = [('obj:Widget', 'parent', 'None')]
PARENT_TEXT = [('obj:Widget', 'parent', 'None'), ('string', 'text', "''")]

CLASSES = [
    Cls('Object', 'tokotachi::Object', None, 'core/property.hpp', [
        P('owner', 'obj:Object', ro=True, bindable=False),
    ], doc='すべてのクラスの基底'),
    Cls('Application', 'tokotachi::Application', 'Object', 'core/application.hpp', [
        P('color_scheme', 'ColorScheme'),
        P('light_theme', 'Theme'),
        P('dark_theme', 'Theme'),
        P('dark', 'bool', ro=True),
        P('theme', 'Theme', ro=True),
        P('language', 'string'),
        P('main_window', 'obj:Window', bindable=False, doc='閉じたら(隠れたら・破棄されたら)アプリケーションを終えるウィンドウ'),
        P('auto_quit_enabled', 'bool'),
        H('on_theme_change', 'notify'),
        M('exec', 'int', doc='イベントループを回し、quit で抜ける。戻り値は quit に渡した値'),
        M('quit', 'void', [('int', 'code', '0')]),
        F('tokotachi_application_instance', 'obj:Application', []),
    ], new=[('raw:int', 'argc', None), ('raw:char**', 'argv', None)],
        doc='1 つのプロセスに 1 つだけ作る', py_mixin='_ApplicationMixin', py_init=False),
    Cls('Timer', 'tokotachi::Timer', 'Object', 'core/timer.hpp', [
        P('interval', 'int'),
        P('repeat', 'bool'),
        P('enabled', 'bool'),
        H('on_timeout', 'notify'),
    ], new=[('obj:Object', 'owner', 'None')]),
    Cls('Widget', 'tokotachi::Widget', 'Object', 'widgets/widget.hpp', [
        P('parent', 'obj:Widget', bindable=False),
        P('children', 'objs:Widget', ro=True),
        P('window', 'obj:Window', ro=True, bindable=False),
        P('x', 'float'),
        P('y', 'float'),
        P('width', 'float'),
        P('height', 'float'),
        P('bounds', 'Rect'),
        P('local_rect', 'Rect', ro=True),
        P('preferred_size', 'Size', ro=True),
        P('minimum_size', 'Size'),
        P('maximum_size', 'Size'),
        P('minimum_hint', 'Size', ro=True),
        P('layout', 'obj:Layout', ro=True, bindable=False),
        P('visible', 'bool'),
        P('showing', 'bool', ro=True),
        P('enabled', 'bool'),
        P('effectively_enabled', 'bool', ro=True),
        P('focus_policy', 'FocusPolicy'),
        P('focused', 'bool'),
        P('cursor', 'MouseCursor'),
        P('accepts_text_input', 'bool'),
        P('display_scale', 'float', ro=True),
        P('tooltip', 'string'),
        P('actions', 'objs:Action', ro=True),
        M('add_action', 'void', [('obj:Action', 'action')]),
        M('remove_action', 'void', [('obj:Action', 'action')]),
        M('invalidate', 'void'),
        M('invalidate_rect', 'void', [('Rect', 'rect')], cpp='{self}->invalidate({rect})'),
        M('request_layout', 'void'),
        M('apply_layout', 'void'),
        M('map_to_window', 'Point', [('Point', 'p')]),
        M('map_from_window', 'Point', [('Point', 'p')]),
        M('child_at', 'obj:Widget', [('Point', 'p')]),
    ] + WIDGET_HANDLERS, new=PARENT, doc='ウィジェットの基底。tokotachi_widget_new は中身の無い入れ物を作る'),
    # ---- widgets ----
    Cls('ButtonBase', 'tokotachi::ButtonBase', 'Widget', 'widgets/button_base.hpp', [
        P('text', 'string'),
        P('font', 'Font'),
        H('on_click', 'notify'),
    ]),
    Cls('Button', 'tokotachi::Button', 'ButtonBase', 'widgets/button.hpp', [
        P('style', 'ButtonStyle'),
    ], new=PARENT_TEXT),
    Cls('CheckBox', 'tokotachi::CheckBox', 'ButtonBase', 'widgets/check_box.hpp', [
        P('checked', 'bool'),
        H('on_change', 'notify'),
    ], new=PARENT_TEXT),
    Cls('RadioButton', 'tokotachi::RadioButton', 'ButtonBase', 'widgets/radio_button.hpp', [
        P('value', 'int'),
        P('group_value', 'int'),
        P('checked', 'bool', ro=True),
        H('on_change', 'notify'),
    ], new=PARENT_TEXT + [('int', 'value', '0')]),
    Cls('ComboBox', 'tokotachi::ComboBox', 'Widget', 'widgets/combo_box.hpp', [
        P('items', 'strings'),
        P('current_index', 'int'),
        P('current_text', 'string', ro=True),
        P('placeholder_text', 'string'),
        P('font', 'Font'),
        H('on_change', 'notify'),
        M('open_popup', 'void'),
        M('close_popup', 'void'),
        P('popup_open', 'bool', ro=True),
    ], new=PARENT),
    Cls('Label', 'tokotachi::Label', 'Widget', 'widgets/label.hpp', [
        P('text', 'string'),
        P('font', 'Font'),
        P('text_color', 'ThemeColor'),
        P('alignment', 'Align'),
        P('word_wrap', 'bool'),
    ], new=PARENT_TEXT),
    Cls('TextInputBase', 'tokotachi::TextInputBase', 'Widget', 'widgets/text_input_base.hpp', [
        P('text', 'string'),
        P('placeholder_text', 'string'),
        P('font', 'Font'),
        P('cursor_position', 'size_t'),
        P('has_selection', 'bool', ro=True),
        P('selected_text', 'string', ro=True),
        H('on_change', 'notify'),
        M('select_all', 'void'),
        M('clear_selection', 'void'),
        M('cut', 'void'),
        M('copy', 'void'),
        M('paste', 'void'),
        M('insert', 'void', [('string', 's')]),
    ]),
    Cls('LineEdit', 'tokotachi::LineEdit', 'TextInputBase', 'widgets/line_edit.hpp', [
        H('on_submit', 'notify'),
    ], new=PARENT_TEXT),
    Cls('TextEdit', 'tokotachi::TextEdit', 'TextInputBase', 'widgets/text_edit.hpp', [
        P('word_wrap', 'bool'),
    ], new=PARENT_TEXT),
    Cls('PaintBox', 'tokotachi::PaintBox', 'Widget', 'widgets/paint_box.hpp', [
        P('cached', 'bool'),
        M('refresh', 'void'),
    ], new=PARENT),
    Cls('ProgressBar', 'tokotachi::ProgressBar', 'Widget', 'widgets/progress_bar.hpp', [
        P('minimum', 'double'),
        P('maximum', 'double'),
        P('value', 'double'),
        P('text_visible', 'bool'),
    ], new=PARENT),
    Cls('RangeBase', 'tokotachi::RangeBase', 'Widget', 'widgets/range_base.hpp', [
        P('minimum', 'double'),
        P('maximum', 'double'),
        P('value', 'double'),
        P('single_step', 'double'),
        P('page_step', 'double'),
        H('on_change', 'notify'),
    ]),
    Cls('Slider', 'tokotachi::Slider', 'RangeBase', 'widgets/slider.hpp', [
        P('orientation', 'Orientation'),
    ], new=PARENT + [('Orientation', 'orientation', 'Orientation.HORIZONTAL')]),
    Cls('SpinBox', 'tokotachi::SpinBox', 'RangeBase', 'widgets/spin_box.hpp', [
        P('decimals', 'int'),
        P('text', 'string', ro=True),
        M('commit', 'void'),
    ], new=PARENT),
    Cls('ScrollArea', 'tokotachi::ScrollArea', 'Widget', 'widgets/scroll_area.hpp', [
        P('content', 'obj:Widget', bindable=False),
        P('fit_width', 'bool'),
        P('scroll_position', 'Point'),
        M('scroll_to_visible', 'void', [('Rect', 'rect')]),
    ], new=PARENT),
    Cls('ScrollBar', 'tokotachi::ScrollBar', 'Widget', 'widgets/scroll_bar.hpp', [
        P('orientation', 'Orientation'),
        P('value', 'float'),
        P('maximum', 'float'),
        P('page_step', 'float'),
        P('single_step', 'float'),
        H('on_change', 'notify'),
    ], new=PARENT + [('Orientation', 'orientation', 'Orientation.VERTICAL')]),
    Cls('SplitterPane', 'tokotachi::SplitterPane', 'Object', 'widgets/splitter.hpp', [
        P('stretch', 'int'),
        P('collapsible', 'bool'),
        P('collapsed', 'bool', ro=True),
        P('widget', 'obj:Widget', ro=True, bindable=False),
    ], doc='Splitter が作り、所有する'),
    Cls('Splitter', 'tokotachi::Splitter', 'Widget', 'widgets/splitter.hpp', [
        P('orientation', 'Orientation'),
        P('panes', 'objs:SplitterPane', ro=True),
        P('sizes', 'floats'),
        H('on_change', 'notify'),
        M('add_widget', 'obj:SplitterPane', [('obj:Widget', 'widget'), ('int', 'stretch', '1')]),
        M('insert_widget', 'obj:SplitterPane', [('int', 'index'), ('obj:Widget', 'widget'), ('int', 'stretch', '1')]),
    ], new=PARENT + [('Orientation', 'orientation', 'Orientation.HORIZONTAL')]),
    Cls('Tab', 'tokotachi::Tab', 'Object', 'widgets/tab_widget.hpp', [
        P('title', 'string'),
        P('enabled', 'bool'),
        P('page', 'obj:Widget', ro=True, bindable=False),
    ], doc='TabWidget が作り、所有する'),
    Cls('TabWidget', 'tokotachi::TabWidget', 'Widget', 'widgets/tab_widget.hpp', [
        P('tabs', 'objs:Tab', ro=True),
        P('current_index', 'int'),
        P('current_tab', 'obj:Tab', ro=True, bindable=False),
        H('on_change', 'notify'),
        M('add_tab', 'obj:Tab', [('obj:Widget', 'page'), ('string', 'title')]),
        M('insert_tab', 'obj:Tab', [('int', 'index'), ('obj:Widget', 'page'), ('string', 'title')]),
        M('remove_tab', 'obj:Widget', [('obj:Tab', 'tab')],
          doc='タブを除き、ページを返す(ページは親が無くなり、呼んだ側が破棄する)'),
    ], new=PARENT),
    # ---- windows ----
    Cls('Window', 'tokotachi::Window', 'Widget', 'windows/window.hpp', [
        P('title', 'string'),
        P('background', 'ThemeColor'),
        P('focused_widget', 'obj:Widget', ro=True, bindable=False),
        P('menu_bar', 'obj:MenuBar', bindable=False),
        P('tool_bar', 'obj:ToolBar', bindable=False),
        P('status_bar', 'obj:StatusBar', bindable=False),
        P('destroy_on_close', 'bool', doc='true なら、閉じた後に破棄する'),
        H('on_close', 'close'),
        M('close', 'void'),
        F('tokotachi_window_new_owned', 'obj:Window',
          [('obj:Window', 'owner'), ('string', 'title'), ('float', 'width'), ('float', 'height')],
          doc='owner が所有し、その上に出すウィンドウ(docs/adr/0062)'),
        M('save_png', 'bool', [('string', 'path')]),
        M('capture', 'Bitmap', doc='全体を描いて Bitmap にする(tokotachi_bitmap_free で解放する)'),
    ], new=[('string', 'title', "''"), ('float', 'width', '640'), ('float', 'height', '480')],
        doc='Application を作った後に作る。Application が所有する(docs/adr/0062)', py_mixin='_WindowMixin',
        py_init=False),
    Cls('Dialog', 'tokotachi::Dialog', 'Window', 'windows/dialog.hpp', [
        P('result', 'DialogButton', ro=True),
        H('on_finish', 'notify'),
        M('add_button', 'obj:Button', [('DialogButton', 'button'), ('string', 'text', "''")],
          doc='text が空なら、言語に合わせた標準の文字'),
        M('open', 'void'),
        M('finish', 'void', [('DialogButton', 'result')]),
    ], new=[('obj:Window', 'parent', None), ('string', 'title', "''")], doc='parent が所有する'),
    Cls('MessageDialog', 'tokotachi::MessageDialog', 'Dialog', 'windows/message_dialog.hpp', [
        P('text', 'string'),
        P('type', 'MessageType'),
        F('tokotachi_message_dialog_show', 'obj:MessageDialog',
          [('obj:Window', 'parent'), ('string', 'title'), ('string', 'text'),
           ('raw:const tokotachi_dialog_button*', 'buttons'), ('size_t', 'button_count'),
           ('raw:tokotachi_notify_fn', 'on_finish'), ('raw:void*', 'user_data'),
           ('raw:tokotachi_destroy_fn', 'destroy'), ('MessageType', 'type')],
          doc='作ってボタンを加えて開き、閉じたら on_finish を呼んで破棄する(button_count が 0 なら OK だけ)'),
    ], new=[('obj:Window', 'parent', None), ('string', 'title', "''"), ('string', 'text', "''"),
            ('MessageType', 'type', 'MessageType.INFORMATION')], py_mixin='_MessageDialogMixin'),
    Cls('FileDialog', 'tokotachi::FileDialog', 'Object', 'windows/file_dialog.hpp', [
        P('mode', 'FileDialogMode'),
        P('title', 'string'),
        P('filters', 'file_filters', doc='名前と拡張子(";" でつなぐ)を交互に並べたもの(["画像", "png;jpg"])', bindable=False),
        P('location', 'string'),
        P('multi_enabled', 'bool'),
        P('paths', 'strings', ro=True),
        P('error', 'string', ro=True),
        P('selected_filter', 'int', ro=True),
        P('running', 'bool', ro=True),
        H('on_finish', 'notify'),
        M('open', 'void'),
    ], new=[('obj:Window', 'parent', 'None'), ('FileDialogMode', 'mode', 'FileDialogMode.OPEN')]),
    Cls('MenuBar', 'tokotachi::MenuBar', 'Widget', 'windows/menu_bar.hpp', [
        P('menus', 'objs:Menu', ro=True),
        M('add_menu', 'obj:Menu', [('string', 'title')]),
        M('remove_menu', 'void', [('obj:Menu', 'menu')]),
    ], new=PARENT),
    Cls('Popup', 'tokotachi::Popup', 'Window', 'windows/popup.hpp', [
        P('owner_widget', 'obj:Widget', ro=True, bindable=False),
        P('kind', 'PopupKind', ro=True),
        H('on_closed', 'notify'),
        M('open', 'void', [('Rect', 'anchor'), ('Size', 'size'), ('PopupPlacement', 'placement', 'PopupPlacement.BELOW')]),
    ], new=[('obj:Widget', 'owner_widget', None), ('PopupKind', 'kind', 'PopupKind.MENU'),
            ('obj:Object', 'owner_object', 'None')]),
    Cls('StatusBar', 'tokotachi::StatusBar', 'Widget', 'windows/status_bar.hpp', [
        P('text', 'string'),
        P('message', 'string', ro=True),
        M('show_message', 'void', [('string', 'message'), ('int', 'timeout_ms', '0')]),
        M('clear_message', 'void'),
        M('add_widget', 'void', [('obj:Widget', 'widget')]),
    ], new=PARENT),
    Cls('ToolBar', 'tokotachi::ToolBar', 'Widget', 'windows/tool_bar.hpp', [
        P('button_style', 'ToolButtonStyle'),
        M('add_item', 'obj:Action', [('string', 'text')], doc='Action を作ってボタンとして加える(ツールバーが所有する)'),
        M('add_separator', 'void'),
        M('add_widget', 'void', [('obj:Widget', 'widget')]),
    ], new=PARENT),
    # ---- layout ----
    Cls('Layout', 'tokotachi::Layout', 'Object', 'layout/layout.hpp', [
        P('widget', 'obj:Widget', ro=True, bindable=False),
        P('margins', 'Margins'),
        M('remove', 'void', [('obj:Widget', 'widget')]),
    ], doc='target が所有する(target が前のレイアウトを持っていれば、それは破棄される)'),
    Cls('BoxLayout', 'tokotachi::BoxLayout', 'Layout', 'layout/box_layout.hpp', [
        P('orientation', 'Orientation'),
        P('spacing', 'float'),
        M('add', 'void', [('obj:Widget', 'widget'), ('int', 'stretch', '0'), ('Align', 'cross_alignment', 'Align.FILL')]),
        M('add_spacing', 'void', [('float', 'size')]),
        M('add_stretch', 'void', [('int', 'stretch', '1')]),
        M('set_stretch', 'void', [('obj:Widget', 'widget'), ('int', 'stretch')]),
        M('set_alignment', 'void', [('obj:Widget', 'widget'), ('Align', 'cross_alignment')]),
    ], new=[('obj:Widget', 'target', None), ('Orientation', 'orientation', 'Orientation.VERTICAL')]),
    Cls('GridLayout', 'tokotachi::GridLayout', 'Layout', 'layout/grid_layout.hpp', [
        P('row_spacing', 'float'),
        P('column_spacing', 'float'),
        M('add', 'void', [('obj:Widget', 'widget'), ('int', 'row'), ('int', 'column'), ('int', 'row_span', '1'),
                          ('int', 'column_span', '1'), ('Align', 'horizontal', 'Align.FILL'),
                          ('Align', 'vertical', 'Align.CENTER')]),
        M('set_row_stretch', 'void', [('int', 'row'), ('int', 'stretch')]),
        M('set_column_stretch', 'void', [('int', 'column'), ('int', 'stretch')]),
        M('set_row_minimum', 'void', [('int', 'row'), ('float', 'size')]),
        M('set_column_minimum', 'void', [('int', 'column'), ('float', 'size')]),
    ], new=[('obj:Widget', 'target', None)]),
    Cls('PlaceLayout', 'tokotachi::PlaceLayout', 'Layout', 'layout/place_layout.hpp', [
        M('add', 'void', [('obj:Widget', 'widget'), ('Placement', 'placement')]),
    ], new=[('obj:Widget', 'target', None)]),
    # ---- actions ----
    Cls('Action', 'tokotachi::Action', 'Object', 'actions/action.hpp', [
        P('text', 'string'),
        P('shortcut', 'Shortcut'),
        P('icon', 'Image', bindable=False),
        P('tooltip', 'string'),
        P('enabled', 'bool'),
        P('checkable', 'bool'),
        P('checked', 'bool'),
        H('on_trigger', 'notify'),
        M('trigger', 'void'),
    ], new=[('obj:Object', 'owner', 'None'), ('string', 'text', "''")]),
    Cls('Menu', 'tokotachi::Menu', 'Object', 'actions/menu.hpp', [
        P('title', 'string'),
        P('actions', 'objs:Action', ro=True),
        M('add_item', 'obj:Action', [('string', 'text')], doc='Action を作って最後に加える(メニューが所有する)'),
        M('add_action', 'void', [('obj:Action', 'action')]),
        M('remove_action', 'void', [('obj:Action', 'action')]),
        M('add_separator', 'void'),
        M('add_submenu', 'obj:Menu', [('string', 'title')]),
        M('popup', 'void', [('obj:Widget', 'anchor_widget'), ('Point', 'at')]),
        M('close', 'void'),
        P('opened', 'bool', ro=True),
    ], new=[('obj:Object', 'owner', 'None'), ('string', 'title', "''")]),
    # ---- views ----
    Cls('ItemModel', 'tokotachi::ItemModel', 'Object', 'views/item_model.hpp', [
        M('row_count', 'int', [('ModelIndex', 'parent', 'None')]),
        M('index', 'ModelIndex', [('int', 'row'), ('ModelIndex', 'parent', 'None')]),
        M('parent_of', 'ModelIndex', [('ModelIndex', 'index')]),
        M('text', 'string', [('ModelIndex', 'index'), ('int', 'column', '0')]),
        M('icon', 'Image', [('ModelIndex', 'index')]),
        M('enabled', 'bool', [('ModelIndex', 'index')]),
        M('checkable', 'bool', [('ModelIndex', 'index')]),
        M('checked', 'bool', [('ModelIndex', 'index')]),
        M('set_checked', 'void', [('ModelIndex', 'index'), ('bool', 'checked')]),
        M('sort', 'void', [('int', 'column'), ('SortOrder', 'order')]),
        M('locate', 'ModelIndex', [('ModelIndex', 'index')]),
    ], doc='一覧・木のモデルの基底。独自のモデルは tokotachi_custom_model_new で作る'),
    Cls('Item', 'tokotachi::Item', 'Object', 'views/item.hpp', [
        P('text', 'string'),
        P('column_texts', 'strings'),
        P('icon', 'Image', bindable=False),
        P('enabled', 'bool'),
        P('checkable', 'bool'),
        P('checked', 'bool'),
        M('column_text', 'string', [('int', 'column')]),
    ], doc='ListModel・TreeModel の項目(モデルが所有する)'),
    Cls('ListModel', 'tokotachi::ListModel', 'ItemModel', 'views/list_model.hpp', [
        P('items', 'objs:Item', ro=True),
        M('add_item', 'obj:Item', [('string', 'text')]),
        M('insert_item', 'obj:Item', [('int', 'index'), ('string', 'text')]),
        M('remove_item', 'void', [('obj:Item', 'item')]),
        M('clear', 'void'),
        M('index_of', 'ModelIndex', [('obj:Item', 'item')]),
        M('item_at', 'obj:Item', [('ModelIndex', 'index')]),
    ], new=[('obj:Object', 'owner', 'None')]),
    Cls('TreeItem', 'tokotachi::TreeItem', 'Item', 'views/tree_model.hpp', [
        P('parent_item', 'obj:TreeItem', ro=True, bindable=False),
        P('children', 'objs:TreeItem', ro=True),
        M('add_child', 'obj:TreeItem', [('string', 'text')]),
        M('insert_child', 'obj:TreeItem', [('int', 'index'), ('string', 'text')]),
    ]),
    Cls('TreeModel', 'tokotachi::TreeModel', 'ItemModel', 'views/tree_model.hpp', [
        P('items', 'objs:TreeItem', ro=True),
        M('add_item', 'obj:TreeItem', [('string', 'text')]),
        M('insert_item', 'obj:TreeItem', [('int', 'index'), ('string', 'text')]),
        M('remove_item', 'void', [('obj:TreeItem', 'item')]),
        M('clear', 'void'),
        M('index_of', 'ModelIndex', [('obj:TreeItem', 'item')]),
        M('item_at', 'obj:TreeItem', [('ModelIndex', 'index')]),
    ], new=[('obj:Object', 'owner', 'None')]),
    Cls('Column', 'tokotachi::Column', 'Object', 'views/item_view.hpp', [
        P('title', 'string'),
        P('width', 'float'),
        P('alignment', 'Align'),
    ], doc='ItemView が作り、所有する'),
    Cls('ItemView', 'tokotachi::ItemView', 'Widget', 'views/item_view.hpp', [
        P('model', 'obj:ItemModel', bindable=False),
        P('columns', 'objs:Column', ro=True),
        P('header_visible', 'bool'),
        P('sorting_enabled', 'bool'),
        P('sort_column', 'int'),
        P('sort_order', 'SortOrder'),
        P('selection_mode', 'SelectionMode'),
        P('current', 'ModelIndex'),
        P('selection', 'ModelIndexes', ro=True),
        H('on_selection_change', 'notify'),
        H('on_activate', 'notify'),
        M('add_column', 'obj:Column', [('string', 'title'), ('float', 'width', '120')]),
        M('remove_column', 'void', [('obj:Column', 'column')]),
        M('is_selected', 'bool', [('ModelIndex', 'index')]),
        M('select', 'void', [('ModelIndex', 'index'), ('bool', 'selected', 'True')]),
        M('select_all', 'void'),
        M('clear_selection', 'void'),
        M('scroll_to', 'void', [('ModelIndex', 'index')]),
        M('index_at', 'ModelIndex', [('Point', 'p')]),
    ], doc='ListView・TreeView の基底(モデルは所有しない)'),
    Cls('ListView', 'tokotachi::ListView', 'ItemView', 'views/list_view.hpp', [], new=PARENT),
    Cls('TreeView', 'tokotachi::TreeView', 'ItemView', 'views/tree_view.hpp', [
        P('indentation', 'float'),
        M('expand', 'void', [('ModelIndex', 'index')]),
        M('collapse', 'void', [('ModelIndex', 'index')]),
        M('is_expanded', 'bool', [('ModelIndex', 'index')]),
        M('expand_all', 'void'),
        M('collapse_all', 'void'),
    ], new=PARENT),
    # ---- C API だけのクラス(src/manual.cpp) ----
    Cls('Variable', 'tokotachi::Object', 'Object', 'core/variable.hpp', [
        F('tokotachi_variable_new', 'obj:Variable', [('obj:Object', 'owner'), ('VariableType', 'type')],
          doc='値の型が type の Variable(C++ の Variable<T>)を作る。値は T()'),
        F('tokotachi_variable_get_type', 'VariableType', [('obj:Variable', 'variable')]),
        F('tokotachi_variable_get_bool', 'bool', [('obj:Variable', 'variable')]),
        F('tokotachi_variable_set_bool', 'void', [('obj:Variable', 'variable'), ('bool', 'value')]),
        F('tokotachi_variable_get_int', 'int', [('obj:Variable', 'variable')]),
        F('tokotachi_variable_set_int', 'void', [('obj:Variable', 'variable'), ('int', 'value')]),
        F('tokotachi_variable_get_float', 'float', [('obj:Variable', 'variable')]),
        F('tokotachi_variable_set_float', 'void', [('obj:Variable', 'variable'), ('float', 'value')]),
        F('tokotachi_variable_get_double', 'double', [('obj:Variable', 'variable')]),
        F('tokotachi_variable_set_double', 'void', [('obj:Variable', 'variable'), ('double', 'value')]),
        F('tokotachi_variable_get_string', 'string', [('obj:Variable', 'variable')]),
        F('tokotachi_variable_set_string', 'void', [('obj:Variable', 'variable'), ('string', 'value')]),
        F('tokotachi_variable_set_on_set', 'void',
          [('obj:Variable', 'variable'), ('raw:tokotachi_notify_fn', 'fn'), ('raw:void*', 'user_data'),
           ('raw:tokotachi_destroy_fn', 'destroy')]),
        F('tokotachi_variable_set_on_change', 'void',
          [('obj:Variable', 'variable'), ('raw:tokotachi_notify_fn', 'fn'), ('raw:void*', 'user_data'),
           ('raw:tokotachi_destroy_fn', 'destroy')]),
    ], doc='値を 1 つ持つオブジェクト(docs/adr/0016)。プロパティ "value" を tokotachi_bind で結ぶ。\n'
           '型と合わない get・set は失敗する', py_mixin='_VariableMixin', py_init=False),
    Cls('Task', 'tokotachi::TaskBase', 'Object', 'core/task.hpp', [
        P('running', 'bool', ro=True),
        P('progress', 'double', ro=True),
        P('cancelled', 'bool', ro=True),
        P('error', 'string', ro=True),
        H('on_progress', 'notify'),
        H('on_finish', 'notify'),
        M('cancel', 'void'),
        F('tokotachi_task_new', 'obj:Task', [('obj:Object', 'owner')], doc='結果を持たない Task(C++ の Task<void>)'),
        F('tokotachi_task_start', 'bool',
          [('obj:Task', 'task'), ('raw:tokotachi_task_fn', 'work'), ('raw:void*', 'user_data'),
           ('raw:tokotachi_destroy_fn', 'destroy')],
          doc='work(context, user_data) を別のスレッドで呼ぶ。動いていれば何もせず false。\n'
              'destroy(user_data) は仕事が終わった後に呼ぶ(どのスレッドかは決まっていない)'),
        F('tokotachi_task_context_cancel_requested', 'bool', [('raw:tokotachi_task_context*', 'context')]),
        F('tokotachi_task_context_report_progress', 'void', [('raw:tokotachi_task_context*', 'context'),
                                                              ('double', 'value')]),
        F('tokotachi_task_context_fail', 'void', [('raw:tokotachi_task_context*', 'context'), ('string', 'message')],
          doc='仕事を失敗として終える(work が戻った後、error に message が入る)'),
    ], doc='別のスレッドで行う、終わりのある仕事(docs/adr/0029)', py_mixin='_TaskMixin', py_init=False),
    Cls('CustomModel', 'tokotachi::ItemModel', 'ItemModel', 'views/item_model.hpp', [
        F('tokotachi_custom_model_new', 'obj:CustomModel',
          [('obj:Object', 'owner'), ('raw:const tokotachi_model_callbacks*', 'callbacks'), ('raw:void*', 'user_data'),
           ('raw:tokotachi_destroy_fn', 'destroy')],
          doc='関数で中身を答えるモデル。callbacks は写す。モデルが破棄されるときに destroy(user_data) を呼ぶ'),
        F('tokotachi_custom_model_notify_rows_inserted', 'void',
          [('obj:CustomModel', 'model'), ('ModelIndex', 'parent'), ('int', 'first'), ('int', 'count')],
          doc='加えた後に呼ぶ'),
        F('tokotachi_custom_model_notify_rows_removing', 'void',
          [('obj:CustomModel', 'model'), ('ModelIndex', 'parent'), ('int', 'first'), ('int', 'count')],
          doc='除く前に呼ぶ'),
        F('tokotachi_custom_model_notify_data_changed', 'void', [('obj:CustomModel', 'model'), ('ModelIndex', 'index')]),
        F('tokotachi_custom_model_notify_layout_changed', 'void', [('obj:CustomModel', 'model')]),
        F('tokotachi_custom_model_notify_reset', 'void', [('obj:CustomModel', 'model')]),
    ], doc='関数で中身を答えるモデル(C++ では ItemModel を継承して作るもの)', py_mixin='_CustomModelMixin',
        py_init=False),
]
