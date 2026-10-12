"""Python のバインディングの土台(docs/adr/0065・docs/bindings/python.md)。

C API の共有ライブラリ(tokotachi_c)を ctypes で読み、値の変換・Object の寿命・ハンドラ・描画の型を受け持つ。
クラスとプロパティは _api.py(api.py から生成したもの)にある。
"""

import atexit
import builtins
import ctypes
import ctypes.util
import enum
import glob
import os
import sys
import threading
import weakref
from ctypes import (CFUNCTYPE, POINTER, Structure, byref, c_bool, c_char_p, c_float, c_int, c_size_t,
                    c_uint8, c_uint32, c_void_p)
from typing import NamedTuple, Optional


class TokotachiError(RuntimeError):
    """C API の呼び出しが失敗した(型の違う Object・範囲の外の値・C++ の例外など)。"""


# ---------------------------------------------------------------------------------------------------------------------
# 共有ライブラリ
# ---------------------------------------------------------------------------------------------------------------------

def _candidates():
    env = os.environ.get('TOKOTACHI_C_LIBRARY')
    if env:
        yield env
        return
    here = os.path.join(os.path.dirname(os.path.abspath(__file__)), '_lib')
    for pattern in ('libtokotachi_c.so*', 'libtokotachi_c*.dll', 'tokotachi_c*.dll', 'libtokotachi_c*.dylib'):
        for path in sorted(glob.glob(os.path.join(here, pattern))):
            yield path
    found = ctypes.util.find_library('tokotachi_c')
    if found:
        yield found


def _load():
    tried = []
    for path in _candidates():
        try:
            return ctypes.CDLL(path)
        except OSError as e:
            tried.append('%s (%s)' % (path, e))
    raise ImportError('the C API of Tokotachi (tokotachi_c) was not found. Build it with CMake '
                      '(-DTOKOTACHI_BUILD_C_API=ON) and set TOKOTACHI_C_LIBRARY to the path of the library'
                      + ('. Tried: ' + '; '.join(tried) if tried else ''))


_lib = _load()

# ---------------------------------------------------------------------------------------------------------------------
# 失敗
# ---------------------------------------------------------------------------------------------------------------------

_pending = threading.local()

ERROR_FN = CFUNCTYPE(None, c_char_p, c_void_p)


@ERROR_FN
def _on_error(message, user_data):
    _pending.message = message.decode('utf-8', 'replace') if message else 'unknown error'


def _errcheck(result, func, args):
    message = getattr(_pending, 'message', None)
    if message is not None:
        _pending.message = None
        raise TokotachiError(message)
    return result


def _proto(name, restype, argtypes):
    f = getattr(_lib, name)
    f.restype = restype
    f.argtypes = argtypes
    f.errcheck = _errcheck


_lib.tokotachi_set_error_handler.restype = None
_lib.tokotachi_set_error_handler.argtypes = [ERROR_FN, c_void_p]
_lib.tokotachi_set_error_handler(_on_error, None)

# ---------------------------------------------------------------------------------------------------------------------
# C の構造体
# ---------------------------------------------------------------------------------------------------------------------


class CPoint(Structure):
    _fields_ = [('x', c_float), ('y', c_float)]


class CSize(Structure):
    _fields_ = [('width', c_float), ('height', c_float)]


class CRect(Structure):
    _fields_ = [('x', c_float), ('y', c_float), ('width', c_float), ('height', c_float)]


class CColor(Structure):
    _fields_ = [('r', c_uint8), ('g', c_uint8), ('b', c_uint8), ('a', c_uint8)]


class CMargins(Structure):
    _fields_ = [('left', c_float), ('top', c_float), ('right', c_float), ('bottom', c_float)]


class CFont(Structure):
    _fields_ = [('size', c_float), ('bold', c_bool), ('italic', c_bool)]


class CDate(Structure):
    _fields_ = [('year', c_int), ('month', c_int), ('day', c_int)]


class CCellRange(Structure):
    _fields_ = [('first_row', c_int), ('first_column', c_int), ('last_row', c_int), ('last_column', c_int)]


class CControlState(Structure):
    _fields_ = [('enabled', c_bool), ('hovered', c_bool), ('pressed', c_bool), ('focused', c_bool),
                ('checked', c_bool)]


class CShortcut(Structure):
    _fields_ = [('key', c_int), ('modifiers', ctypes.c_uint)]


class CThemeColor(Structure):
    _fields_ = [('is_role', c_bool), ('role', c_int), ('color', CColor)]


class CModelIndex(Structure):
    _fields_ = [('row', c_int), ('id', c_void_p)]


class CPlacement(Structure):
    _fields_ = [('x', c_float), ('y', c_float), ('relative_x', c_float), ('relative_y', c_float),
                ('width', c_float), ('height', c_float), ('relative_width', c_float), ('relative_height', c_float),
                ('anchor', c_int)]


COLOR_ROLE_COUNT = 24


class CTheme(Structure):
    _fields_ = [('colors', CColor * COLOR_ROLE_COUNT), ('dark_colors', c_bool), ('radius', c_float),
                ('field_radius', c_float), ('field_padding', c_float)]


class CThemeOverride(Structure):
    _fields_ = [('has', c_bool * COLOR_ROLE_COUNT), ('light', CColor * COLOR_ROLE_COUNT),
                ('dark', CColor * COLOR_ROLE_COUNT), ('radius', c_float), ('field_radius', c_float),
                ('field_padding', c_float)]


class CDragEvent(Structure):
    _fields_ = [('pos', CPoint), ('data', c_void_p), ('modifiers', ctypes.c_uint), ('action', c_int),
                ('accepted', c_bool)]


class CMouseEvent(Structure):
    _fields_ = [('pos', CPoint), ('button', c_int), ('modifiers', ctypes.c_uint), ('clicks', c_int),
                ('accepted', c_bool)]


class CKeyEvent(Structure):
    _fields_ = [('key', c_int), ('modifiers', ctypes.c_uint), ('repeat', c_bool), ('native_key', c_uint32),
                ('accepted', c_bool)]


class CWheelEvent(Structure):
    _fields_ = [('pos', CPoint), ('delta', CPoint), ('modifiers', ctypes.c_uint), ('accepted', c_bool)]


class CGradientStop(Structure):
    _fields_ = [('offset', c_float), ('color', CColor)]


class CPaint(Structure):
    _fields_ = [('type', c_int), ('color', CColor), ('p0', CPoint), ('p1', CPoint), ('radius', c_float),
                ('stops', POINTER(CGradientStop)), ('stop_count', c_size_t)]


class CStrokeStyle(Structure):
    _fields_ = [('width', c_float), ('cap', c_int), ('join', c_int), ('dashes', POINTER(c_float)),
                ('dash_count', c_size_t), ('dash_offset', c_float)]


DESTROY_FN = CFUNCTYPE(None, c_void_p)
CALLBACK_FN = CFUNCTYPE(None, c_void_p)
NOTIFY_FN = CFUNCTYPE(None, c_void_p, c_void_p)
MOUSE_FN = CFUNCTYPE(None, c_void_p, POINTER(CMouseEvent), c_void_p)
KEY_FN = CFUNCTYPE(None, c_void_p, POINTER(CKeyEvent), c_void_p)
WHEEL_FN = CFUNCTYPE(None, c_void_p, POINTER(CWheelEvent), c_void_p)
PAINT_FN = CFUNCTYPE(None, c_void_p, c_void_p, c_void_p)
CLOSE_FN = CFUNCTYPE(None, c_void_p, POINTER(c_bool), c_void_p)
TASK_FN = CFUNCTYPE(None, c_void_p, c_void_p)
DRAG_FN = CFUNCTYPE(None, c_void_p, POINTER(CDragEvent), c_void_p)
DRAG_FINISH_FN = CFUNCTYPE(None, c_void_p, c_int, c_void_p)

_INDEX_P = POINTER(CModelIndex)


class CModelCallbacks(Structure):
    _fields_ = [
        ('row_count', CFUNCTYPE(c_int, _INDEX_P, c_void_p)),
        ('index_id', CFUNCTYPE(c_void_p, c_int, _INDEX_P, c_void_p)),
        ('parent_of', CFUNCTYPE(c_bool, _INDEX_P, _INDEX_P, c_void_p)),
        ('text', CFUNCTYPE(c_void_p, _INDEX_P, c_int, c_void_p)),
        ('icon', CFUNCTYPE(c_void_p, _INDEX_P, c_void_p)),
        ('enabled', CFUNCTYPE(c_bool, _INDEX_P, c_void_p)),
        ('checkable', CFUNCTYPE(c_bool, _INDEX_P, c_void_p)),
        ('checked', CFUNCTYPE(c_bool, _INDEX_P, c_void_p)),
        ('set_checked', CFUNCTYPE(None, _INDEX_P, c_bool, c_void_p)),
        ('sort', CFUNCTYPE(None, c_int, c_int, c_void_p)),
        ('locate', CFUNCTYPE(c_bool, _INDEX_P, _INDEX_P, c_void_p)),
        ('text_color', CFUNCTYPE(c_bool, _INDEX_P, c_int, POINTER(CThemeColor), c_void_p)),
        ('font', CFUNCTYPE(c_bool, _INDEX_P, c_int, POINTER(CFont), c_void_p)),
        ('editable', CFUNCTYPE(c_bool, _INDEX_P, c_int, c_void_p)),
        ('set_text', CFUNCTYPE(None, _INDEX_P, c_int, c_char_p, c_void_p)),
        ('can_move_rows', CFUNCTYPE(c_bool, _INDEX_P, c_size_t, _INDEX_P, c_int, c_void_p)),
        ('move_rows', CFUNCTYPE(c_bool, _INDEX_P, c_size_t, _INDEX_P, c_int, c_void_p)),
        ('can_drop', CFUNCTYPE(c_bool, c_void_p, _INDEX_P, c_int, c_void_p)),
        ('drop_data', CFUNCTYPE(c_bool, c_void_p, c_int, _INDEX_P, c_int, c_void_p)),
    ]


# ---------------------------------------------------------------------------------------------------------------------
# 値の型(C++ の同じ名前の型。座標はすべて dp)
# ---------------------------------------------------------------------------------------------------------------------


class Point(NamedTuple):
    x: float = 0.0
    y: float = 0.0


class Size(NamedTuple):
    width: float = 0.0
    height: float = 0.0


class Rect(NamedTuple):
    x: float = 0.0
    y: float = 0.0
    width: float = 0.0
    height: float = 0.0

    @property
    def right(self):
        return self.x + self.width

    @property
    def bottom(self):
        return self.y + self.height

    def contains(self, p):
        return self.x <= p[0] < self.x + self.width and self.y <= p[1] < self.y + self.height


class Color(NamedTuple):
    r: int = 0
    g: int = 0
    b: int = 0
    a: int = 255

    @staticmethod
    def rgb(hex_value, alpha=255):
        """0xRRGGBB から作る。"""
        return Color((hex_value >> 16) & 0xFF, (hex_value >> 8) & 0xFF, hex_value & 0xFF, alpha)

    def with_alpha(self, alpha):
        return Color(self.r, self.g, self.b, alpha)


class Margins(NamedTuple):
    left: float = 0.0
    top: float = 0.0
    right: float = 0.0
    bottom: float = 0.0

    @staticmethod
    def all(value):
        return Margins(value, value, value, value)


class Font(NamedTuple):
    size: float = 14.0
    bold: bool = False
    italic: bool = False


class Date(NamedTuple):
    """日付(すべて 0 なら空の日付)。"""

    year: int = 0
    month: int = 0
    day: int = 0

    @staticmethod
    def today():
        return _out_Date(_lib.tokotachi_date_today())

    @staticmethod
    def parse(text, pattern='yyyy/MM/dd'):
        """pattern で読む。読めなければ空の日付。"""
        return _out_Date(_lib.tokotachi_date_parse(_in_string(text), _in_string(pattern)))

    @staticmethod
    def days_in_month(year, month):
        return _lib.tokotachi_date_days_in_month(year, month)

    @staticmethod
    def is_leap_year(year):
        return bool(_lib.tokotachi_date_is_leap_year(year))

    def is_null(self):
        return self.year == 0 and self.month == 0 and self.day == 0

    def is_valid(self):
        return bool(_lib.tokotachi_date_is_valid(_in_Date(self)))

    def day_of_week(self):
        return _enums().DayOfWeek(_lib.tokotachi_date_day_of_week(_in_Date(self)))

    def add_days(self, days):
        return _out_Date(_lib.tokotachi_date_add_days(_in_Date(self), days))

    def add_months(self, months):
        return _out_Date(_lib.tokotachi_date_add_months(_in_Date(self), months))

    def add_years(self, years):
        return _out_Date(_lib.tokotachi_date_add_years(_in_Date(self), years))

    def days_to(self, other):
        return _lib.tokotachi_date_days_to(_in_Date(self), _in_Date(other))

    def format(self, pattern='yyyy/MM/dd'):
        return _out_string(_lib.tokotachi_date_format(_in_Date(self), _in_string(pattern)))


class CellRange(NamedTuple):
    """表のセルの矩形の範囲(行・列の番号。両端を含む)。"""

    first_row: int = 0
    first_column: int = 0
    last_row: int = -1
    last_column: int = -1

    def is_empty(self):
        return self.last_row < self.first_row or self.last_column < self.first_column

    def contains(self, row, column):
        return self.first_row <= row <= self.last_row and self.first_column <= column <= self.last_column


class ControlState(NamedTuple):
    """部品の状態(標準の見た目を描く関数 draw_* に渡す)。"""

    enabled: bool = True
    hovered: bool = False
    pressed: bool = False
    focused: bool = False
    checked: bool = False


class ModelIndex(NamedTuple):
    """モデルの中の項目の位置(row が負なら無効)。id はモデルが決める整数(無ければ None)。"""

    row: int = -1
    id: Optional[int] = None

    def is_valid(self):
        return self.row >= 0


class Modifiers(enum.IntFlag):
    NONE = 0
    SHIFT = 1 << 0
    CONTROL = 1 << 1
    ALT = 1 << 2
    META = 1 << 3


class Shortcut(NamedTuple):
    """キーと修飾キー。文字("Ctrl+S")からも作れる(Shortcut.parse)。"""

    key: int = 0
    modifiers: int = 0

    @staticmethod
    def parse(text):
        return _out_Shortcut(_lib.tokotachi_shortcut_parse(_in_string(text)))

    def __str__(self):
        return _out_string(_lib.tokotachi_shortcut_to_string(_in_Shortcut(self)))

    def is_empty(self):
        return self.key == 0


class Placement(NamedTuple):
    x: float = 0.0
    y: float = 0.0
    relative_x: float = 0.0
    relative_y: float = 0.0
    width: float = 0.0
    height: float = 0.0
    relative_width: float = 0.0
    relative_height: float = 0.0
    anchor: int = 0


class Theme:
    """テーマ(値)。色は ColorRole で引く(theme[ColorRole.ACCENT] = Color.rgb(0x059669))。"""

    def __init__(self, colors=None, dark_colors=False, radius=6.0, field_radius=4.0, field_padding=8.0):
        if colors is None:
            base = Theme.light()
            colors, dark_colors = base.colors, base.dark_colors
            radius, field_radius, field_padding = base.radius, base.field_radius, base.field_padding
        self.colors = list(colors)
        self.dark_colors = dark_colors
        self.radius = radius
        self.field_radius = field_radius
        self.field_padding = field_padding

    @staticmethod
    def light():
        t = CTheme()
        _lib.tokotachi_theme_light(byref(t))
        return _out_Theme(t)

    @staticmethod
    def dark():
        t = CTheme()
        _lib.tokotachi_theme_dark(byref(t))
        return _out_Theme(t)

    def __getitem__(self, role):
        return self.colors[int(role)]

    def __setitem__(self, role, color):
        self.colors[int(role)] = Color(*color)

    def __eq__(self, other):
        return isinstance(other, Theme) and vars(self) == vars(other)

    def __repr__(self):
        return 'Theme(dark_colors=%r, radius=%r)' % (self.dark_colors, self.radius)


class ThemeOverride:
    """テーマの一部の上書き(Widget.theme_override。docs/adr/0042)。

    >>> o = ThemeOverride().set(ColorRole.ACCENT, Color.rgb(0x059669))
    >>> widget.theme_override = o
    """

    def __init__(self):
        self.light = {}  # ColorRole → Color
        self.dark = {}
        self.radius = -1.0  # 負なら上書きしない
        self.field_radius = -1.0
        self.field_padding = -1.0

    def set(self, role, color, dark=None):
        """役割の色を上書きする(dark を渡すと、ダークのテーマでの色を別にする)。"""
        self.light[_enums().ColorRole(role)] = Color(*color)
        self.dark[_enums().ColorRole(role)] = Color(*(dark if dark is not None else color))
        return self

    def unset(self, role):
        self.light.pop(role, None)
        self.dark.pop(role, None)
        return self

    def has(self, role):
        return role in self.light

    def is_empty(self):
        return not self.light and self.radius < 0 and self.field_radius < 0 and self.field_padding < 0

    def __eq__(self, other):
        return isinstance(other, ThemeOverride) and vars(self) == vars(other)

    def __repr__(self):
        return 'ThemeOverride(%r)' % ({int(k): v for k, v in self.light.items()},)


def resolve_color(color):
    """ColorRole・Color を、今のアプリケーションのテーマで具体的な色にする。"""
    return _out_Color(_lib.tokotachi_theme_color_resolve(_in_ThemeColor(color)))


# ---------------------------------------------------------------------------------------------------------------------
# 値の変換(_api.py が使う)
# ---------------------------------------------------------------------------------------------------------------------

def _enums():
    from . import _api
    return _api


def _in_string(v):
    if v is None:
        return b''
    if isinstance(v, bytes):
        return v
    return str(v).encode('utf-8')


def _out_string(p):
    if not p:
        return ''
    try:
        return ctypes.string_at(p).decode('utf-8', 'replace')
    finally:
        _lib.tokotachi_free(p)


def _in_strings(values):
    values = [_in_string(v) for v in values]
    return (c_char_p * max(len(values), 1))(*values), len(values)


def _out_strings(p):
    if not p:
        return []
    array = ctypes.cast(p, POINTER(c_char_p))
    out = []
    i = 0
    while array[i] is not None:
        out.append(array[i].decode('utf-8', 'replace'))
        i += 1
    _lib.tokotachi_strv_free(p)
    return out


def _in_floats(values):
    values = [float(v) for v in values]
    return (c_float * max(len(values), 1))(*values), len(values)


def _get_buffer(fn, h, ctype):
    n = fn(h, None, 0)
    if n == 0:
        return []
    array = (ctype * n)()
    fn(h, array, n)
    return list(array)


def _get_floats(fn, h):
    return _get_buffer(fn, h, c_float)


def _get_indexes(fn, h):
    return [_out_ModelIndex(i) for i in _get_buffer(fn, h, CModelIndex)]


def _get_objects(fn, h):
    return [_wrap(p) for p in _get_buffer(fn, h, c_void_p)]


def _get_out(fn, h, ctype):
    out = ctype()
    fn(h, byref(out))
    return out


def _in_Point(v):
    return CPoint(*v)


def _out_Point(c):
    return Point(c.x, c.y)


def _in_Size(v):
    return CSize(*v)


def _out_Size(c):
    return Size(c.width, c.height)


def _in_Rect(v):
    return CRect(*v)


def _out_Rect(c):
    return Rect(c.x, c.y, c.width, c.height)


def _in_Color(v):
    if isinstance(v, int):
        v = Color.rgb(v)
    return CColor(*v)


def _out_Color(c):
    return Color(c.r, c.g, c.b, c.a)


def _in_Margins(v):
    if isinstance(v, (int, float)):
        v = Margins.all(v)
    return CMargins(*v)


def _out_Margins(c):
    return Margins(c.left, c.top, c.right, c.bottom)


def _in_Font(v):
    if isinstance(v, (int, float)):
        return CFont(v, False, False)
    v = Font(*v)
    return CFont(v.size, v.bold, v.italic)


def _out_Font(c):
    return Font(c.size, c.bold, c.italic)


def _in_Shortcut(v):
    if isinstance(v, str):
        v = Shortcut.parse(v)
    elif v is None:
        v = Shortcut()
    return CShortcut(int(v[0]), int(v[1]))


def _out_Shortcut(c):
    return Shortcut(_enums().Key(c.key), Modifiers(c.modifiers))


def _in_ThemeColor(v):
    if isinstance(v, _enums().ColorRole):
        return CThemeColor(True, int(v), CColor())
    return CThemeColor(False, 0, _in_Color(v))


def _out_ThemeColor(c):
    if c.is_role:
        return _enums().ColorRole(c.role)
    return _out_Color(c.color)


def _in_ModelIndex(v):
    if v is None:
        return CModelIndex(-1, None)
    return CModelIndex(v[0], v[1])


def _out_ModelIndex(c):
    return ModelIndex(c.row, c.id) if c.row >= 0 else ModelIndex()


def _in_Placement(v):
    v = tuple(v)
    return CPlacement(*(v[:8] + (int(v[8]),) if len(v) > 8 else v))


def _out_Placement(c):
    return Placement(c.x, c.y, c.relative_x, c.relative_y, c.width, c.height, c.relative_width, c.relative_height,
                     _enums().Anchor(c.anchor))


def _in_Theme(v):
    t = CTheme()
    for i, color in enumerate(v.colors):
        t.colors[i] = _in_Color(color)
    t.dark_colors = v.dark_colors
    t.radius = v.radius
    t.field_radius = v.field_radius
    t.field_padding = v.field_padding
    return byref(t)


def _out_Theme(c):
    return Theme([_out_Color(c.colors[i]) for i in range(COLOR_ROLE_COUNT)], c.dark_colors, c.radius, c.field_radius,
                 c.field_padding)


def _in_Date(v):
    if v is None:
        return CDate()
    return CDate(*v)


def _out_Date(c):
    return Date(c.year, c.month, c.day)


def _in_CellRange(v):
    return CCellRange(*v)


def _out_CellRange(c):
    return CellRange(c.first_row, c.first_column, c.last_row, c.last_column)


def _in_ControlState(v):
    return CControlState(*v)


def _get_ranges(fn, h):
    return [_out_CellRange(r) for r in _get_buffer(fn, h, CCellRange)]


def _in_indexes(values):
    values = [_in_ModelIndex(v) for v in values]
    return (CModelIndex * max(len(values), 1))(*values), len(values)


def _in_ThemeOverride(v):
    o = CThemeOverride()
    for role, color in v.light.items():
        o.has[int(role)] = True
        o.light[int(role)] = _in_Color(color)
        o.dark[int(role)] = _in_Color(v.dark.get(role, color))
    o.radius = v.radius
    o.field_radius = v.field_radius
    o.field_padding = v.field_padding
    return byref(o)


def _out_ThemeOverride(c):
    roles = _enums().ColorRole
    v = ThemeOverride()
    for i in range(COLOR_ROLE_COUNT):
        if c.has[i]:
            v.light[roles(i)] = _out_Color(c.light[i])
            v.dark[roles(i)] = _out_Color(c.dark[i])
    v.radius = c.radius
    v.field_radius = c.field_radius
    v.field_padding = c.field_padding
    return v


def _in_DragData(v):
    if not isinstance(v, DragData) or not v._p:
        raise TypeError('expected DragData, got %r' % (v,))
    return v._p


def _in_Image(v):
    if v is None:
        return None
    if not isinstance(v, Image):
        raise TypeError('expected an Image, got %r' % (v,))
    return v._handle()


def _out_Image(p):
    return Image._take(p) if p else None


def _out_Bitmap(p):
    return Bitmap._take(p) if p else None


# ---------------------------------------------------------------------------------------------------------------------
# Object の寿命
# ---------------------------------------------------------------------------------------------------------------------
#
# - Python の Object は C++ の Object 1 つに 1 つ(同じアドレスなら同じ Python の Object を返す)。
# - 所有者のある Object(親のあるウィジェットなど)は C++ の所有者が寿命を決める。Python の Object は C++ の Object が
#   破棄されるまで _pinned に残る(ハンドラの sender・children などで同じものが返る)。
# - Python で作った所有者の無い Object(Window・Application など)は、Python の Object が無くなったとき・delete() で破棄する。
# - C++ の Object が破棄された後に触ると RuntimeError。

_CLASSES = {}
_pinned = {}
_live = weakref.WeakValueDictionary()
_shutting_down = False


def _register(cls):
    _CLASSES[cls.__name__] = cls
    return cls


@DESTROY_FN
def _on_destroyed(address):
    w = _pinned.pop(address, None) or _live.get(address)
    if w is not None and w._ptr == address:
        w._ptr = None
    _live.pop(address, None)


def _wrap(p):
    """C のハンドルを Python の Object にする(NULL なら None)。"""
    if not p:
        return None
    w = _pinned.get(p)
    if w is None:
        w = _live.get(p)
    if w is not None and w._ptr == p:
        return w
    name = _lib.tokotachi_object_class_name(p).decode('ascii')
    cls = _CLASSES.get(name, _CLASSES['Object'])
    w = cls.__new__(cls)
    w._attach(p, False)
    return w


def _repin(obj):
    """所有者が変わったかもしれない Object の、Python の Object の持ち方を直す。"""
    if not isinstance(obj, _ObjectBase) or obj._ptr is None:
        return
    if not obj._py_owned or _lib.tokotachi_object_get_owner(obj._ptr):
        _pinned[obj._ptr] = obj
    else:
        _pinned.pop(obj._ptr, None)


def _delete_on_main_thread(address, keep):
    if _lib.tokotachi_is_main_thread() or not _lib.tokotachi_application_instance():
        _lib.tokotachi_object_delete(address)
        return
    # 別のスレッドで回収されたときは、メインスレッドで破棄する(docs/adr/0029)。keep(独自のモデルの関数など)は
    # C++ の Object が破棄されるまで保つ。
    post(lambda: (_lib.tokotachi_object_delete(address), keep))


class _ObjectBase:
    _ptr = None
    _py_owned = False

    def __init__(self, *args, **kwargs):
        raise TypeError('%s cannot be created directly' % type(self).__name__)

    def _attach(self, p, py_owned):
        if not p:
            raise TokotachiError('failed to create %s' % type(self).__name__)
        _lib.tokotachi_object_set_user_data(p, p, _on_destroyed)
        self._ptr = p
        self._py_owned = py_owned
        _live[p] = self
        _repin(self)

    @property
    def _h(self):
        p = self._ptr
        if p is None:
            raise RuntimeError('the %s has been deleted' % type(self).__name__)
        return p

    @property
    def is_deleted(self):
        """C++ の Object が破棄されたか。"""
        return self._ptr is None

    def delete(self):
        """C++ の Object を破棄する(所有しているものも一緒に破棄される)。"""
        p = self._ptr
        if p is not None:
            _lib.tokotachi_object_delete(p)

    def delete_later(self):
        """今のイベントの処理が終わった後に破棄する(自分のハンドラの中から自分を破棄するとき)。"""
        _lib.tokotachi_object_delete_later(self._h)

    def __del__(self):
        p = self._ptr
        if p is None or _shutting_down or not self._py_owned:
            return
        try:
            if _lib.tokotachi_object_get_owner(p):
                _pinned[p] = self  # 所有者ができていた(C++ の所有者が寿命を決める)
                return
            self._ptr = None
            _delete_on_main_thread(p, self.__dict__.get('_callbacks'))
        except Exception:
            pass

    def __repr__(self):
        if self._ptr is None:
            return '<%s (deleted)>' % type(self).__name__
        return '<%s at 0x%x>' % (type(self).__name__, self._ptr)


def _shutdown():
    """終わるときに、Python で作った所有者の無い Object を破棄する(Application は最後)。"""
    global _shutting_down
    if _shutting_down:
        return
    objects = [w for w in list(_live.values()) + list(_pinned.values())
               if w._ptr is not None and w._py_owned]
    apps = [w for w in objects if type(w).__name__ == 'Application' or isinstance(w, _ApplicationMixin)]
    for w in objects:
        if w in apps or w._ptr is None:
            continue
        try:
            if not _lib.tokotachi_object_get_owner(w._ptr):
                w.delete()
        except Exception:
            pass
    for w in apps:
        w.delete()
    _shutting_down = True


atexit.register(_shutdown)


class _Property:
    """C++ の Property・ReadOnlyProperty。"""

    def __init__(self, getter, setter, repin=False):
        self._getter = getter
        self._setter = setter
        self._repin = repin

    def __set_name__(self, owner, name):
        self._name = name

    def __get__(self, obj, objtype=None):
        if obj is None:
            return self
        return self._getter(obj._h)

    def __set__(self, obj, value):
        if self._setter is None:
            raise AttributeError('%s is read-only' % self._name)
        self._setter(obj._h, value)
        if self._repin:
            _repin(value)
            _repin(obj)


# ---------------------------------------------------------------------------------------------------------------------
# ハンドラ
# ---------------------------------------------------------------------------------------------------------------------

_callbacks = {}
_next_token = [0]
_token_lock = threading.Lock()


def _store(value):
    with _token_lock:
        _next_token[0] += 1
        token = _next_token[0]
    _callbacks[token] = value
    return token


@DESTROY_FN
def _release(token):
    _callbacks.pop(token, None)


def _report(exc_info):
    try:
        sys.excepthook(*exc_info)
    except Exception:
        pass


def _call(token, *args):
    fn = _callbacks.get(token)
    if fn is None:
        return None
    try:
        return fn(*args)
    except BaseException:
        _report(sys.exc_info())
        return None


class MouseEvent:
    """マウスのボタン・移動(位置はウィジェットの左上が原点の dp)。accepted を False にすると親へ送る。"""

    __slots__ = ('pos', 'button', 'modifiers', 'clicks', 'accepted')

    def has_modifier(self, m):
        return (self.modifiers & m) == m


class KeyEvent:
    __slots__ = ('key', 'modifiers', 'repeat', 'native_key', 'accepted')

    def has_modifier(self, m):
        return (self.modifiers & m) == m


class WheelEvent:
    __slots__ = ('pos', 'delta', 'modifiers', 'accepted')

    def has_modifier(self, m):
        return (self.modifiers & m) == m


class DragData:
    """ドラッグの中身(文字・ファイル・名前の付いた中身)。DragData() で作って Widget.start_drag に渡す。

    ドラッグのハンドラ(on_drag_move・on_drop)の e.data は借りもので、ハンドラの中だけで使える。
    """

    __slots__ = ('_p', '_owned')

    def __init__(self, text=None, files=None, formats=None):
        self._p = _lib.tokotachi_drag_data_new()
        self._owned = True
        if text is not None:
            self.text = text
        if files is not None:
            self.files = files
        for name, value in (formats or {}).items():
            self.set_format(name, value)

    @classmethod
    def _borrow(cls, p):
        self = object.__new__(cls)
        self._p = p
        self._owned = False
        return self

    def __del__(self):
        p = getattr(self, '_p', None)
        if p and self._owned:
            self._p = None
            _lib.tokotachi_drag_data_free(p)

    @property
    def _h(self):
        if not self._p:
            raise RuntimeError('the drag data can be used only inside the drag handler')
        return self._p

    @property
    def text(self):
        return _out_string(_lib.tokotachi_drag_data_get_text(self._h))

    @text.setter
    def text(self, value):
        _lib.tokotachi_drag_data_set_text(self._h, _in_string(value))

    @property
    def files(self):
        return _out_strings(_lib.tokotachi_drag_data_get_files(self._h))

    @files.setter
    def files(self, value):
        _lib.tokotachi_drag_data_set_files(self._h, *_in_strings(value))

    def format(self, name):
        """名前の付いた中身(無ければ None)。"""
        p = _lib.tokotachi_drag_data_get_format(self._h, _in_string(name))
        return _out_string(p) if p else None

    def set_format(self, name, value):
        _lib.tokotachi_drag_data_set_format(self._h, _in_string(name), _in_string(value))

    @property
    def formats(self):
        return {name: self.format(name) for name in _out_strings(_lib.tokotachi_drag_data_format_names(self._h))}

    @property
    def source(self):
        """ドラッグを始めた部品(OS からなら None)。"""
        return _wrap(_lib.tokotachi_drag_data_source(self._h))

    @property
    def from_outside(self):
        return bool(_lib.tokotachi_drag_data_from_outside(self._h))


class DragEvent:
    """ドラッグが上を動いた・落とされた。accepted を False にすると受け取らない。action は受け取り方(変えてよい)。"""

    __slots__ = ('pos', 'data', 'modifiers', 'action', 'accepted')

    def has_modifier(self, m):
        return (self.modifiers & m) == m


class CloseEvent:
    """can_close を False にすると閉じない。"""

    __slots__ = ('can_close',)


@NOTIFY_FN
def _dispatch_notify(sender, token):
    _call(token, _wrap(sender))


@MOUSE_FN
def _dispatch_mouse(sender, e, token):
    c = e.contents
    ev = MouseEvent()
    ev.pos = _out_Point(c.pos)
    ev.button = _enums().MouseButton(c.button)
    ev.modifiers = Modifiers(c.modifiers)
    ev.clicks = c.clicks
    ev.accepted = c.accepted
    _call(token, _wrap(sender), ev)
    c.accepted = bool(ev.accepted)


@KEY_FN
def _dispatch_key(sender, e, token):
    c = e.contents
    ev = KeyEvent()
    ev.key = _enums().Key(c.key)
    ev.modifiers = Modifiers(c.modifiers)
    ev.repeat = c.repeat
    ev.native_key = c.native_key
    ev.accepted = c.accepted
    _call(token, _wrap(sender), ev)
    c.accepted = bool(ev.accepted)


@WHEEL_FN
def _dispatch_wheel(sender, e, token):
    c = e.contents
    ev = WheelEvent()
    ev.pos = _out_Point(c.pos)
    ev.delta = _out_Point(c.delta)
    ev.modifiers = Modifiers(c.modifiers)
    ev.accepted = c.accepted
    _call(token, _wrap(sender), ev)
    c.accepted = bool(ev.accepted)


@PAINT_FN
def _dispatch_paint(sender, canvas, token):
    cv = Canvas(canvas)
    try:
        _call(token, _wrap(sender), cv)
    finally:
        cv._p = None  # ハンドラの外では使えない


@CLOSE_FN
def _dispatch_close(sender, can_close, token):
    ev = CloseEvent()
    ev.can_close = can_close[0]
    _call(token, _wrap(sender), ev)
    can_close[0] = bool(ev.can_close)


@DRAG_FN
def _dispatch_drag(sender, e, token):
    c = e.contents
    ev = DragEvent()
    ev.pos = _out_Point(c.pos)
    ev.data = DragData._borrow(c.data)
    ev.modifiers = Modifiers(c.modifiers)
    ev.action = _enums().DropAction(c.action)
    ev.accepted = c.accepted
    try:
        _call(token, _wrap(sender), ev)
    finally:
        ev.data._p = None
    c.action = int(ev.action)
    c.accepted = bool(ev.accepted)


@DRAG_FINISH_FN
def _dispatch_drag_finish(sender, action, token):
    _call(token, _wrap(sender), _enums().DropAction(action))


_DISPATCH = {
    'notify': _dispatch_notify,
    'mouse': _dispatch_mouse,
    'key': _dispatch_key,
    'wheel': _dispatch_wheel,
    'paint': _dispatch_paint,
    'close': _dispatch_close,
    'drag': _dispatch_drag,
    'drag_finish': _dispatch_drag_finish,
}


class _Handler:
    """ハンドラのプロパティ(on_xxx)。呼べるものを代入し、None で外す。第 1 引数は発生元(sender)。"""

    def __init__(self, kind, setter):
        self._kind = kind
        self._setter = setter

    def __set_name__(self, owner, name):
        self._name = name

    def __get__(self, obj, objtype=None):
        if obj is None:
            return self
        return obj.__dict__.get('_handler_' + self._name)

    def __set__(self, obj, fn):
        setter = getattr(_lib, self._setter)
        if fn is None:
            setter(obj._h, None, None, None)
        else:
            if not callable(fn):
                raise TypeError('%s must be callable' % self._name)
            setter(obj._h, _DISPATCH[self._kind], _store(fn), _release)
        obj.__dict__['_handler_' + self._name] = fn


# ---------------------------------------------------------------------------------------------------------------------
# 全体の関数
# ---------------------------------------------------------------------------------------------------------------------

def version():
    """実行時に読み込まれたライブラリの版("0.1.0")。"""
    return _lib.tokotachi_version().decode('ascii')


def is_main_thread():
    """今のスレッドがメインスレッド(Application を作ったスレッド)か。"""
    return bool(_lib.tokotachi_is_main_thread())


@CALLBACK_FN
def _dispatch_post(token):
    fn = _callbacks.get(token)
    if fn is not None:
        try:
            fn()
        except BaseException:
            _report(sys.exc_info())


def post(fn):
    """どのスレッドからでも呼べる: fn() をメインスレッドで呼ぶよう頼む(docs/adr/0029)。Application が無ければ False。"""
    return bool(_lib.tokotachi_post(_dispatch_post, _store(fn), _release))


def bind(target, target_property, source, source_property):
    """target のプロパティを source のプロパティに結ぶ(C++ の tokotachi::bind)。結んだ時点で target は source の値になる。

    >>> bind(line_edit, 'text', name, 'value')
    """
    return bool(_lib.tokotachi_bind(target._h, _in_string(target_property), source._h, _in_string(source_property)))


def unbind(a, a_property, b, b_property):
    return bool(_lib.tokotachi_unbind(a._h, _in_string(a_property), b._h, _in_string(b_property)))


def unbind_all(obj, prop):
    return bool(_lib.tokotachi_unbind_all(obj._h, _in_string(prop)))


class clipboard:
    """クリップボード(テキストのみ)。"""

    @staticmethod
    def text():
        return _out_string(_lib.tokotachi_clipboard_text())

    @staticmethod
    def set_text(text):
        _lib.tokotachi_clipboard_set_text(_in_string(text))

    @staticmethod
    def has_text():
        return bool(_lib.tokotachi_clipboard_has_text())


# ---------------------------------------------------------------------------------------------------------------------
# 画像・描画
# ---------------------------------------------------------------------------------------------------------------------

class Image:
    """変更しない画像(コピーは安い)。Image.load・Image.from_pixels・Bitmap.to_image で作る。"""

    __slots__ = ('_p',)

    def __init__(self):
        raise TypeError('use Image.load, Image.load_from_data or Image.from_pixels')

    @classmethod
    def _take(cls, p):
        self = object.__new__(cls)
        self._p = p
        return self

    def _handle(self):
        return self._p

    def __del__(self):
        p = getattr(self, '_p', None)
        if p:
            self._p = None
            _lib.tokotachi_image_free(p)

    @staticmethod
    def load(path):
        """PNG・JPEG・BMP・GIF を読む。読めなければ None。"""
        return _out_Image(_lib.tokotachi_image_load(_in_string(os.fspath(path))))

    @staticmethod
    def icon(icon, color=None):
        """よく使うアイコン(線画。docs/adr/0054)。color を渡さなければ、使う部品の文字の色で塗る。"""
        if color is None:
            return _out_Image(_lib.tokotachi_image_icon(int(icon)))
        return _out_Image(_lib.tokotachi_image_icon_colored(int(icon), _in_Color(color)))

    @property
    def is_icon(self):
        return bool(_lib.tokotachi_image_is_icon(self._p))

    @staticmethod
    def load_from_data(data):
        data = bytes(data)
        return _out_Image(_lib.tokotachi_image_load_from_data(data, len(data)))

    @staticmethod
    def from_pixels(width, height, pixels):
        """画素(左上から行ごとの Color。アルファは乗算していない)から作る。数が合わなければ None。"""
        pixels = list(pixels)
        array = (CColor * max(len(pixels), 1))(*[_in_Color(p) for p in pixels])
        return _out_Image(_lib.tokotachi_image_from_pixels(width, height, array, len(pixels)))

    @property
    def width(self):
        return _lib.tokotachi_image_width(self._p)

    @property
    def height(self):
        return _lib.tokotachi_image_height(self._p)

    def pixel(self, x, y):
        return _out_Color(_lib.tokotachi_image_pixel(self._p, x, y))

    def __eq__(self, other):
        return isinstance(other, Image) and bool(_lib.tokotachi_image_equal(self._p, other._p))

    def __hash__(self):
        return hash((self.width, self.height))


class Bitmap:
    """書き換えられる画像(大きさはピクセル。scale は dp からピクセルへの倍率)。canvas で描く。"""

    __slots__ = ('_p', '_canvas')

    def __init__(self, width, height, scale=1.0):
        self._p = None
        self._canvas = None
        p = _lib.tokotachi_bitmap_new(width, height, scale)
        if not p:
            raise TokotachiError('failed to create the bitmap')
        self._p = p

    @classmethod
    def _take(cls, p):
        self = object.__new__(cls)
        self._p = p
        self._canvas = None
        return self

    @staticmethod
    def from_image(image, scale=1.0):
        return _out_Bitmap(_lib.tokotachi_bitmap_from_image(image._p, scale))

    def __del__(self):
        p = getattr(self, '_p', None)
        if p:
            if self._canvas is not None:
                self._canvas._p = None
            self._p = None
            _lib.tokotachi_bitmap_free(p)

    @property
    def width(self):
        return _lib.tokotachi_bitmap_width(self._p)

    @property
    def height(self):
        return _lib.tokotachi_bitmap_height(self._p)

    @property
    def scale(self):
        return _lib.tokotachi_bitmap_scale(self._p)

    @property
    def canvas(self):
        """この画像に描く Canvas(Bitmap が生きている間有効)。"""
        if self._canvas is None:
            self._canvas = Canvas(_lib.tokotachi_bitmap_canvas(self._p), self)
        return self._canvas

    def clear(self, color=Color(0, 0, 0, 0)):
        _lib.tokotachi_bitmap_clear(self._p, _in_Color(color))

    def pixel(self, x, y):
        return _out_Color(_lib.tokotachi_bitmap_pixel(self._p, x, y))

    def set_pixel(self, x, y, color):
        _lib.tokotachi_bitmap_set_pixel(self._p, x, y, _in_Color(color))

    def pixels(self):
        """画素の並び(ctypes の配列。アルファ乗算済みの 0xAARRGGBB。Bitmap と同じメモリ)。"""
        p = _lib.tokotachi_bitmap_pixels(self._p)
        return ctypes.cast(p, POINTER(c_uint32 * (self.width * self.height))).contents

    def to_image(self):
        return _out_Image(_lib.tokotachi_bitmap_to_image(self._p))

    def save_png(self, path):
        return bool(_lib.tokotachi_bitmap_save_png(self._p, _in_string(os.fspath(path))))


class Path:
    """塗る・線を引く・クリップする形。"""

    __slots__ = ('_p',)

    def __init__(self):
        self._p = _lib.tokotachi_path_new()

    def __del__(self):
        p = getattr(self, '_p', None)
        if p:
            self._p = None
            _lib.tokotachi_path_free(p)

    def move_to(self, x, y):
        _lib.tokotachi_path_move_to(self._p, x, y)
        return self

    def line_to(self, x, y):
        _lib.tokotachi_path_line_to(self._p, x, y)
        return self

    def quad_to(self, x1, y1, x2, y2):
        _lib.tokotachi_path_quad_to(self._p, x1, y1, x2, y2)
        return self

    def cubic_to(self, x1, y1, x2, y2, x3, y3):
        _lib.tokotachi_path_cubic_to(self._p, x1, y1, x2, y2, x3, y3)
        return self

    def close(self):
        _lib.tokotachi_path_close(self._p)
        return self

    def add_rect(self, rect):
        _lib.tokotachi_path_add_rect(self._p, _in_Rect(rect))
        return self

    def add_round_rect(self, rect, radius):
        _lib.tokotachi_path_add_round_rect(self._p, _in_Rect(rect), radius)
        return self

    def add_ellipse(self, center, rx, ry):
        _lib.tokotachi_path_add_ellipse(self._p, _in_Point(center), rx, ry)
        return self

    def add_arc(self, center, rx, ry, start_angle, sweep_angle, new_figure=True):
        _lib.tokotachi_path_add_arc(self._p, _in_Point(center), rx, ry, start_angle, sweep_angle, new_figure)
        return self

    def add_polyline(self, points, close=False):
        points = list(points)
        array = (CPoint * max(len(points), 1))(*[_in_Point(p) for p in points])
        _lib.tokotachi_path_add_polyline(self._p, array, len(points), close)
        return self

    def is_empty(self):
        return bool(_lib.tokotachi_path_is_empty(self._p))


class TextLayout:
    """文字のレイアウト(作った後は変わらない)。scale は dp からピクセルへの倍率(Widget.display_scale)。"""

    __slots__ = ('_p',)

    def __init__(self, text, font=Font(), scale=1.0, wrap_width=0.0, alignment=0):
        self._p = _lib.tokotachi_text_layout_new(_in_string(text), _in_Font(font), scale, wrap_width, int(alignment))

    def __del__(self):
        p = getattr(self, '_p', None)
        if p:
            self._p = None
            _lib.tokotachi_text_layout_free(p)

    width = property(lambda self: _lib.tokotachi_text_layout_width(self._p))
    height = property(lambda self: _lib.tokotachi_text_layout_height(self._p))
    ascent = property(lambda self: _lib.tokotachi_text_layout_ascent(self._p))
    descent = property(lambda self: _lib.tokotachi_text_layout_descent(self._p))
    line_height = property(lambda self: _lib.tokotachi_text_layout_line_height(self._p))
    line_count = property(lambda self: _lib.tokotachi_text_layout_line_count(self._p))

    def line_rect(self, line):
        return _out_Rect(_lib.tokotachi_text_layout_line_rect(self._p, line))

    def caret_position(self, offset):
        """キャレットの左上(offset は UTF-8 のバイトの位置)。"""
        return _out_Point(_lib.tokotachi_text_layout_caret_position(self._p, offset))

    def offset_at(self, p):
        return _lib.tokotachi_text_layout_offset_at(self._p, _in_Point(p))


class Paint:
    """塗り(単色・線形グラデーション・放射グラデーション)。Canvas の塗りには Color もそのまま渡せる。"""

    __slots__ = ('_c', '_stops')

    def __init__(self, color=Color()):
        self._c = CPaint()
        self._c.type = 0
        self._c.color = _in_Color(color)
        self._stops = None

    @classmethod
    def _gradient(cls, kind, p0, p1, radius, stops):
        self = cls()
        stops = list(stops)
        self._stops = (CGradientStop * max(len(stops), 1))(*[CGradientStop(o, _in_Color(c)) for o, c in stops])
        self._c.type = kind
        self._c.p0 = _in_Point(p0)
        self._c.p1 = _in_Point(p1)
        self._c.radius = radius
        self._c.stops = ctypes.cast(self._stops, POINTER(CGradientStop))
        self._c.stop_count = len(stops)
        return self

    @classmethod
    def linear(cls, start, end, stops):
        """stops は (offset, Color) の並び。"""
        return cls._gradient(1, start, end, 0.0, stops)

    @classmethod
    def radial(cls, center, radius, focal, stops):
        return cls._gradient(2, center, focal, radius, stops)


def _paint(v):
    if isinstance(v, Paint):
        return byref(v._c)
    return byref(Paint(v)._c)


class StrokeStyle:
    """線の形(太さ dp・端・角・破線)。"""

    __slots__ = ('_c', '_dashes')

    def __init__(self, width=1.0, cap=0, join=0, dashes=(), dash_offset=0.0):
        self._c = CStrokeStyle()
        self._c.width = width
        self._c.cap = int(cap)
        self._c.join = int(join)
        dashes = list(dashes)
        self._dashes = (c_float * max(len(dashes), 1))(*dashes)
        self._c.dashes = ctypes.cast(self._dashes, POINTER(c_float))
        self._c.dash_count = len(dashes)
        self._c.dash_offset = dash_offset


def _stroke(v):
    if isinstance(v, StrokeStyle):
        return byref(v._c)
    return byref(StrokeStyle(float(v))._c)


class Canvas:
    """描く先(on_paint の中・Bitmap.canvas)。on_paint の Canvas はハンドラの外では使えない。座標は dp。"""

    __slots__ = ('_p', '_owner')

    def __init__(self, p, owner=None):
        self._p = p
        self._owner = owner

    @property
    def _h(self):
        if not self._p:
            raise RuntimeError('the canvas can be used only inside the paint handler')
        return self._p

    @property
    def scale(self):
        return _lib.tokotachi_canvas_scale(self._h)

    def save(self):
        _lib.tokotachi_canvas_save(self._h)

    def restore(self):
        _lib.tokotachi_canvas_restore(self._h)

    def translate(self, dx, dy):
        _lib.tokotachi_canvas_translate(self._h, dx, dy)

    def scale_by(self, sx, sy):
        _lib.tokotachi_canvas_scale_by(self._h, sx, sy)

    def rotate(self, radians):
        _lib.tokotachi_canvas_rotate(self._h, radians)

    def clip_rect(self, rect):
        _lib.tokotachi_canvas_clip_rect(self._h, _in_Rect(rect))

    def clip_path(self, path):
        _lib.tokotachi_canvas_clip_path(self._h, path._p)

    def clip_bounds(self):
        return _out_Rect(_lib.tokotachi_canvas_clip_bounds(self._h))

    def begin_layer(self, opacity):
        _lib.tokotachi_canvas_begin_layer(self._h, opacity)

    def end_layer(self):
        _lib.tokotachi_canvas_end_layer(self._h)

    def fill_rect(self, rect, color):
        _lib.tokotachi_canvas_fill_rect(self._h, _in_Rect(rect), _in_Color(color))

    def fill_round_rect(self, rect, radius, paint):
        _lib.tokotachi_canvas_fill_round_rect(self._h, _in_Rect(rect), radius, _paint(paint))

    def stroke_round_rect(self, rect, radius, width, color):
        _lib.tokotachi_canvas_stroke_round_rect(self._h, _in_Rect(rect), radius, width, _in_Color(color))

    def stroke_rect(self, rect, width, color):
        _lib.tokotachi_canvas_stroke_rect(self._h, _in_Rect(rect), width, _in_Color(color))

    def fill_path(self, path, paint):
        _lib.tokotachi_canvas_fill_path(self._h, path._p, _paint(paint))

    def stroke_path(self, path, style, paint):
        """style は StrokeStyle か太さ(dp)。"""
        _lib.tokotachi_canvas_stroke_path(self._h, path._p, _stroke(style), _paint(paint))

    def draw_line(self, start, end, style, paint):
        _lib.tokotachi_canvas_draw_line(self._h, _in_Point(start), _in_Point(end), _stroke(style), _paint(paint))

    def fill_ellipse(self, rect, paint):
        _lib.tokotachi_canvas_fill_ellipse(self._h, _in_Rect(rect), _paint(paint))

    def stroke_ellipse(self, rect, style, paint):
        _lib.tokotachi_canvas_stroke_ellipse(self._h, _in_Rect(rect), _stroke(style), _paint(paint))

    def draw_text(self, text, pos, color, font=Font()):
        """1 行の文字(pos は左上)。text に TextLayout も渡せる。"""
        if isinstance(text, TextLayout):
            _lib.tokotachi_canvas_draw_text_layout(self._h, text._p, _in_Point(pos), _in_Color(color))
        else:
            _lib.tokotachi_canvas_draw_text(self._h, _in_string(text), _in_Point(pos), _in_Color(color),
                                            _in_Font(font))

    def draw_image(self, image, dst, opacity=1.0):
        _lib.tokotachi_canvas_draw_image(self._h, image._p, _in_Rect(dst), opacity)

    # 標準の見た目(C++ の tokotachi::draw。docs/adr/0058)
    def draw_button_face(self, rect, state, style=0):
        _lib.tokotachi_draw_button_face(self._h, _in_Rect(rect), _in_ControlState(state), int(style))

    def draw_check_mark(self, rect, state):
        _lib.tokotachi_draw_check_mark(self._h, _in_Rect(rect), _in_ControlState(state))

    def draw_radio_mark(self, rect, state):
        _lib.tokotachi_draw_radio_mark(self._h, _in_Rect(rect), _in_ControlState(state))

    def draw_focus_ring(self, rect):
        _lib.tokotachi_draw_focus_ring(self._h, _in_Rect(rect))

    def draw_field_frame(self, rect, state):
        _lib.tokotachi_draw_field_frame(self._h, _in_Rect(rect), _in_ControlState(state))


def button_text_color(state, style=0):
    """ボタンの面の上に描く文字・画像の色(C++ の tokotachi::draw::button_text_color)。"""
    return _out_Color(_lib.tokotachi_draw_button_text_color(_in_ControlState(state), int(style)))


# ---------------------------------------------------------------------------------------------------------------------
# 生成したクラスに混ぜる手書きの部分
# ---------------------------------------------------------------------------------------------------------------------

class _ApplicationMixin:
    """1 つのプロセスに 1 つだけ作る。最後に破棄される(Python が終わるとき)。"""

    def __init__(self, argv=None):
        if argv is None:
            argv = sys.argv
        args = [_in_string(a) for a in argv] or [b'tokotachi']
        self._argv = (c_char_p * (len(args) + 1))(*args)
        self._attach(_lib.tokotachi_application_new(len(args), self._argv), True)

    @staticmethod
    def instance():
        return _wrap(_lib.tokotachi_application_instance())


class _WindowMixin:
    """Window(title, width, height) は Application が、Window(owner, title, width, height) は owner が所有する(docs/adr/0062)。

    参照が無くなっても破棄しない。閉じた後に破棄するときは destroy_on_close を True にするか、delete() を呼ぶ。
    """

    def __init__(self, *args, **kwargs):
        owner = None
        if args and isinstance(args[0], _CLASSES['Window']):
            owner, args = args[0], args[1:]
        elif 'owner' in kwargs:
            owner = kwargs.pop('owner')
        params = dict(zip(('title', 'width', 'height'), args))
        params.update(kwargs)
        title = params.pop('title', '')
        width = params.pop('width', 640)
        height = params.pop('height', 480)
        if params:
            raise TypeError('unexpected arguments: %s' % ', '.join(params))
        if owner is None:
            p = _lib.tokotachi_window_new(_in_string(title), width, height)
        else:
            p = _lib.tokotachi_window_new_owned(_in_obj(owner, 'Window'), _in_string(title), width, height)
        self._attach(p, True)


class _MessageDialogMixin:
    @staticmethod
    def show(parent, title, text, buttons=(), on_finish=None, type=0):
        """作ってボタンを加えて開き、閉じたら on_finish(dialog) を呼んで破棄する(buttons が空なら OK だけ)。"""
        buttons = [int(b) for b in buttons]
        array = (c_int * max(len(buttons), 1))(*buttons)
        if on_finish is None:
            p = _lib.tokotachi_message_dialog_show(_in_obj(parent, 'Window'), _in_string(title), _in_string(text),
                                                   array, len(buttons), None, None, None, int(type))
        else:
            p = _lib.tokotachi_message_dialog_show(_in_obj(parent, 'Window'), _in_string(title), _in_string(text),
                                                   array, len(buttons), _dispatch_notify, _store(on_finish),
                                                   _release, int(type))
        return _wrap(p)


class _ColorDialogMixin:
    @staticmethod
    def show(parent, initial, on_finish=None, alpha=False, title=''):
        """作って開き、閉じたら on_finish(dialog) を呼んで破棄する(決めた色は dialog.color、結果は dialog.result)。"""
        if on_finish is None:
            p = _lib.tokotachi_color_dialog_show(_in_obj(parent, 'Window'), _in_Color(initial), None, None, None,
                                                 alpha, _in_string(title))
        else:
            p = _lib.tokotachi_color_dialog_show(_in_obj(parent, 'Window'), _in_Color(initial), _dispatch_notify,
                                                 _store(on_finish), _release, alpha, _in_string(title))
        return _wrap(p)


_VARIABLE_ACCESS = ['bool', 'int', 'float', 'double', 'string']


class _VariableMixin:
    """値を 1 つ持つオブジェクト(docs/adr/0016)。bind(widget, 'text', variable, 'value') で結ぶ。

    型は value から決める(bool・int・float(C++ の double)・str)か、type に VariableType・bool・int・float・str を渡す。
    """

    def __init__(self, owner=None, value=None, type=None):
        if type is None:
            if value is None:
                raise TypeError('give the value or the type of the variable')
            type = builtins.type(value)
        self._attach(_lib.tokotachi_variable_new(_in_obj(owner, 'Object'), int(_variable_type(type))), True)
        _repin(owner)
        if value is not None:
            self.value = value

    @property
    def variable_type(self):
        return _enums().VariableType(_lib.tokotachi_variable_get_type(self._h))

    @property
    def value(self):
        h = self._h
        kind = _VARIABLE_ACCESS[_lib.tokotachi_variable_get_type(h)]
        v = getattr(_lib, 'tokotachi_variable_get_' + kind)(h)
        return _out_string(v) if kind == 'string' else v

    @value.setter
    def value(self, v):
        h = self._h
        kind = _VARIABLE_ACCESS[_lib.tokotachi_variable_get_type(h)]
        getattr(_lib, 'tokotachi_variable_set_' + kind)(h, _in_string(v) if kind == 'string' else v)

    on_set = _Handler('notify', 'tokotachi_variable_set_on_set')
    on_change = _Handler('notify', 'tokotachi_variable_set_on_change')


def _variable_type(t):
    api = _enums()
    if isinstance(t, api.VariableType):
        return t
    mapping = {bool: api.VariableType.BOOL, int: api.VariableType.INT, float: api.VariableType.DOUBLE,
               str: api.VariableType.STRING}
    if t in mapping:
        return mapping[t]
    raise TypeError('unsupported variable type: %r' % (t,))


class TaskContext:
    """Task の仕事に渡す(別のスレッドで使う)。"""

    __slots__ = ('_p',)

    def cancel_requested(self):
        return bool(_lib.tokotachi_task_context_cancel_requested(self._p))

    def report_progress(self, value):
        _lib.tokotachi_task_context_report_progress(self._p, value)


@TASK_FN
def _dispatch_task(context, token):
    entry = _callbacks.get(token)
    if entry is None:
        return
    task_ref, work = entry
    ctx = TaskContext()
    ctx._p = context
    try:
        result = work(ctx)
        task = task_ref()
        if task is not None:
            task._result = result
    except BaseException as e:  # 例外は error に入る
        _lib.tokotachi_task_context_fail(context, _in_string('%s: %s' % (type(e).__name__, e)))
    finally:
        ctx._p = None


class _TaskMixin:
    """別のスレッドで行う、終わりのある仕事(docs/adr/0029)。

    >>> task = Task(window)
    >>> task.on_finish = lambda t: print(t.error or t.result)
    >>> task.start(lambda ctx: heavy_work())   # 別のスレッドで動く。ウィジェットには触らない
    """

    _result = None

    def __init__(self, owner=None):
        self._attach(_lib.tokotachi_task_new(_in_obj(owner, 'Object')), True)
        _repin(owner)

    def start(self, work):
        """work(context) を別のスレッドで呼ぶ。戻り値は result に入る。動いていれば何もせず False。"""
        self._result = None
        return bool(_lib.tokotachi_task_start(self._h, _dispatch_task, _store((weakref.ref(self), work)), _release))

    @property
    def result(self):
        """仕事の戻り値(on_finish で、error が空で cancelled でなければ読む)。"""
        return self._result


class _CustomModelMixin:
    """Python のメソッドで中身を答えるモデル。継承して row_count・text を上書きする(必須)。

    木にするときは index_id(row, parent) で子の識別子(0 でない整数)を返し、parent_of(index) で親を返す。
    上書きしないメソッドは C++ の ItemModel の既定の答えになる。中身を変えたら notify_* で知らせる。
    """

    _OPTIONAL = ('index_id', 'parent_of', 'icon', 'enabled', 'checkable', 'checked', 'set_checked', 'sort', 'locate',
                 'text_color', 'font', 'editable', 'set_text', 'can_move_rows', 'move_rows', 'can_drop', 'drop_data')

    def __init__(self, owner=None):
        cls = type(self)
        base = _CLASSES['CustomModel']
        for required in ('row_count', 'text'):
            if getattr(cls, required) is getattr(base, required):
                raise TypeError('%s must override %s' % (cls.__name__, required))
        self._keep = {}
        me = weakref.ref(self)
        cb = CModelCallbacks()
        types = dict(CModelCallbacks._fields_)

        def index_of(p):
            c = p.contents
            return ModelIndex(c.row, c.id) if c.row >= 0 else ModelIndex()

        def guarded(default, fn):
            def wrapper(*args):
                model = me()
                if model is None:
                    return default
                try:
                    return fn(model, *args)
                except BaseException:
                    _report(sys.exc_info())
                    return default
            return wrapper

        def write(out, index):
            if index is None or not index[0] >= 0:
                return False
            out.contents.row = index[0]
            out.contents.id = index[1]
            return True

        def text(model, p, column, ud):
            buf = ctypes.create_string_buffer(_in_string(model.text(index_of(p), column)))
            model._keep['text'] = buf
            return ctypes.addressof(buf)

        def text_color(model, p, column, out, ud):
            v = model.text_color(index_of(p), column)
            if v is None:
                return False
            out.contents.is_role, out.contents.role, out.contents.color = (
                lambda t: (t.is_role, t.role, t.color))(_in_ThemeColor(v))
            return True

        def font(model, p, column, out, ud):
            v = model.font(index_of(p), column)
            if v is None:
                return False
            c = _in_Font(v)
            out.contents.size, out.contents.bold, out.contents.italic = c.size, c.bold, c.italic
            return True

        def borrowed(p, fn):
            data = DragData._borrow(p)
            try:
                return bool(fn(data))
            finally:
                data._p = None

        def drop(fn):
            return lambda model, p, parent, row, ud: borrowed(p, lambda data: fn(model, data, parent, row))

        def icon(model, p, ud):
            image = model.icon(index_of(p))
            model._keep['icon'] = image
            return image._p if image is not None else None

        impl = {
            'row_count': (0, lambda m, p, ud: int(m.row_count(index_of(p)))),
            'text': (None, text),
            'index_id': (None, lambda m, row, p, ud: m.index_id(row, index_of(p))),
            'parent_of': (False, lambda m, p, out, ud: write(out, m.parent_of(index_of(p)))),
            'icon': (None, icon),
            'enabled': (True, lambda m, p, ud: bool(m.enabled(index_of(p)))),
            'checkable': (False, lambda m, p, ud: bool(m.checkable(index_of(p)))),
            'checked': (False, lambda m, p, ud: bool(m.checked(index_of(p)))),
            'set_checked': (None, lambda m, p, v, ud: m.set_checked(index_of(p), bool(v))),
            'sort': (None, lambda m, column, order, ud: m.sort(column, _enums().SortOrder(order))),
            'locate': (False, lambda m, p, out, ud: write(out, m.locate(index_of(p)))),
            'text_color': (False, text_color),
            'font': (False, font),
            'editable': (False, lambda m, p, column, ud: bool(m.editable(index_of(p), column))),
            'set_text': (None, lambda m, p, column, text, ud: m.set_text(index_of(p), column,
                                                                          text.decode('utf-8', 'replace'))),
            'can_move_rows': (False, lambda m, p, n, parent, row, ud: bool(m.can_move_rows(
                [_out_ModelIndex(p[i]) for i in range(n)], index_of(parent), row))),
            'move_rows': (False, lambda m, p, n, parent, row, ud: bool(m.move_rows(
                [_out_ModelIndex(p[i]) for i in range(n)], index_of(parent), row))),
            'can_drop': (False, drop(lambda m, data, parent, row: m.can_drop(data, index_of(parent), row))),
            'drop_data': (False, lambda m, d, action, parent, row, ud: borrowed(
                d, lambda data: m.drop_data(data, _enums().DropAction(action), index_of(parent), row))),
        }
        functions = []
        for name, (default, fn) in impl.items():
            if name in self._OPTIONAL and getattr(cls, name, None) is getattr(base, name, None):
                continue
            f = types[name](guarded(default, fn))
            functions.append(f)
            setattr(cb, name, f)
        self._callbacks = (cb, functions)
        self._attach(_lib.tokotachi_custom_model_new(_in_obj(owner, 'Object'), byref(cb), None, None), True)
        _repin(owner)

    def index_id(self, row, parent):
        """parent の row 番目の子の識別子(0 でない整数)。上書きしなければ一覧のモデル。"""
        return None

    def notify_rows_inserted(self, parent, first, count):
        _lib.tokotachi_custom_model_notify_rows_inserted(self._h, _in_ModelIndex(parent), first, count)

    def notify_rows_removing(self, parent, first, count):
        _lib.tokotachi_custom_model_notify_rows_removing(self._h, _in_ModelIndex(parent), first, count)

    def notify_data_changed(self, index):
        _lib.tokotachi_custom_model_notify_data_changed(self._h, _in_ModelIndex(index))

    def notify_layout_changed(self):
        _lib.tokotachi_custom_model_notify_layout_changed(self._h)

    def notify_reset(self):
        _lib.tokotachi_custom_model_notify_reset(self._h)


def _in_obj(v, cls_name):
    if v is None:
        return None
    cls = _CLASSES[cls_name]
    if not isinstance(v, cls):
        raise TypeError('expected %s, got %r' % (cls_name, v))
    return v._h
