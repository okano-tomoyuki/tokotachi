# api.py から generate.py で作ったファイル。手で書き換えない。
# flake8: noqa
from ctypes import POINTER, c_bool, c_char_p, c_double, c_float, c_int, c_size_t, c_uint32, c_void_p
import enum

from ._core import *  # noqa: F401,F403
from ._core import _lib, _ObjectBase, _Property, _Handler, _proto, _register, _wrap, _repin
from ._core import (_in_string, _out_string, _in_strings, _out_strings, _in_floats, _get_floats,
                    _get_indexes, _get_objects, _get_out, _in_obj, _in_Image, _out_Image, _out_Bitmap,
                    _in_Theme, _out_Theme)
from ._core import (_in_Point, _out_Point, _in_Size, _out_Size, _in_Rect, _out_Rect, _in_Color, _out_Color,
                    _in_Margins, _out_Margins, _in_Font, _out_Font, _in_Shortcut, _out_Shortcut,
                    _in_ThemeColor, _out_ThemeColor, _in_ModelIndex, _out_ModelIndex, _in_Placement,
                    _out_Placement)
from ._core import (_in_indexes, _get_ranges, _in_DragData, _in_Date, _out_Date, _in_CellRange,
                    _out_CellRange, _in_ThemeOverride, _out_ThemeOverride, _in_ControlState)
from ._core import (_ApplicationMixin, _WindowMixin, _MessageDialogMixin, _ColorDialogMixin, _VariableMixin,
                    _TaskMixin, _CustomModelMixin)

# ---- 列挙 ----


class MouseButton(enum.IntEnum):
    """C++ の tokotachi::MouseButton"""

    NONE = 0
    LEFT = 1
    MIDDLE = 2
    RIGHT = 3


class Key(enum.IntEnum):
    """C++ の tokotachi::Key"""

    UNKNOWN = 0
    BACKSPACE = 1
    TAB = 2
    RETURN = 3
    ESCAPE = 4
    SPACE = 5
    DEL = 6
    INSERT = 7
    LEFT = 8
    RIGHT = 9
    UP = 10
    DOWN = 11
    HOME = 12
    END = 13
    PAGE_UP = 14
    PAGE_DOWN = 15
    A = 16
    B = 17
    C = 18
    D = 19
    E = 20
    F = 21
    G = 22
    H = 23
    I = 24
    J = 25
    K = 26
    L = 27
    M = 28
    N = 29
    O = 30
    P = 31
    Q = 32
    R = 33
    S = 34
    T = 35
    U = 36
    V = 37
    W = 38
    X = 39
    Y = 40
    Z = 41
    DIGIT_0 = 42
    DIGIT_1 = 43
    DIGIT_2 = 44
    DIGIT_3 = 45
    DIGIT_4 = 46
    DIGIT_5 = 47
    DIGIT_6 = 48
    DIGIT_7 = 49
    DIGIT_8 = 50
    DIGIT_9 = 51
    F1 = 52
    F2 = 53
    F3 = 54
    F4 = 55
    F5 = 56
    F6 = 57
    F7 = 58
    F8 = 59
    F9 = 60
    F10 = 61
    F11 = 62
    F12 = 63
    SHIFT = 64
    CONTROL = 65
    ALT = 66
    META = 67


class FocusPolicy(enum.IntEnum):
    """C++ の tokotachi::FocusPolicy"""

    NONE = 0
    TAB = 1
    CLICK = 2
    TAB_AND_CLICK = 3


class MouseCursor(enum.IntEnum):
    """C++ の tokotachi::MouseCursor"""

    ARROW = 0
    IBEAM = 1
    POINTING_HAND = 2
    WAIT = 3
    CROSSHAIR = 4
    SIZE_HORIZONTAL = 5
    SIZE_VERTICAL = 6


class Orientation(enum.IntEnum):
    """C++ の tokotachi::Orientation"""

    HORIZONTAL = 0
    VERTICAL = 1


class Align(enum.IntEnum):
    """C++ の tokotachi::Align"""

    START = 0
    CENTER = 1
    END = 2
    FILL = 3


class Anchor(enum.IntEnum):
    """C++ の tokotachi::Anchor"""

    TOP_LEFT = 0
    TOP = 1
    TOP_RIGHT = 2
    LEFT = 3
    CENTER = 4
    RIGHT = 5
    BOTTOM_LEFT = 6
    BOTTOM = 7
    BOTTOM_RIGHT = 8


class ColorScheme(enum.IntEnum):
    """C++ の tokotachi::ColorScheme"""

    SYSTEM = 0
    LIGHT = 1
    DARK = 2


class ColorRole(enum.IntEnum):
    """C++ の tokotachi::ColorRole"""

    WINDOW = 0
    POPUP = 1
    TEXT = 2
    TEXT_SECONDARY = 3
    TEXT_DISABLED = 4
    PLACEHOLDER = 5
    ACCENT = 6
    ACCENT_HOVER = 7
    ACCENT_PRESSED = 8
    ACCENT_DISABLED = 9
    ON_ACCENT = 10
    FOCUS_RING = 11
    FIELD = 12
    FIELD_DISABLED = 13
    BORDER = 14
    SELECTION = 15
    SELECTION_INACTIVE = 16
    ON_SELECTION = 17
    PREEDIT_UNDERLINE = 18
    SCROLL_TRACK = 19
    SCROLL_THUMB = 20
    SCROLL_THUMB_ACTIVE = 21
    TOOLTIP = 22
    ON_TOOLTIP = 23


class ButtonStyle(enum.IntEnum):
    """C++ の tokotachi::ButtonStyle"""

    PRIMARY = 0
    SECONDARY = 1


class DialogButton(enum.IntEnum):
    """C++ の tokotachi::DialogButton"""

    NONE = 0
    OK = 1
    YES = 2
    SAVE = 3
    NO = 4
    DISCARD = 5
    CANCEL = 6
    CLOSE = 7


class FileDialogMode(enum.IntEnum):
    """C++ の tokotachi::FileDialogMode"""

    OPEN = 0
    SAVE = 1
    FOLDER = 2


class MessageType(enum.IntEnum):
    """C++ の tokotachi::MessageType"""

    INFORMATION = 0
    QUESTION = 1
    WARNING = 2
    CRITICAL = 3


class PopupKind(enum.IntEnum):
    """C++ の tokotachi::PopupKind"""

    MENU = 0
    TOOLTIP = 1


class PopupPlacement(enum.IntEnum):
    """C++ の tokotachi::PopupPlacement"""

    BELOW = 0
    RIGHT = 1
    AT = 2


class ToolButtonStyle(enum.IntEnum):
    """C++ の tokotachi::ToolButtonStyle"""

    ICON_ONLY = 0
    TEXT_BESIDE_ICON = 1
    TEXT_UNDER_ICON = 2


class SelectionMode(enum.IntEnum):
    """C++ の tokotachi::SelectionMode"""

    NONE = 0
    SINGLE = 1
    MULTI = 2


class SortOrder(enum.IntEnum):
    """C++ の tokotachi::SortOrder"""

    ASCENDING = 0
    DESCENDING = 1


class LineCap(enum.IntEnum):
    """C++ の tokotachi::LineCap"""

    BUTT = 0
    ROUND = 1
    SQUARE = 2


class LineJoin(enum.IntEnum):
    """C++ の tokotachi::LineJoin"""

    MITER = 0
    ROUND = 1
    BEVEL = 2


class PaintType(enum.IntEnum):
    """塗りの種類(C++ の Paint::Type)"""

    SOLID = 0
    LINEAR = 1
    RADIAL = 2


class DayOfWeek(enum.IntEnum):
    """C++ の tokotachi::DayOfWeek"""

    SUNDAY = 0
    MONDAY = 1
    TUESDAY = 2
    WEDNESDAY = 3
    THURSDAY = 4
    FRIDAY = 5
    SATURDAY = 6


class DropAction(enum.IntEnum):
    """C++ の tokotachi::DropAction"""

    NONE = 0
    COPY = 1
    MOVE = 2


class SelectionBehavior(enum.IntEnum):
    """C++ の tokotachi::SelectionBehavior"""

    CELLS = 0
    ROWS = 1


class ImageFit(enum.IntEnum):
    """C++ の tokotachi::ImageFit"""

    NONE = 0
    CONTAIN = 1
    COVER = 2
    STRETCH = 3


class Icon(enum.IntEnum):
    """よく使うアイコン(C++ の tokotachi::Icon。docs/adr/0054)"""

    FILE = 0
    FILE_PLUS = 1
    FILE_TEXT = 2
    FOLDER = 3
    FOLDER_OPEN = 4
    SAVE = 5
    PRINTER = 6
    DOWNLOAD = 7
    UPLOAD = 8
    UNDO_2 = 9
    REDO_2 = 10
    SCISSORS = 11
    COPY = 12
    CLIPBOARD_PASTE = 13
    TRASH = 14
    PENCIL = 15
    PLUS = 16
    MINUS = 17
    X = 18
    CHECK = 19
    ARROW_LEFT = 20
    ARROW_RIGHT = 21
    ARROW_UP = 22
    ARROW_DOWN = 23
    CHEVRON_LEFT = 24
    CHEVRON_RIGHT = 25
    CHEVRON_UP = 26
    CHEVRON_DOWN = 27
    HOUSE = 28
    MENU = 29
    ELLIPSIS = 30
    REFRESH_CW = 31
    EXTERNAL_LINK = 32
    SEARCH = 33
    ZOOM_IN = 34
    ZOOM_OUT = 35
    EYE = 36
    EYE_OFF = 37
    FUNNEL = 38
    SETTINGS = 39
    SLIDERS_HORIZONTAL = 40
    LIST = 41
    LAYOUT_GRID = 42
    INFO = 43
    TRIANGLE_ALERT = 44
    CIRCLE_X = 45
    CIRCLE_CHECK = 46
    CIRCLE_QUESTION_MARK = 47
    BELL = 48
    PLAY = 49
    PAUSE = 50
    SQUARE = 51
    SKIP_BACK = 52
    SKIP_FORWARD = 53
    VOLUME_2 = 54
    VOLUME_X = 55
    USER = 56
    USERS = 57
    MAIL = 58
    CALENDAR = 59
    CLOCK = 60
    LOCK = 61
    LOCK_OPEN = 62
    STAR = 63
    HEART = 64
    LINK = 65
    IMAGE = 66
    LOG_IN = 67
    LOG_OUT = 68
    SUN = 69
    MOON = 70


class VariableType(enum.IntEnum):
    """Variable の値の型(bool・int・float・double・std::string)"""

    BOOL = 0
    INT = 1
    FLOAT = 2
    DOUBLE = 3
    STRING = 4


# ---- 関数の宣言(ctypes) ----

_proto('tokotachi_version', c_char_p, [])
_proto('tokotachi_free', None, [c_void_p])
_proto('tokotachi_strv_free', None, [c_void_p])
_proto('tokotachi_last_error', c_char_p, [])
_proto('tokotachi_clear_error', None, [])
_proto('tokotachi_set_error_handler', None, [c_void_p, c_void_p])
_proto('tokotachi_struct_size', c_size_t, [c_char_p])
_proto('tokotachi_object_delete', None, [c_void_p])
_proto('tokotachi_object_delete_later', None, [c_void_p])
_proto('tokotachi_object_class_name', c_char_p, [c_void_p])
_proto('tokotachi_object_set_user_data', None, [c_void_p, c_void_p, c_void_p])
_proto('tokotachi_object_get_user_data', c_void_p, [c_void_p])
_proto('tokotachi_bind', c_bool, [c_void_p, c_char_p, c_void_p, c_char_p])
_proto('tokotachi_unbind', c_bool, [c_void_p, c_char_p, c_void_p, c_char_p])
_proto('tokotachi_unbind_all', c_bool, [c_void_p, c_char_p])
_proto('tokotachi_is_main_thread', c_bool, [])
_proto('tokotachi_post', c_bool, [c_void_p, c_void_p, c_void_p])
_proto('tokotachi_clipboard_text', c_void_p, [])
_proto('tokotachi_clipboard_set_text', None, [c_char_p])
_proto('tokotachi_clipboard_has_text', c_bool, [])
_proto('tokotachi_shortcut_parse', CShortcut, [c_char_p])
_proto('tokotachi_shortcut_to_string', c_void_p, [CShortcut])
_proto('tokotachi_theme_light', None, [POINTER(CTheme)])
_proto('tokotachi_theme_dark', None, [POINTER(CTheme)])
_proto('tokotachi_theme_color_resolve', CColor, [CThemeColor])
_proto('tokotachi_date_today', CDate, [])
_proto('tokotachi_date_parse', CDate, [c_char_p, c_char_p])
_proto('tokotachi_date_format', c_void_p, [CDate, c_char_p])
_proto('tokotachi_date_is_valid', c_bool, [CDate])
_proto('tokotachi_date_day_of_week', c_int, [CDate])
_proto('tokotachi_date_add_days', CDate, [CDate, c_int])
_proto('tokotachi_date_add_months', CDate, [CDate, c_int])
_proto('tokotachi_date_add_years', CDate, [CDate, c_int])
_proto('tokotachi_date_days_to', c_int, [CDate, CDate])
_proto('tokotachi_date_days_in_month', c_int, [c_int, c_int])
_proto('tokotachi_date_is_leap_year', c_bool, [c_int])
_proto('tokotachi_drag_data_new', c_void_p, [])
_proto('tokotachi_drag_data_free', None, [c_void_p])
_proto('tokotachi_drag_data_get_text', c_void_p, [c_void_p])
_proto('tokotachi_drag_data_set_text', None, [c_void_p, c_char_p])
_proto('tokotachi_drag_data_get_files', c_void_p, [c_void_p])
_proto('tokotachi_drag_data_set_files', None, [c_void_p, POINTER(c_char_p), c_size_t])
_proto('tokotachi_drag_data_get_format', c_void_p, [c_void_p, c_char_p])
_proto('tokotachi_drag_data_set_format', None, [c_void_p, c_char_p, c_char_p])
_proto('tokotachi_drag_data_format_names', c_void_p, [c_void_p])
_proto('tokotachi_drag_data_source', c_void_p, [c_void_p])
_proto('tokotachi_drag_data_from_outside', c_bool, [c_void_p])
_proto('tokotachi_draw_button_face', None, [c_void_p, CRect, CControlState, c_int])
_proto('tokotachi_draw_button_text_color', CColor, [CControlState, c_int])
_proto('tokotachi_draw_check_mark', None, [c_void_p, CRect, CControlState])
_proto('tokotachi_draw_radio_mark', None, [c_void_p, CRect, CControlState])
_proto('tokotachi_draw_focus_ring', None, [c_void_p, CRect])
_proto('tokotachi_draw_field_frame', None, [c_void_p, CRect, CControlState])
_proto('tokotachi_image_load', c_void_p, [c_char_p])
_proto('tokotachi_image_load_from_data', c_void_p, [c_void_p, c_size_t])
_proto('tokotachi_image_from_pixels', c_void_p, [c_int, c_int, POINTER(CColor), c_size_t])
_proto('tokotachi_image_icon', c_void_p, [c_int])
_proto('tokotachi_image_icon_colored', c_void_p, [c_int, CColor])
_proto('tokotachi_image_is_icon', c_bool, [c_void_p])
_proto('tokotachi_image_copy', c_void_p, [c_void_p])
_proto('tokotachi_image_free', None, [c_void_p])
_proto('tokotachi_image_width', c_int, [c_void_p])
_proto('tokotachi_image_height', c_int, [c_void_p])
_proto('tokotachi_image_pixel', CColor, [c_void_p, c_int, c_int])
_proto('tokotachi_image_equal', c_bool, [c_void_p, c_void_p])
_proto('tokotachi_bitmap_new', c_void_p, [c_int, c_int, c_float])
_proto('tokotachi_bitmap_from_image', c_void_p, [c_void_p, c_float])
_proto('tokotachi_bitmap_free', None, [c_void_p])
_proto('tokotachi_bitmap_width', c_int, [c_void_p])
_proto('tokotachi_bitmap_height', c_int, [c_void_p])
_proto('tokotachi_bitmap_scale', c_float, [c_void_p])
_proto('tokotachi_bitmap_canvas', c_void_p, [c_void_p])
_proto('tokotachi_bitmap_clear', None, [c_void_p, CColor])
_proto('tokotachi_bitmap_pixel', CColor, [c_void_p, c_int, c_int])
_proto('tokotachi_bitmap_set_pixel', None, [c_void_p, c_int, c_int, CColor])
_proto('tokotachi_bitmap_pixels', POINTER(c_uint32), [c_void_p])
_proto('tokotachi_bitmap_to_image', c_void_p, [c_void_p])
_proto('tokotachi_bitmap_save_png', c_bool, [c_void_p, c_char_p])
_proto('tokotachi_path_new', c_void_p, [])
_proto('tokotachi_path_free', None, [c_void_p])
_proto('tokotachi_path_move_to', None, [c_void_p, c_float, c_float])
_proto('tokotachi_path_line_to', None, [c_void_p, c_float, c_float])
_proto('tokotachi_path_quad_to', None, [c_void_p, c_float, c_float, c_float, c_float])
_proto('tokotachi_path_cubic_to', None, [c_void_p, c_float, c_float, c_float, c_float, c_float, c_float])
_proto('tokotachi_path_close', None, [c_void_p])
_proto('tokotachi_path_add_rect', None, [c_void_p, CRect])
_proto('tokotachi_path_add_round_rect', None, [c_void_p, CRect, c_float])
_proto('tokotachi_path_add_ellipse', None, [c_void_p, CPoint, c_float, c_float])
_proto('tokotachi_path_add_arc', None, [c_void_p, CPoint, c_float, c_float, c_float, c_float, c_bool])
_proto('tokotachi_path_add_polyline', None, [c_void_p, POINTER(CPoint), c_size_t, c_bool])
_proto('tokotachi_path_is_empty', c_bool, [c_void_p])
_proto('tokotachi_text_layout_new', c_void_p, [c_char_p, CFont, c_float, c_float, c_int])
_proto('tokotachi_text_layout_free', None, [c_void_p])
_proto('tokotachi_text_layout_width', c_float, [c_void_p])
_proto('tokotachi_text_layout_height', c_float, [c_void_p])
_proto('tokotachi_text_layout_ascent', c_float, [c_void_p])
_proto('tokotachi_text_layout_descent', c_float, [c_void_p])
_proto('tokotachi_text_layout_line_height', c_float, [c_void_p])
_proto('tokotachi_text_layout_line_count', c_size_t, [c_void_p])
_proto('tokotachi_text_layout_line_rect', CRect, [c_void_p, c_size_t])
_proto('tokotachi_text_layout_caret_position', CPoint, [c_void_p, c_size_t])
_proto('tokotachi_text_layout_offset_at', c_size_t, [c_void_p, CPoint])
_proto('tokotachi_canvas_scale', c_float, [c_void_p])
_proto('tokotachi_canvas_save', None, [c_void_p])
_proto('tokotachi_canvas_restore', None, [c_void_p])
_proto('tokotachi_canvas_translate', None, [c_void_p, c_float, c_float])
_proto('tokotachi_canvas_scale_by', None, [c_void_p, c_float, c_float])
_proto('tokotachi_canvas_rotate', None, [c_void_p, c_float])
_proto('tokotachi_canvas_clip_rect', None, [c_void_p, CRect])
_proto('tokotachi_canvas_clip_path', None, [c_void_p, c_void_p])
_proto('tokotachi_canvas_clip_bounds', CRect, [c_void_p])
_proto('tokotachi_canvas_begin_layer', None, [c_void_p, c_float])
_proto('tokotachi_canvas_end_layer', None, [c_void_p])
_proto('tokotachi_canvas_fill_rect', None, [c_void_p, CRect, CColor])
_proto('tokotachi_canvas_fill_round_rect', None, [c_void_p, CRect, c_float, POINTER(CPaint)])
_proto('tokotachi_canvas_stroke_round_rect', None, [c_void_p, CRect, c_float, c_float, CColor])
_proto('tokotachi_canvas_stroke_rect', None, [c_void_p, CRect, c_float, CColor])
_proto('tokotachi_canvas_fill_path', None, [c_void_p, c_void_p, POINTER(CPaint)])
_proto('tokotachi_canvas_stroke_path', None, [c_void_p, c_void_p, POINTER(CStrokeStyle), POINTER(CPaint)])
_proto('tokotachi_canvas_draw_line', None, [c_void_p, CPoint, CPoint, POINTER(CStrokeStyle), POINTER(CPaint)])
_proto('tokotachi_canvas_fill_ellipse', None, [c_void_p, CRect, POINTER(CPaint)])
_proto('tokotachi_canvas_stroke_ellipse', None, [c_void_p, CRect, POINTER(CStrokeStyle), POINTER(CPaint)])
_proto('tokotachi_canvas_draw_text', None, [c_void_p, c_char_p, CPoint, CColor, CFont])
_proto('tokotachi_canvas_draw_text_layout', None, [c_void_p, c_void_p, CPoint, CColor])
_proto('tokotachi_canvas_draw_image', None, [c_void_p, c_void_p, CRect, c_float])
_proto('tokotachi_object_get_owner', c_void_p, [c_void_p])
_proto('tokotachi_application_new', c_void_p, [c_int, c_void_p])
_proto('tokotachi_application_get_color_scheme', c_int, [c_void_p])
_proto('tokotachi_application_set_color_scheme', None, [c_void_p, c_int])
_proto('tokotachi_application_get_light_theme', None, [c_void_p, POINTER(CTheme)])
_proto('tokotachi_application_set_light_theme', None, [c_void_p, POINTER(CTheme)])
_proto('tokotachi_application_get_dark_theme', None, [c_void_p, POINTER(CTheme)])
_proto('tokotachi_application_set_dark_theme', None, [c_void_p, POINTER(CTheme)])
_proto('tokotachi_application_get_dark', c_bool, [c_void_p])
_proto('tokotachi_application_get_theme', None, [c_void_p, POINTER(CTheme)])
_proto('tokotachi_application_get_language', c_void_p, [c_void_p])
_proto('tokotachi_application_set_language', None, [c_void_p, c_char_p])
_proto('tokotachi_application_get_main_window', c_void_p, [c_void_p])
_proto('tokotachi_application_set_main_window', None, [c_void_p, c_void_p])
_proto('tokotachi_application_get_auto_quit_enabled', c_bool, [c_void_p])
_proto('tokotachi_application_set_auto_quit_enabled', None, [c_void_p, c_bool])
_proto('tokotachi_application_set_on_theme_change', None, [c_void_p, c_void_p, c_void_p, c_void_p])
_proto('tokotachi_application_exec', c_int, [c_void_p])
_proto('tokotachi_application_quit', None, [c_void_p, c_int])
_proto('tokotachi_application_instance', c_void_p, [])
_proto('tokotachi_timer_new', c_void_p, [c_void_p])
_proto('tokotachi_timer_get_interval', c_int, [c_void_p])
_proto('tokotachi_timer_set_interval', None, [c_void_p, c_int])
_proto('tokotachi_timer_get_repeat', c_bool, [c_void_p])
_proto('tokotachi_timer_set_repeat', None, [c_void_p, c_bool])
_proto('tokotachi_timer_get_enabled', c_bool, [c_void_p])
_proto('tokotachi_timer_set_enabled', None, [c_void_p, c_bool])
_proto('tokotachi_timer_set_on_timeout', None, [c_void_p, c_void_p, c_void_p, c_void_p])
_proto('tokotachi_widget_new', c_void_p, [c_void_p])
_proto('tokotachi_widget_get_parent', c_void_p, [c_void_p])
_proto('tokotachi_widget_set_parent', None, [c_void_p, c_void_p])
_proto('tokotachi_widget_get_children', c_size_t, [c_void_p, POINTER(c_void_p), c_size_t])
_proto('tokotachi_widget_get_window', c_void_p, [c_void_p])
_proto('tokotachi_widget_get_x', c_float, [c_void_p])
_proto('tokotachi_widget_set_x', None, [c_void_p, c_float])
_proto('tokotachi_widget_get_y', c_float, [c_void_p])
_proto('tokotachi_widget_set_y', None, [c_void_p, c_float])
_proto('tokotachi_widget_get_width', c_float, [c_void_p])
_proto('tokotachi_widget_set_width', None, [c_void_p, c_float])
_proto('tokotachi_widget_get_height', c_float, [c_void_p])
_proto('tokotachi_widget_set_height', None, [c_void_p, c_float])
_proto('tokotachi_widget_get_bounds', CRect, [c_void_p])
_proto('tokotachi_widget_set_bounds', None, [c_void_p, CRect])
_proto('tokotachi_widget_get_local_rect', CRect, [c_void_p])
_proto('tokotachi_widget_get_preferred_size', CSize, [c_void_p])
_proto('tokotachi_widget_get_minimum_size', CSize, [c_void_p])
_proto('tokotachi_widget_set_minimum_size', None, [c_void_p, CSize])
_proto('tokotachi_widget_get_maximum_size', CSize, [c_void_p])
_proto('tokotachi_widget_set_maximum_size', None, [c_void_p, CSize])
_proto('tokotachi_widget_get_minimum_hint', CSize, [c_void_p])
_proto('tokotachi_widget_get_layout', c_void_p, [c_void_p])
_proto('tokotachi_widget_get_visible', c_bool, [c_void_p])
_proto('tokotachi_widget_set_visible', None, [c_void_p, c_bool])
_proto('tokotachi_widget_get_showing', c_bool, [c_void_p])
_proto('tokotachi_widget_get_enabled', c_bool, [c_void_p])
_proto('tokotachi_widget_set_enabled', None, [c_void_p, c_bool])
_proto('tokotachi_widget_get_effectively_enabled', c_bool, [c_void_p])
_proto('tokotachi_widget_get_focus_policy', c_int, [c_void_p])
_proto('tokotachi_widget_set_focus_policy', None, [c_void_p, c_int])
_proto('tokotachi_widget_get_focused', c_bool, [c_void_p])
_proto('tokotachi_widget_set_focused', None, [c_void_p, c_bool])
_proto('tokotachi_widget_get_cursor', c_int, [c_void_p])
_proto('tokotachi_widget_set_cursor', None, [c_void_p, c_int])
_proto('tokotachi_widget_get_accepts_text_input', c_bool, [c_void_p])
_proto('tokotachi_widget_set_accepts_text_input', None, [c_void_p, c_bool])
_proto('tokotachi_widget_get_display_scale', c_float, [c_void_p])
_proto('tokotachi_widget_get_tooltip', c_void_p, [c_void_p])
_proto('tokotachi_widget_set_tooltip', None, [c_void_p, c_char_p])
_proto('tokotachi_widget_get_actions', c_size_t, [c_void_p, POINTER(c_void_p), c_size_t])
_proto('tokotachi_widget_get_theme_override', None, [c_void_p, POINTER(CThemeOverride)])
_proto('tokotachi_widget_set_theme_override', None, [c_void_p, POINTER(CThemeOverride)])
_proto('tokotachi_widget_get_theme', None, [c_void_p, POINTER(CTheme)])
_proto('tokotachi_widget_get_accepts_drops', c_bool, [c_void_p])
_proto('tokotachi_widget_set_accepts_drops', None, [c_void_p, c_bool])
_proto('tokotachi_widget_add_action', None, [c_void_p, c_void_p])
_proto('tokotachi_widget_remove_action', None, [c_void_p, c_void_p])
_proto('tokotachi_widget_invalidate', None, [c_void_p])
_proto('tokotachi_widget_invalidate_rect', None, [c_void_p, CRect])
_proto('tokotachi_widget_request_layout', None, [c_void_p])
_proto('tokotachi_widget_apply_layout', None, [c_void_p])
_proto('tokotachi_widget_map_to_window', CPoint, [c_void_p, CPoint])
_proto('tokotachi_widget_map_from_window', CPoint, [c_void_p, CPoint])
_proto('tokotachi_widget_child_at', c_void_p, [c_void_p, CPoint])
_proto('tokotachi_widget_start_drag', c_bool, [c_void_p, c_void_p, c_char_p])
_proto('tokotachi_widget_focus_next_after', c_bool, [c_void_p])
_proto('tokotachi_widget_focus_first_within', c_bool, [c_void_p])
_proto('tokotachi_widget_set_on_paint', None, [c_void_p, c_void_p, c_void_p, c_void_p])
_proto('tokotachi_widget_set_on_resize', None, [c_void_p, c_void_p, c_void_p, c_void_p])
_proto('tokotachi_widget_set_on_mouse_down', None, [c_void_p, c_void_p, c_void_p, c_void_p])
_proto('tokotachi_widget_set_on_mouse_up', None, [c_void_p, c_void_p, c_void_p, c_void_p])
_proto('tokotachi_widget_set_on_mouse_move', None, [c_void_p, c_void_p, c_void_p, c_void_p])
_proto('tokotachi_widget_set_on_mouse_enter', None, [c_void_p, c_void_p, c_void_p, c_void_p])
_proto('tokotachi_widget_set_on_mouse_leave', None, [c_void_p, c_void_p, c_void_p, c_void_p])
_proto('tokotachi_widget_set_on_mouse_wheel', None, [c_void_p, c_void_p, c_void_p, c_void_p])
_proto('tokotachi_widget_set_on_key_down', None, [c_void_p, c_void_p, c_void_p, c_void_p])
_proto('tokotachi_widget_set_on_key_up', None, [c_void_p, c_void_p, c_void_p, c_void_p])
_proto('tokotachi_widget_set_on_focus_in', None, [c_void_p, c_void_p, c_void_p, c_void_p])
_proto('tokotachi_widget_set_on_focus_out', None, [c_void_p, c_void_p, c_void_p, c_void_p])
_proto('tokotachi_widget_set_on_drag_move', None, [c_void_p, c_void_p, c_void_p, c_void_p])
_proto('tokotachi_widget_set_on_drag_leave', None, [c_void_p, c_void_p, c_void_p, c_void_p])
_proto('tokotachi_widget_set_on_drop', None, [c_void_p, c_void_p, c_void_p, c_void_p])
_proto('tokotachi_widget_set_on_drag_finish', None, [c_void_p, c_void_p, c_void_p, c_void_p])
_proto('tokotachi_button_base_get_text', c_void_p, [c_void_p])
_proto('tokotachi_button_base_set_text', None, [c_void_p, c_char_p])
_proto('tokotachi_button_base_get_font', CFont, [c_void_p])
_proto('tokotachi_button_base_set_font', None, [c_void_p, CFont])
_proto('tokotachi_button_base_set_on_click', None, [c_void_p, c_void_p, c_void_p, c_void_p])
_proto('tokotachi_button_new', c_void_p, [c_void_p, c_char_p])
_proto('tokotachi_button_get_style', c_int, [c_void_p])
_proto('tokotachi_button_set_style', None, [c_void_p, c_int])
_proto('tokotachi_button_get_icon', c_void_p, [c_void_p])
_proto('tokotachi_button_set_icon', None, [c_void_p, c_void_p])
_proto('tokotachi_button_get_icon_size', c_float, [c_void_p])
_proto('tokotachi_button_set_icon_size', None, [c_void_p, c_float])
_proto('tokotachi_check_box_new', c_void_p, [c_void_p, c_char_p])
_proto('tokotachi_check_box_get_checked', c_bool, [c_void_p])
_proto('tokotachi_check_box_set_checked', None, [c_void_p, c_bool])
_proto('tokotachi_check_box_set_on_change', None, [c_void_p, c_void_p, c_void_p, c_void_p])
_proto('tokotachi_radio_button_new', c_void_p, [c_void_p, c_char_p, c_int])
_proto('tokotachi_radio_button_get_value', c_int, [c_void_p])
_proto('tokotachi_radio_button_set_value', None, [c_void_p, c_int])
_proto('tokotachi_radio_button_get_group_value', c_int, [c_void_p])
_proto('tokotachi_radio_button_set_group_value', None, [c_void_p, c_int])
_proto('tokotachi_radio_button_get_checked', c_bool, [c_void_p])
_proto('tokotachi_radio_button_set_on_change', None, [c_void_p, c_void_p, c_void_p, c_void_p])
_proto('tokotachi_combo_box_new', c_void_p, [c_void_p])
_proto('tokotachi_combo_box_get_items', c_void_p, [c_void_p])
_proto('tokotachi_combo_box_set_items', None, [c_void_p, POINTER(c_char_p), c_size_t])
_proto('tokotachi_combo_box_get_current_index', c_int, [c_void_p])
_proto('tokotachi_combo_box_set_current_index', None, [c_void_p, c_int])
_proto('tokotachi_combo_box_get_current_text', c_void_p, [c_void_p])
_proto('tokotachi_combo_box_get_placeholder_text', c_void_p, [c_void_p])
_proto('tokotachi_combo_box_set_placeholder_text', None, [c_void_p, c_char_p])
_proto('tokotachi_combo_box_get_font', CFont, [c_void_p])
_proto('tokotachi_combo_box_set_font', None, [c_void_p, CFont])
_proto('tokotachi_combo_box_set_on_change', None, [c_void_p, c_void_p, c_void_p, c_void_p])
_proto('tokotachi_combo_box_open_popup', None, [c_void_p])
_proto('tokotachi_combo_box_close_popup', None, [c_void_p])
_proto('tokotachi_combo_box_get_popup_open', c_bool, [c_void_p])
_proto('tokotachi_label_new', c_void_p, [c_void_p, c_char_p])
_proto('tokotachi_label_get_text', c_void_p, [c_void_p])
_proto('tokotachi_label_set_text', None, [c_void_p, c_char_p])
_proto('tokotachi_label_get_font', CFont, [c_void_p])
_proto('tokotachi_label_set_font', None, [c_void_p, CFont])
_proto('tokotachi_label_get_text_color', CThemeColor, [c_void_p])
_proto('tokotachi_label_set_text_color', None, [c_void_p, CThemeColor])
_proto('tokotachi_label_get_alignment', c_int, [c_void_p])
_proto('tokotachi_label_set_alignment', None, [c_void_p, c_int])
_proto('tokotachi_label_get_word_wrap', c_bool, [c_void_p])
_proto('tokotachi_label_set_word_wrap', None, [c_void_p, c_bool])
_proto('tokotachi_label_get_icon', c_void_p, [c_void_p])
_proto('tokotachi_label_set_icon', None, [c_void_p, c_void_p])
_proto('tokotachi_label_get_icon_size', c_float, [c_void_p])
_proto('tokotachi_label_set_icon_size', None, [c_void_p, c_float])
_proto('tokotachi_label_get_buddy', c_void_p, [c_void_p])
_proto('tokotachi_label_set_buddy', None, [c_void_p, c_void_p])
_proto('tokotachi_text_input_base_get_text', c_void_p, [c_void_p])
_proto('tokotachi_text_input_base_set_text', None, [c_void_p, c_char_p])
_proto('tokotachi_text_input_base_get_placeholder_text', c_void_p, [c_void_p])
_proto('tokotachi_text_input_base_set_placeholder_text', None, [c_void_p, c_char_p])
_proto('tokotachi_text_input_base_get_font', CFont, [c_void_p])
_proto('tokotachi_text_input_base_set_font', None, [c_void_p, CFont])
_proto('tokotachi_text_input_base_get_cursor_position', c_size_t, [c_void_p])
_proto('tokotachi_text_input_base_set_cursor_position', None, [c_void_p, c_size_t])
_proto('tokotachi_text_input_base_get_has_selection', c_bool, [c_void_p])
_proto('tokotachi_text_input_base_get_selected_text', c_void_p, [c_void_p])
_proto('tokotachi_text_input_base_get_can_undo', c_bool, [c_void_p])
_proto('tokotachi_text_input_base_get_can_redo', c_bool, [c_void_p])
_proto('tokotachi_text_input_base_set_on_change', None, [c_void_p, c_void_p, c_void_p, c_void_p])
_proto('tokotachi_text_input_base_select_all', None, [c_void_p])
_proto('tokotachi_text_input_base_clear_selection', None, [c_void_p])
_proto('tokotachi_text_input_base_cut', None, [c_void_p])
_proto('tokotachi_text_input_base_copy', None, [c_void_p])
_proto('tokotachi_text_input_base_paste', None, [c_void_p])
_proto('tokotachi_text_input_base_insert', None, [c_void_p, c_char_p])
_proto('tokotachi_text_input_base_undo', None, [c_void_p])
_proto('tokotachi_text_input_base_redo', None, [c_void_p])
_proto('tokotachi_text_input_base_clear_undo_history', None, [c_void_p])
_proto('tokotachi_line_edit_new', c_void_p, [c_void_p, c_char_p])
_proto('tokotachi_line_edit_set_on_submit', None, [c_void_p, c_void_p, c_void_p, c_void_p])
_proto('tokotachi_text_edit_new', c_void_p, [c_void_p, c_char_p])
_proto('tokotachi_text_edit_get_word_wrap', c_bool, [c_void_p])
_proto('tokotachi_text_edit_set_word_wrap', None, [c_void_p, c_bool])
_proto('tokotachi_paint_box_new', c_void_p, [c_void_p])
_proto('tokotachi_paint_box_get_cached', c_bool, [c_void_p])
_proto('tokotachi_paint_box_set_cached', None, [c_void_p, c_bool])
_proto('tokotachi_paint_box_refresh', None, [c_void_p])
_proto('tokotachi_progress_bar_new', c_void_p, [c_void_p])
_proto('tokotachi_progress_bar_get_minimum', c_double, [c_void_p])
_proto('tokotachi_progress_bar_set_minimum', None, [c_void_p, c_double])
_proto('tokotachi_progress_bar_get_maximum', c_double, [c_void_p])
_proto('tokotachi_progress_bar_set_maximum', None, [c_void_p, c_double])
_proto('tokotachi_progress_bar_get_value', c_double, [c_void_p])
_proto('tokotachi_progress_bar_set_value', None, [c_void_p, c_double])
_proto('tokotachi_progress_bar_get_text_visible', c_bool, [c_void_p])
_proto('tokotachi_progress_bar_set_text_visible', None, [c_void_p, c_bool])
_proto('tokotachi_progress_bar_get_indeterminate', c_bool, [c_void_p])
_proto('tokotachi_progress_bar_set_indeterminate', None, [c_void_p, c_bool])
_proto('tokotachi_range_base_get_minimum', c_double, [c_void_p])
_proto('tokotachi_range_base_set_minimum', None, [c_void_p, c_double])
_proto('tokotachi_range_base_get_maximum', c_double, [c_void_p])
_proto('tokotachi_range_base_set_maximum', None, [c_void_p, c_double])
_proto('tokotachi_range_base_get_value', c_double, [c_void_p])
_proto('tokotachi_range_base_set_value', None, [c_void_p, c_double])
_proto('tokotachi_range_base_get_single_step', c_double, [c_void_p])
_proto('tokotachi_range_base_set_single_step', None, [c_void_p, c_double])
_proto('tokotachi_range_base_get_page_step', c_double, [c_void_p])
_proto('tokotachi_range_base_set_page_step', None, [c_void_p, c_double])
_proto('tokotachi_range_base_set_on_change', None, [c_void_p, c_void_p, c_void_p, c_void_p])
_proto('tokotachi_slider_new', c_void_p, [c_void_p, c_int])
_proto('tokotachi_slider_get_orientation', c_int, [c_void_p])
_proto('tokotachi_slider_set_orientation', None, [c_void_p, c_int])
_proto('tokotachi_slider_get_tick_interval', c_double, [c_void_p])
_proto('tokotachi_slider_set_tick_interval', None, [c_void_p, c_double])
_proto('tokotachi_spin_box_new', c_void_p, [c_void_p])
_proto('tokotachi_spin_box_get_decimals', c_int, [c_void_p])
_proto('tokotachi_spin_box_set_decimals', None, [c_void_p, c_int])
_proto('tokotachi_spin_box_get_prefix', c_void_p, [c_void_p])
_proto('tokotachi_spin_box_set_prefix', None, [c_void_p, c_char_p])
_proto('tokotachi_spin_box_get_suffix', c_void_p, [c_void_p])
_proto('tokotachi_spin_box_set_suffix', None, [c_void_p, c_char_p])
_proto('tokotachi_spin_box_get_text', c_void_p, [c_void_p])
_proto('tokotachi_spin_box_commit', None, [c_void_p])
_proto('tokotachi_scroll_area_new', c_void_p, [c_void_p])
_proto('tokotachi_scroll_area_get_content', c_void_p, [c_void_p])
_proto('tokotachi_scroll_area_set_content', None, [c_void_p, c_void_p])
_proto('tokotachi_scroll_area_get_fit_width', c_bool, [c_void_p])
_proto('tokotachi_scroll_area_set_fit_width', None, [c_void_p, c_bool])
_proto('tokotachi_scroll_area_get_scroll_position', CPoint, [c_void_p])
_proto('tokotachi_scroll_area_set_scroll_position', None, [c_void_p, CPoint])
_proto('tokotachi_scroll_area_scroll_to_visible', None, [c_void_p, CRect])
_proto('tokotachi_scroll_bar_new', c_void_p, [c_void_p, c_int])
_proto('tokotachi_scroll_bar_get_orientation', c_int, [c_void_p])
_proto('tokotachi_scroll_bar_set_orientation', None, [c_void_p, c_int])
_proto('tokotachi_scroll_bar_get_value', c_float, [c_void_p])
_proto('tokotachi_scroll_bar_set_value', None, [c_void_p, c_float])
_proto('tokotachi_scroll_bar_get_maximum', c_float, [c_void_p])
_proto('tokotachi_scroll_bar_set_maximum', None, [c_void_p, c_float])
_proto('tokotachi_scroll_bar_get_page_step', c_float, [c_void_p])
_proto('tokotachi_scroll_bar_set_page_step', None, [c_void_p, c_float])
_proto('tokotachi_scroll_bar_get_single_step', c_float, [c_void_p])
_proto('tokotachi_scroll_bar_set_single_step', None, [c_void_p, c_float])
_proto('tokotachi_scroll_bar_set_on_change', None, [c_void_p, c_void_p, c_void_p, c_void_p])
_proto('tokotachi_splitter_pane_get_stretch', c_int, [c_void_p])
_proto('tokotachi_splitter_pane_set_stretch', None, [c_void_p, c_int])
_proto('tokotachi_splitter_pane_get_collapsible', c_bool, [c_void_p])
_proto('tokotachi_splitter_pane_set_collapsible', None, [c_void_p, c_bool])
_proto('tokotachi_splitter_pane_get_collapsed', c_bool, [c_void_p])
_proto('tokotachi_splitter_pane_get_widget', c_void_p, [c_void_p])
_proto('tokotachi_splitter_new', c_void_p, [c_void_p, c_int])
_proto('tokotachi_splitter_get_orientation', c_int, [c_void_p])
_proto('tokotachi_splitter_set_orientation', None, [c_void_p, c_int])
_proto('tokotachi_splitter_get_panes', c_size_t, [c_void_p, POINTER(c_void_p), c_size_t])
_proto('tokotachi_splitter_get_sizes', c_size_t, [c_void_p, POINTER(c_float), c_size_t])
_proto('tokotachi_splitter_set_sizes', None, [c_void_p, POINTER(c_float), c_size_t])
_proto('tokotachi_splitter_set_on_change', None, [c_void_p, c_void_p, c_void_p, c_void_p])
_proto('tokotachi_splitter_add_widget', c_void_p, [c_void_p, c_void_p, c_int])
_proto('tokotachi_splitter_insert_widget', c_void_p, [c_void_p, c_int, c_void_p, c_int])
_proto('tokotachi_tab_get_title', c_void_p, [c_void_p])
_proto('tokotachi_tab_set_title', None, [c_void_p, c_char_p])
_proto('tokotachi_tab_get_enabled', c_bool, [c_void_p])
_proto('tokotachi_tab_set_enabled', None, [c_void_p, c_bool])
_proto('tokotachi_tab_get_page', c_void_p, [c_void_p])
_proto('tokotachi_tab_widget_new', c_void_p, [c_void_p])
_proto('tokotachi_tab_widget_get_tabs', c_size_t, [c_void_p, POINTER(c_void_p), c_size_t])
_proto('tokotachi_tab_widget_get_current_index', c_int, [c_void_p])
_proto('tokotachi_tab_widget_set_current_index', None, [c_void_p, c_int])
_proto('tokotachi_tab_widget_get_current_tab', c_void_p, [c_void_p])
_proto('tokotachi_tab_widget_set_on_change', None, [c_void_p, c_void_p, c_void_p, c_void_p])
_proto('tokotachi_tab_widget_add_tab', c_void_p, [c_void_p, c_void_p, c_char_p])
_proto('tokotachi_tab_widget_insert_tab', c_void_p, [c_void_p, c_int, c_void_p, c_char_p])
_proto('tokotachi_tab_widget_remove_tab', c_void_p, [c_void_p, c_void_p])
_proto('tokotachi_group_box_new', c_void_p, [c_void_p, c_char_p])
_proto('tokotachi_group_box_get_title', c_void_p, [c_void_p])
_proto('tokotachi_group_box_set_title', None, [c_void_p, c_char_p])
_proto('tokotachi_group_box_get_content_rect', CRect, [c_void_p])
_proto('tokotachi_image_box_new', c_void_p, [c_void_p, c_void_p])
_proto('tokotachi_image_box_get_image', c_void_p, [c_void_p])
_proto('tokotachi_image_box_set_image', None, [c_void_p, c_void_p])
_proto('tokotachi_image_box_get_fit', c_int, [c_void_p])
_proto('tokotachi_image_box_set_fit', None, [c_void_p, c_int])
_proto('tokotachi_calendar_new', c_void_p, [c_void_p])
_proto('tokotachi_calendar_get_date', CDate, [c_void_p])
_proto('tokotachi_calendar_set_date', None, [c_void_p, CDate])
_proto('tokotachi_calendar_get_minimum', CDate, [c_void_p])
_proto('tokotachi_calendar_set_minimum', None, [c_void_p, CDate])
_proto('tokotachi_calendar_get_maximum', CDate, [c_void_p])
_proto('tokotachi_calendar_set_maximum', None, [c_void_p, CDate])
_proto('tokotachi_calendar_get_first_day_of_week', c_int, [c_void_p])
_proto('tokotachi_calendar_set_first_day_of_week', None, [c_void_p, c_int])
_proto('tokotachi_calendar_get_shown_year', c_int, [c_void_p])
_proto('tokotachi_calendar_get_shown_month', c_int, [c_void_p])
_proto('tokotachi_calendar_set_on_change', None, [c_void_p, c_void_p, c_void_p, c_void_p])
_proto('tokotachi_calendar_set_on_activate', None, [c_void_p, c_void_p, c_void_p, c_void_p])
_proto('tokotachi_calendar_show_month', None, [c_void_p, c_int, c_int])
_proto('tokotachi_date_edit_new', c_void_p, [c_void_p])
_proto('tokotachi_date_edit_get_date', CDate, [c_void_p])
_proto('tokotachi_date_edit_set_date', None, [c_void_p, CDate])
_proto('tokotachi_date_edit_get_minimum', CDate, [c_void_p])
_proto('tokotachi_date_edit_set_minimum', None, [c_void_p, CDate])
_proto('tokotachi_date_edit_get_maximum', CDate, [c_void_p])
_proto('tokotachi_date_edit_set_maximum', None, [c_void_p, CDate])
_proto('tokotachi_date_edit_get_format', c_void_p, [c_void_p])
_proto('tokotachi_date_edit_set_format', None, [c_void_p, c_char_p])
_proto('tokotachi_date_edit_get_first_day_of_week', c_int, [c_void_p])
_proto('tokotachi_date_edit_set_first_day_of_week', None, [c_void_p, c_int])
_proto('tokotachi_date_edit_get_placeholder_text', c_void_p, [c_void_p])
_proto('tokotachi_date_edit_set_placeholder_text', None, [c_void_p, c_char_p])
_proto('tokotachi_date_edit_get_text', c_void_p, [c_void_p])
_proto('tokotachi_date_edit_get_calendar_open', c_bool, [c_void_p])
_proto('tokotachi_date_edit_set_on_change', None, [c_void_p, c_void_p, c_void_p, c_void_p])
_proto('tokotachi_date_edit_commit', None, [c_void_p])
_proto('tokotachi_date_edit_open_calendar', None, [c_void_p])
_proto('tokotachi_date_edit_close_calendar', None, [c_void_p])
_proto('tokotachi_color_button_new', c_void_p, [c_void_p, CColor])
_proto('tokotachi_color_button_get_color', CColor, [c_void_p])
_proto('tokotachi_color_button_set_color', None, [c_void_p, CColor])
_proto('tokotachi_color_button_get_alpha_enabled', c_bool, [c_void_p])
_proto('tokotachi_color_button_set_alpha_enabled', None, [c_void_p, c_bool])
_proto('tokotachi_color_button_get_dialog_title', c_void_p, [c_void_p])
_proto('tokotachi_color_button_set_dialog_title', None, [c_void_p, c_char_p])
_proto('tokotachi_color_button_set_on_change', None, [c_void_p, c_void_p, c_void_p, c_void_p])
_proto('tokotachi_color_button_open_dialog', None, [c_void_p])
_proto('tokotachi_window_new', c_void_p, [c_char_p, c_float, c_float])
_proto('tokotachi_window_get_title', c_void_p, [c_void_p])
_proto('tokotachi_window_set_title', None, [c_void_p, c_char_p])
_proto('tokotachi_window_get_background', CThemeColor, [c_void_p])
_proto('tokotachi_window_set_background', None, [c_void_p, CThemeColor])
_proto('tokotachi_window_get_focused_widget', c_void_p, [c_void_p])
_proto('tokotachi_window_get_menu_bar', c_void_p, [c_void_p])
_proto('tokotachi_window_set_menu_bar', None, [c_void_p, c_void_p])
_proto('tokotachi_window_get_tool_bar', c_void_p, [c_void_p])
_proto('tokotachi_window_set_tool_bar', None, [c_void_p, c_void_p])
_proto('tokotachi_window_get_status_bar', c_void_p, [c_void_p])
_proto('tokotachi_window_set_status_bar', None, [c_void_p, c_void_p])
_proto('tokotachi_window_get_destroy_on_close', c_bool, [c_void_p])
_proto('tokotachi_window_set_destroy_on_close', None, [c_void_p, c_bool])
_proto('tokotachi_window_set_on_close', None, [c_void_p, c_void_p, c_void_p, c_void_p])
_proto('tokotachi_window_close', None, [c_void_p])
_proto('tokotachi_window_new_owned', c_void_p, [c_void_p, c_char_p, c_float, c_float])
_proto('tokotachi_window_save_png', c_bool, [c_void_p, c_char_p])
_proto('tokotachi_window_capture', c_void_p, [c_void_p])
_proto('tokotachi_dialog_new', c_void_p, [c_void_p, c_char_p])
_proto('tokotachi_dialog_get_result', c_int, [c_void_p])
_proto('tokotachi_dialog_set_on_finish', None, [c_void_p, c_void_p, c_void_p, c_void_p])
_proto('tokotachi_dialog_add_button', c_void_p, [c_void_p, c_int, c_char_p])
_proto('tokotachi_dialog_open', None, [c_void_p])
_proto('tokotachi_dialog_finish', None, [c_void_p, c_int])
_proto('tokotachi_message_dialog_new', c_void_p, [c_void_p, c_char_p, c_char_p, c_int])
_proto('tokotachi_message_dialog_get_text', c_void_p, [c_void_p])
_proto('tokotachi_message_dialog_set_text', None, [c_void_p, c_char_p])
_proto('tokotachi_message_dialog_get_type', c_int, [c_void_p])
_proto('tokotachi_message_dialog_set_type', None, [c_void_p, c_int])
_proto('tokotachi_message_dialog_show', c_void_p, [c_void_p, c_char_p, c_char_p, POINTER(c_int), c_size_t, c_void_p, c_void_p, c_void_p, c_int])
_proto('tokotachi_color_dialog_new', c_void_p, [c_void_p, c_char_p])
_proto('tokotachi_color_dialog_get_color', CColor, [c_void_p])
_proto('tokotachi_color_dialog_set_color', None, [c_void_p, CColor])
_proto('tokotachi_color_dialog_get_alpha_enabled', c_bool, [c_void_p])
_proto('tokotachi_color_dialog_set_alpha_enabled', None, [c_void_p, c_bool])
_proto('tokotachi_color_dialog_show', c_void_p, [c_void_p, CColor, c_void_p, c_void_p, c_void_p, c_bool, c_char_p])
_proto('tokotachi_file_dialog_new', c_void_p, [c_void_p, c_int])
_proto('tokotachi_file_dialog_get_mode', c_int, [c_void_p])
_proto('tokotachi_file_dialog_set_mode', None, [c_void_p, c_int])
_proto('tokotachi_file_dialog_get_title', c_void_p, [c_void_p])
_proto('tokotachi_file_dialog_set_title', None, [c_void_p, c_char_p])
_proto('tokotachi_file_dialog_get_filters', c_void_p, [c_void_p])
_proto('tokotachi_file_dialog_set_filters', None, [c_void_p, POINTER(c_char_p), c_size_t])
_proto('tokotachi_file_dialog_get_location', c_void_p, [c_void_p])
_proto('tokotachi_file_dialog_set_location', None, [c_void_p, c_char_p])
_proto('tokotachi_file_dialog_get_multi_enabled', c_bool, [c_void_p])
_proto('tokotachi_file_dialog_set_multi_enabled', None, [c_void_p, c_bool])
_proto('tokotachi_file_dialog_get_paths', c_void_p, [c_void_p])
_proto('tokotachi_file_dialog_get_error', c_void_p, [c_void_p])
_proto('tokotachi_file_dialog_get_selected_filter', c_int, [c_void_p])
_proto('tokotachi_file_dialog_get_running', c_bool, [c_void_p])
_proto('tokotachi_file_dialog_set_on_finish', None, [c_void_p, c_void_p, c_void_p, c_void_p])
_proto('tokotachi_file_dialog_open', None, [c_void_p])
_proto('tokotachi_menu_bar_new', c_void_p, [c_void_p])
_proto('tokotachi_menu_bar_get_menus', c_size_t, [c_void_p, POINTER(c_void_p), c_size_t])
_proto('tokotachi_menu_bar_add_menu', c_void_p, [c_void_p, c_char_p])
_proto('tokotachi_menu_bar_remove_menu', None, [c_void_p, c_void_p])
_proto('tokotachi_popup_new', c_void_p, [c_void_p, c_int, c_void_p])
_proto('tokotachi_popup_get_owner_widget', c_void_p, [c_void_p])
_proto('tokotachi_popup_get_kind', c_int, [c_void_p])
_proto('tokotachi_popup_set_on_closed', None, [c_void_p, c_void_p, c_void_p, c_void_p])
_proto('tokotachi_popup_open', None, [c_void_p, CRect, CSize, c_int])
_proto('tokotachi_status_bar_new', c_void_p, [c_void_p])
_proto('tokotachi_status_bar_get_text', c_void_p, [c_void_p])
_proto('tokotachi_status_bar_set_text', None, [c_void_p, c_char_p])
_proto('tokotachi_status_bar_get_message', c_void_p, [c_void_p])
_proto('tokotachi_status_bar_show_message', None, [c_void_p, c_char_p, c_int])
_proto('tokotachi_status_bar_clear_message', None, [c_void_p])
_proto('tokotachi_status_bar_add_widget', None, [c_void_p, c_void_p])
_proto('tokotachi_tool_bar_new', c_void_p, [c_void_p])
_proto('tokotachi_tool_bar_get_button_style', c_int, [c_void_p])
_proto('tokotachi_tool_bar_set_button_style', None, [c_void_p, c_int])
_proto('tokotachi_tool_bar_add_item', c_void_p, [c_void_p, c_char_p])
_proto('tokotachi_tool_bar_add_separator', None, [c_void_p])
_proto('tokotachi_tool_bar_add_widget', None, [c_void_p, c_void_p])
_proto('tokotachi_layout_get_widget', c_void_p, [c_void_p])
_proto('tokotachi_layout_get_margins', CMargins, [c_void_p])
_proto('tokotachi_layout_set_margins', None, [c_void_p, CMargins])
_proto('tokotachi_layout_remove', None, [c_void_p, c_void_p])
_proto('tokotachi_box_layout_new', c_void_p, [c_void_p, c_int])
_proto('tokotachi_box_layout_get_orientation', c_int, [c_void_p])
_proto('tokotachi_box_layout_set_orientation', None, [c_void_p, c_int])
_proto('tokotachi_box_layout_get_spacing', c_float, [c_void_p])
_proto('tokotachi_box_layout_set_spacing', None, [c_void_p, c_float])
_proto('tokotachi_box_layout_add', None, [c_void_p, c_void_p, c_int, c_int])
_proto('tokotachi_box_layout_add_spacing', None, [c_void_p, c_float])
_proto('tokotachi_box_layout_add_stretch', None, [c_void_p, c_int])
_proto('tokotachi_box_layout_set_stretch', None, [c_void_p, c_void_p, c_int])
_proto('tokotachi_box_layout_set_alignment', None, [c_void_p, c_void_p, c_int])
_proto('tokotachi_grid_layout_new', c_void_p, [c_void_p])
_proto('tokotachi_grid_layout_get_row_spacing', c_float, [c_void_p])
_proto('tokotachi_grid_layout_set_row_spacing', None, [c_void_p, c_float])
_proto('tokotachi_grid_layout_get_column_spacing', c_float, [c_void_p])
_proto('tokotachi_grid_layout_set_column_spacing', None, [c_void_p, c_float])
_proto('tokotachi_grid_layout_add', None, [c_void_p, c_void_p, c_int, c_int, c_int, c_int, c_int, c_int])
_proto('tokotachi_grid_layout_set_row_stretch', None, [c_void_p, c_int, c_int])
_proto('tokotachi_grid_layout_set_column_stretch', None, [c_void_p, c_int, c_int])
_proto('tokotachi_grid_layout_set_row_minimum', None, [c_void_p, c_int, c_float])
_proto('tokotachi_grid_layout_set_column_minimum', None, [c_void_p, c_int, c_float])
_proto('tokotachi_place_layout_new', c_void_p, [c_void_p])
_proto('tokotachi_place_layout_add', None, [c_void_p, c_void_p, CPlacement])
_proto('tokotachi_action_new', c_void_p, [c_void_p, c_char_p])
_proto('tokotachi_action_get_text', c_void_p, [c_void_p])
_proto('tokotachi_action_set_text', None, [c_void_p, c_char_p])
_proto('tokotachi_action_get_shortcut', CShortcut, [c_void_p])
_proto('tokotachi_action_set_shortcut', None, [c_void_p, CShortcut])
_proto('tokotachi_action_get_icon', c_void_p, [c_void_p])
_proto('tokotachi_action_set_icon', None, [c_void_p, c_void_p])
_proto('tokotachi_action_get_tooltip', c_void_p, [c_void_p])
_proto('tokotachi_action_set_tooltip', None, [c_void_p, c_char_p])
_proto('tokotachi_action_get_enabled', c_bool, [c_void_p])
_proto('tokotachi_action_set_enabled', None, [c_void_p, c_bool])
_proto('tokotachi_action_get_checkable', c_bool, [c_void_p])
_proto('tokotachi_action_set_checkable', None, [c_void_p, c_bool])
_proto('tokotachi_action_get_checked', c_bool, [c_void_p])
_proto('tokotachi_action_set_checked', None, [c_void_p, c_bool])
_proto('tokotachi_action_set_on_trigger', None, [c_void_p, c_void_p, c_void_p, c_void_p])
_proto('tokotachi_action_trigger', None, [c_void_p])
_proto('tokotachi_menu_new', c_void_p, [c_void_p, c_char_p])
_proto('tokotachi_menu_get_title', c_void_p, [c_void_p])
_proto('tokotachi_menu_set_title', None, [c_void_p, c_char_p])
_proto('tokotachi_menu_get_actions', c_size_t, [c_void_p, POINTER(c_void_p), c_size_t])
_proto('tokotachi_menu_add_item', c_void_p, [c_void_p, c_char_p])
_proto('tokotachi_menu_add_action', None, [c_void_p, c_void_p])
_proto('tokotachi_menu_remove_action', None, [c_void_p, c_void_p])
_proto('tokotachi_menu_add_separator', None, [c_void_p])
_proto('tokotachi_menu_add_submenu', c_void_p, [c_void_p, c_char_p])
_proto('tokotachi_menu_popup', None, [c_void_p, c_void_p, CPoint])
_proto('tokotachi_menu_close', None, [c_void_p])
_proto('tokotachi_menu_get_opened', c_bool, [c_void_p])
_proto('tokotachi_item_model_row_count', c_int, [c_void_p, CModelIndex])
_proto('tokotachi_item_model_index', CModelIndex, [c_void_p, c_int, CModelIndex])
_proto('tokotachi_item_model_parent_of', CModelIndex, [c_void_p, CModelIndex])
_proto('tokotachi_item_model_text', c_void_p, [c_void_p, CModelIndex, c_int])
_proto('tokotachi_item_model_icon', c_void_p, [c_void_p, CModelIndex])
_proto('tokotachi_item_model_enabled', c_bool, [c_void_p, CModelIndex])
_proto('tokotachi_item_model_checkable', c_bool, [c_void_p, CModelIndex])
_proto('tokotachi_item_model_checked', c_bool, [c_void_p, CModelIndex])
_proto('tokotachi_item_model_set_checked', None, [c_void_p, CModelIndex, c_bool])
_proto('tokotachi_item_model_text_color', CThemeColor, [c_void_p, CModelIndex, c_int])
_proto('tokotachi_item_model_font', CFont, [c_void_p, CModelIndex, c_int])
_proto('tokotachi_item_model_editable', c_bool, [c_void_p, CModelIndex, c_int])
_proto('tokotachi_item_model_set_text', None, [c_void_p, CModelIndex, c_int, c_char_p])
_proto('tokotachi_item_model_can_move_rows', c_bool, [c_void_p, POINTER(CModelIndex), c_size_t, CModelIndex, c_int])
_proto('tokotachi_item_model_move_rows', c_bool, [c_void_p, POINTER(CModelIndex), c_size_t, CModelIndex, c_int])
_proto('tokotachi_item_model_can_drop', c_bool, [c_void_p, c_void_p, CModelIndex, c_int])
_proto('tokotachi_item_model_drop_data', c_bool, [c_void_p, c_void_p, c_int, CModelIndex, c_int])
_proto('tokotachi_item_model_sort', None, [c_void_p, c_int, c_int])
_proto('tokotachi_item_model_locate', CModelIndex, [c_void_p, CModelIndex])
_proto('tokotachi_item_get_text', c_void_p, [c_void_p])
_proto('tokotachi_item_set_text', None, [c_void_p, c_char_p])
_proto('tokotachi_item_get_column_texts', c_void_p, [c_void_p])
_proto('tokotachi_item_set_column_texts', None, [c_void_p, POINTER(c_char_p), c_size_t])
_proto('tokotachi_item_get_icon', c_void_p, [c_void_p])
_proto('tokotachi_item_set_icon', None, [c_void_p, c_void_p])
_proto('tokotachi_item_get_enabled', c_bool, [c_void_p])
_proto('tokotachi_item_set_enabled', None, [c_void_p, c_bool])
_proto('tokotachi_item_get_checkable', c_bool, [c_void_p])
_proto('tokotachi_item_set_checkable', None, [c_void_p, c_bool])
_proto('tokotachi_item_get_checked', c_bool, [c_void_p])
_proto('tokotachi_item_set_checked', None, [c_void_p, c_bool])
_proto('tokotachi_item_get_text_color', CThemeColor, [c_void_p])
_proto('tokotachi_item_set_text_color', None, [c_void_p, CThemeColor])
_proto('tokotachi_item_get_font', CFont, [c_void_p])
_proto('tokotachi_item_set_font', None, [c_void_p, CFont])
_proto('tokotachi_item_get_editable', c_bool, [c_void_p])
_proto('tokotachi_item_set_editable', None, [c_void_p, c_bool])
_proto('tokotachi_item_column_text', c_void_p, [c_void_p, c_int])
_proto('tokotachi_item_set_column_text', None, [c_void_p, c_int, c_char_p])
_proto('tokotachi_list_model_new', c_void_p, [c_void_p])
_proto('tokotachi_list_model_get_items', c_size_t, [c_void_p, POINTER(c_void_p), c_size_t])
_proto('tokotachi_list_model_add_item', c_void_p, [c_void_p, c_char_p])
_proto('tokotachi_list_model_insert_item', c_void_p, [c_void_p, c_int, c_char_p])
_proto('tokotachi_list_model_remove_item', None, [c_void_p, c_void_p])
_proto('tokotachi_list_model_move_item', None, [c_void_p, c_void_p, c_int])
_proto('tokotachi_list_model_clear', None, [c_void_p])
_proto('tokotachi_list_model_index_of', CModelIndex, [c_void_p, c_void_p])
_proto('tokotachi_list_model_item_at', c_void_p, [c_void_p, CModelIndex])
_proto('tokotachi_tree_item_get_parent_item', c_void_p, [c_void_p])
_proto('tokotachi_tree_item_get_children', c_size_t, [c_void_p, POINTER(c_void_p), c_size_t])
_proto('tokotachi_tree_item_add_child', c_void_p, [c_void_p, c_char_p])
_proto('tokotachi_tree_item_insert_child', c_void_p, [c_void_p, c_int, c_char_p])
_proto('tokotachi_tree_model_new', c_void_p, [c_void_p])
_proto('tokotachi_tree_model_get_items', c_size_t, [c_void_p, POINTER(c_void_p), c_size_t])
_proto('tokotachi_tree_model_add_item', c_void_p, [c_void_p, c_char_p])
_proto('tokotachi_tree_model_insert_item', c_void_p, [c_void_p, c_int, c_char_p])
_proto('tokotachi_tree_model_remove_item', None, [c_void_p, c_void_p])
_proto('tokotachi_tree_model_move_item', None, [c_void_p, c_void_p, c_void_p, c_int])
_proto('tokotachi_tree_model_clear', None, [c_void_p])
_proto('tokotachi_tree_model_index_of', CModelIndex, [c_void_p, c_void_p])
_proto('tokotachi_tree_model_item_at', c_void_p, [c_void_p, CModelIndex])
_proto('tokotachi_column_get_title', c_void_p, [c_void_p])
_proto('tokotachi_column_set_title', None, [c_void_p, c_char_p])
_proto('tokotachi_column_get_width', c_float, [c_void_p])
_proto('tokotachi_column_set_width', None, [c_void_p, c_float])
_proto('tokotachi_column_get_alignment', c_int, [c_void_p])
_proto('tokotachi_column_set_alignment', None, [c_void_p, c_int])
_proto('tokotachi_item_view_get_model', c_void_p, [c_void_p])
_proto('tokotachi_item_view_set_model', None, [c_void_p, c_void_p])
_proto('tokotachi_item_view_get_columns', c_size_t, [c_void_p, POINTER(c_void_p), c_size_t])
_proto('tokotachi_item_view_get_header_visible', c_bool, [c_void_p])
_proto('tokotachi_item_view_set_header_visible', None, [c_void_p, c_bool])
_proto('tokotachi_item_view_get_sorting_enabled', c_bool, [c_void_p])
_proto('tokotachi_item_view_set_sorting_enabled', None, [c_void_p, c_bool])
_proto('tokotachi_item_view_get_sort_column', c_int, [c_void_p])
_proto('tokotachi_item_view_set_sort_column', None, [c_void_p, c_int])
_proto('tokotachi_item_view_get_sort_order', c_int, [c_void_p])
_proto('tokotachi_item_view_set_sort_order', None, [c_void_p, c_int])
_proto('tokotachi_item_view_get_selection_mode', c_int, [c_void_p])
_proto('tokotachi_item_view_set_selection_mode', None, [c_void_p, c_int])
_proto('tokotachi_item_view_get_current', CModelIndex, [c_void_p])
_proto('tokotachi_item_view_set_current', None, [c_void_p, CModelIndex])
_proto('tokotachi_item_view_get_selection', c_size_t, [c_void_p, POINTER(CModelIndex), c_size_t])
_proto('tokotachi_item_view_get_editing', c_bool, [c_void_p])
_proto('tokotachi_item_view_get_drag_enabled', c_bool, [c_void_p])
_proto('tokotachi_item_view_set_drag_enabled', None, [c_void_p, c_bool])
_proto('tokotachi_item_view_get_drop_enabled', c_bool, [c_void_p])
_proto('tokotachi_item_view_set_drop_enabled', None, [c_void_p, c_bool])
_proto('tokotachi_item_view_set_on_selection_change', None, [c_void_p, c_void_p, c_void_p, c_void_p])
_proto('tokotachi_item_view_set_on_activate', None, [c_void_p, c_void_p, c_void_p, c_void_p])
_proto('tokotachi_item_view_add_column', c_void_p, [c_void_p, c_char_p, c_float])
_proto('tokotachi_item_view_remove_column', None, [c_void_p, c_void_p])
_proto('tokotachi_item_view_is_selected', c_bool, [c_void_p, CModelIndex])
_proto('tokotachi_item_view_select', None, [c_void_p, CModelIndex, c_bool])
_proto('tokotachi_item_view_select_all', None, [c_void_p])
_proto('tokotachi_item_view_clear_selection', None, [c_void_p])
_proto('tokotachi_item_view_scroll_to', None, [c_void_p, CModelIndex])
_proto('tokotachi_item_view_index_at', CModelIndex, [c_void_p, CPoint])
_proto('tokotachi_item_view_edit', None, [c_void_p, CModelIndex, c_int])
_proto('tokotachi_item_view_finish_editing', None, [c_void_p, c_bool])
_proto('tokotachi_list_view_new', c_void_p, [c_void_p])
_proto('tokotachi_table_view_new', c_void_p, [c_void_p])
_proto('tokotachi_table_view_get_selection_behavior', c_int, [c_void_p])
_proto('tokotachi_table_view_set_selection_behavior', None, [c_void_p, c_int])
_proto('tokotachi_table_view_get_current_column', c_int, [c_void_p])
_proto('tokotachi_table_view_set_current_column', None, [c_void_p, c_int])
_proto('tokotachi_table_view_get_selected_ranges', c_size_t, [c_void_p, POINTER(CCellRange), c_size_t])
_proto('tokotachi_table_view_get_grid_visible', c_bool, [c_void_p])
_proto('tokotachi_table_view_set_grid_visible', None, [c_void_p, c_bool])
_proto('tokotachi_table_view_get_row_header_visible', c_bool, [c_void_p])
_proto('tokotachi_table_view_set_row_header_visible', None, [c_void_p, c_bool])
_proto('tokotachi_table_view_is_cell_selected', c_bool, [c_void_p, c_int, c_int])
_proto('tokotachi_table_view_select_range', None, [c_void_p, CCellRange, c_bool])
_proto('tokotachi_table_view_selected_text', c_void_p, [c_void_p])
_proto('tokotachi_table_view_copy', None, [c_void_p])
_proto('tokotachi_table_view_paste', None, [c_void_p])
_proto('tokotachi_table_view_clear_selected_cells', None, [c_void_p])
_proto('tokotachi_tree_view_new', c_void_p, [c_void_p])
_proto('tokotachi_tree_view_get_indentation', c_float, [c_void_p])
_proto('tokotachi_tree_view_set_indentation', None, [c_void_p, c_float])
_proto('tokotachi_tree_view_expand', None, [c_void_p, CModelIndex])
_proto('tokotachi_tree_view_collapse', None, [c_void_p, CModelIndex])
_proto('tokotachi_tree_view_is_expanded', c_bool, [c_void_p, CModelIndex])
_proto('tokotachi_tree_view_expand_all', None, [c_void_p])
_proto('tokotachi_tree_view_collapse_all', None, [c_void_p])
_proto('tokotachi_variable_new', c_void_p, [c_void_p, c_int])
_proto('tokotachi_variable_get_type', c_int, [c_void_p])
_proto('tokotachi_variable_get_bool', c_bool, [c_void_p])
_proto('tokotachi_variable_set_bool', None, [c_void_p, c_bool])
_proto('tokotachi_variable_get_int', c_int, [c_void_p])
_proto('tokotachi_variable_set_int', None, [c_void_p, c_int])
_proto('tokotachi_variable_get_float', c_float, [c_void_p])
_proto('tokotachi_variable_set_float', None, [c_void_p, c_float])
_proto('tokotachi_variable_get_double', c_double, [c_void_p])
_proto('tokotachi_variable_set_double', None, [c_void_p, c_double])
_proto('tokotachi_variable_get_string', c_void_p, [c_void_p])
_proto('tokotachi_variable_set_string', None, [c_void_p, c_char_p])
_proto('tokotachi_variable_set_on_set', None, [c_void_p, c_void_p, c_void_p, c_void_p])
_proto('tokotachi_variable_set_on_change', None, [c_void_p, c_void_p, c_void_p, c_void_p])
_proto('tokotachi_task_get_running', c_bool, [c_void_p])
_proto('tokotachi_task_get_progress', c_double, [c_void_p])
_proto('tokotachi_task_get_cancelled', c_bool, [c_void_p])
_proto('tokotachi_task_get_error', c_void_p, [c_void_p])
_proto('tokotachi_task_set_on_progress', None, [c_void_p, c_void_p, c_void_p, c_void_p])
_proto('tokotachi_task_set_on_finish', None, [c_void_p, c_void_p, c_void_p, c_void_p])
_proto('tokotachi_task_cancel', None, [c_void_p])
_proto('tokotachi_task_new', c_void_p, [c_void_p])
_proto('tokotachi_task_start', c_bool, [c_void_p, c_void_p, c_void_p, c_void_p])
_proto('tokotachi_task_context_cancel_requested', c_bool, [c_void_p])
_proto('tokotachi_task_context_report_progress', None, [c_void_p, c_double])
_proto('tokotachi_task_context_fail', None, [c_void_p, c_char_p])
_proto('tokotachi_custom_model_new', c_void_p, [c_void_p, POINTER(CModelCallbacks), c_void_p, c_void_p])
_proto('tokotachi_custom_model_notify_rows_inserted', None, [c_void_p, CModelIndex, c_int, c_int])
_proto('tokotachi_custom_model_notify_rows_removing', None, [c_void_p, CModelIndex, c_int, c_int])
_proto('tokotachi_custom_model_notify_data_changed', None, [c_void_p, CModelIndex])
_proto('tokotachi_custom_model_notify_layout_changed', None, [c_void_p])
_proto('tokotachi_custom_model_notify_reset', None, [c_void_p])


# ---- クラス ----


class Object(_ObjectBase):
    """C++ の tokotachi::Object(include/tokotachi/core/property.hpp)。すべてのクラスの基底"""
    owner = _Property(lambda h: _wrap(_lib.tokotachi_object_get_owner(h)), None)


_register(Object)


class Application(_ApplicationMixin, Object):
    """C++ の tokotachi::Application(include/tokotachi/core/application.hpp)。1 つのプロセスに 1 つだけ作る"""
    color_scheme = _Property(lambda h: ColorScheme(_lib.tokotachi_application_get_color_scheme(h)), lambda h, v: _lib.tokotachi_application_set_color_scheme(h, int(v)))
    light_theme = _Property(lambda h: _out_Theme(_get_out(_lib.tokotachi_application_get_light_theme, h, CTheme)), lambda h, v: _lib.tokotachi_application_set_light_theme(h, _in_Theme(v)))
    dark_theme = _Property(lambda h: _out_Theme(_get_out(_lib.tokotachi_application_get_dark_theme, h, CTheme)), lambda h, v: _lib.tokotachi_application_set_dark_theme(h, _in_Theme(v)))
    dark = _Property(lambda h: _lib.tokotachi_application_get_dark(h), None)
    theme = _Property(lambda h: _out_Theme(_get_out(_lib.tokotachi_application_get_theme, h, CTheme)), None)
    language = _Property(lambda h: _out_string(_lib.tokotachi_application_get_language(h)), lambda h, v: _lib.tokotachi_application_set_language(h, _in_string(v)))
    main_window = _Property(lambda h: _wrap(_lib.tokotachi_application_get_main_window(h)), lambda h, v: _lib.tokotachi_application_set_main_window(h, _in_obj(v, 'Window')), repin=True)
    auto_quit_enabled = _Property(lambda h: _lib.tokotachi_application_get_auto_quit_enabled(h), lambda h, v: _lib.tokotachi_application_set_auto_quit_enabled(h, v))
    on_theme_change = _Handler('notify', 'tokotachi_application_set_on_theme_change')

    def exec(self):
        """イベントループを回し、quit で抜ける。戻り値は quit に渡した値"""
        result = _lib.tokotachi_application_exec(self._h)
        return result

    def quit(self, code=0):
        _lib.tokotachi_application_quit(self._h, code)


_register(Application)


class Timer(Object):
    """C++ の tokotachi::Timer(include/tokotachi/core/timer.hpp)。"""

    def __init__(self, owner=None):
        self._attach(_lib.tokotachi_timer_new(_in_obj(owner, 'Object')), True)
        _repin(owner)

    interval = _Property(lambda h: _lib.tokotachi_timer_get_interval(h), lambda h, v: _lib.tokotachi_timer_set_interval(h, v))
    repeat = _Property(lambda h: _lib.tokotachi_timer_get_repeat(h), lambda h, v: _lib.tokotachi_timer_set_repeat(h, v))
    enabled = _Property(lambda h: _lib.tokotachi_timer_get_enabled(h), lambda h, v: _lib.tokotachi_timer_set_enabled(h, v))
    on_timeout = _Handler('notify', 'tokotachi_timer_set_on_timeout')


_register(Timer)


class Widget(Object):
    """C++ の tokotachi::Widget(include/tokotachi/widgets/widget.hpp)。ウィジェットの基底。tokotachi_widget_new は中身の無い入れ物を作る"""

    def __init__(self, parent=None):
        self._attach(_lib.tokotachi_widget_new(_in_obj(parent, 'Widget')), True)
        _repin(parent)

    parent = _Property(lambda h: _wrap(_lib.tokotachi_widget_get_parent(h)), lambda h, v: _lib.tokotachi_widget_set_parent(h, _in_obj(v, 'Widget')), repin=True)
    children = _Property(lambda h: _get_objects(_lib.tokotachi_widget_get_children, h), None)
    window = _Property(lambda h: _wrap(_lib.tokotachi_widget_get_window(h)), None)
    x = _Property(lambda h: _lib.tokotachi_widget_get_x(h), lambda h, v: _lib.tokotachi_widget_set_x(h, v))
    y = _Property(lambda h: _lib.tokotachi_widget_get_y(h), lambda h, v: _lib.tokotachi_widget_set_y(h, v))
    width = _Property(lambda h: _lib.tokotachi_widget_get_width(h), lambda h, v: _lib.tokotachi_widget_set_width(h, v))
    height = _Property(lambda h: _lib.tokotachi_widget_get_height(h), lambda h, v: _lib.tokotachi_widget_set_height(h, v))
    bounds = _Property(lambda h: _out_Rect(_lib.tokotachi_widget_get_bounds(h)), lambda h, v: _lib.tokotachi_widget_set_bounds(h, _in_Rect(v)))
    local_rect = _Property(lambda h: _out_Rect(_lib.tokotachi_widget_get_local_rect(h)), None)
    preferred_size = _Property(lambda h: _out_Size(_lib.tokotachi_widget_get_preferred_size(h)), None)
    minimum_size = _Property(lambda h: _out_Size(_lib.tokotachi_widget_get_minimum_size(h)), lambda h, v: _lib.tokotachi_widget_set_minimum_size(h, _in_Size(v)))
    maximum_size = _Property(lambda h: _out_Size(_lib.tokotachi_widget_get_maximum_size(h)), lambda h, v: _lib.tokotachi_widget_set_maximum_size(h, _in_Size(v)))
    minimum_hint = _Property(lambda h: _out_Size(_lib.tokotachi_widget_get_minimum_hint(h)), None)
    layout = _Property(lambda h: _wrap(_lib.tokotachi_widget_get_layout(h)), None)
    visible = _Property(lambda h: _lib.tokotachi_widget_get_visible(h), lambda h, v: _lib.tokotachi_widget_set_visible(h, v))
    showing = _Property(lambda h: _lib.tokotachi_widget_get_showing(h), None)
    enabled = _Property(lambda h: _lib.tokotachi_widget_get_enabled(h), lambda h, v: _lib.tokotachi_widget_set_enabled(h, v))
    effectively_enabled = _Property(lambda h: _lib.tokotachi_widget_get_effectively_enabled(h), None)
    focus_policy = _Property(lambda h: FocusPolicy(_lib.tokotachi_widget_get_focus_policy(h)), lambda h, v: _lib.tokotachi_widget_set_focus_policy(h, int(v)))
    focused = _Property(lambda h: _lib.tokotachi_widget_get_focused(h), lambda h, v: _lib.tokotachi_widget_set_focused(h, v))
    cursor = _Property(lambda h: MouseCursor(_lib.tokotachi_widget_get_cursor(h)), lambda h, v: _lib.tokotachi_widget_set_cursor(h, int(v)))
    accepts_text_input = _Property(lambda h: _lib.tokotachi_widget_get_accepts_text_input(h), lambda h, v: _lib.tokotachi_widget_set_accepts_text_input(h, v))
    display_scale = _Property(lambda h: _lib.tokotachi_widget_get_display_scale(h), None)
    tooltip = _Property(lambda h: _out_string(_lib.tokotachi_widget_get_tooltip(h)), lambda h, v: _lib.tokotachi_widget_set_tooltip(h, _in_string(v)))
    actions = _Property(lambda h: _get_objects(_lib.tokotachi_widget_get_actions, h), None)
    theme_override = _Property(lambda h: _out_ThemeOverride(_get_out(_lib.tokotachi_widget_get_theme_override, h, CThemeOverride)), lambda h, v: _lib.tokotachi_widget_set_theme_override(h, _in_ThemeOverride(v)))
    theme = _Property(lambda h: _out_Theme(_get_out(_lib.tokotachi_widget_get_theme, h, CTheme)), None)
    accepts_drops = _Property(lambda h: _lib.tokotachi_widget_get_accepts_drops(h), lambda h, v: _lib.tokotachi_widget_set_accepts_drops(h, v))
    on_paint = _Handler('paint', 'tokotachi_widget_set_on_paint')
    on_resize = _Handler('notify', 'tokotachi_widget_set_on_resize')
    on_mouse_down = _Handler('mouse', 'tokotachi_widget_set_on_mouse_down')
    on_mouse_up = _Handler('mouse', 'tokotachi_widget_set_on_mouse_up')
    on_mouse_move = _Handler('mouse', 'tokotachi_widget_set_on_mouse_move')
    on_mouse_enter = _Handler('notify', 'tokotachi_widget_set_on_mouse_enter')
    on_mouse_leave = _Handler('notify', 'tokotachi_widget_set_on_mouse_leave')
    on_mouse_wheel = _Handler('wheel', 'tokotachi_widget_set_on_mouse_wheel')
    on_key_down = _Handler('key', 'tokotachi_widget_set_on_key_down')
    on_key_up = _Handler('key', 'tokotachi_widget_set_on_key_up')
    on_focus_in = _Handler('notify', 'tokotachi_widget_set_on_focus_in')
    on_focus_out = _Handler('notify', 'tokotachi_widget_set_on_focus_out')
    on_drag_move = _Handler('drag', 'tokotachi_widget_set_on_drag_move')
    on_drag_leave = _Handler('notify', 'tokotachi_widget_set_on_drag_leave')
    on_drop = _Handler('drag', 'tokotachi_widget_set_on_drop')
    on_drag_finish = _Handler('drag_finish', 'tokotachi_widget_set_on_drag_finish')

    def add_action(self, action):
        _lib.tokotachi_widget_add_action(self._h, _in_obj(action, 'Action'))
        _repin(action)

    def remove_action(self, action):
        _lib.tokotachi_widget_remove_action(self._h, _in_obj(action, 'Action'))
        _repin(action)

    def invalidate(self):
        _lib.tokotachi_widget_invalidate(self._h)

    def invalidate_rect(self, rect):
        _lib.tokotachi_widget_invalidate_rect(self._h, _in_Rect(rect))

    def request_layout(self):
        _lib.tokotachi_widget_request_layout(self._h)

    def apply_layout(self):
        _lib.tokotachi_widget_apply_layout(self._h)

    def map_to_window(self, p):
        result = _out_Point(_lib.tokotachi_widget_map_to_window(self._h, _in_Point(p)))
        return result

    def map_from_window(self, p):
        result = _out_Point(_lib.tokotachi_widget_map_from_window(self._h, _in_Point(p)))
        return result

    def child_at(self, p):
        result = _wrap(_lib.tokotachi_widget_child_at(self._h, _in_Point(p)))
        _repin(result)
        return result

    def start_drag(self, data, label=''):
        """ドラッグを始める(docs/adr/0047)。マウスのボタンを押している間だけ"""
        result = _lib.tokotachi_widget_start_drag(self._h, _in_DragData(data), _in_string(label))
        return result

    def focus_next_after(self):
        result = _lib.tokotachi_widget_focus_next_after(self._h)
        return result

    def focus_first_within(self):
        result = _lib.tokotachi_widget_focus_first_within(self._h)
        return result


_register(Widget)


class ButtonBase(Widget):
    """C++ の tokotachi::ButtonBase(include/tokotachi/widgets/button_base.hpp)。"""
    __init__ = _ObjectBase.__init__  # 直接は作れない

    text = _Property(lambda h: _out_string(_lib.tokotachi_button_base_get_text(h)), lambda h, v: _lib.tokotachi_button_base_set_text(h, _in_string(v)))
    font = _Property(lambda h: _out_Font(_lib.tokotachi_button_base_get_font(h)), lambda h, v: _lib.tokotachi_button_base_set_font(h, _in_Font(v)))
    on_click = _Handler('notify', 'tokotachi_button_base_set_on_click')


_register(ButtonBase)


class Button(ButtonBase):
    """C++ の tokotachi::Button(include/tokotachi/widgets/button.hpp)。"""

    def __init__(self, parent=None, text=''):
        self._attach(_lib.tokotachi_button_new(_in_obj(parent, 'Widget'), _in_string(text)), True)
        _repin(parent)

    style = _Property(lambda h: ButtonStyle(_lib.tokotachi_button_get_style(h)), lambda h, v: _lib.tokotachi_button_set_style(h, int(v)))
    icon = _Property(lambda h: _out_Image(_lib.tokotachi_button_get_icon(h)), lambda h, v: _lib.tokotachi_button_set_icon(h, _in_Image(v)))
    icon_size = _Property(lambda h: _lib.tokotachi_button_get_icon_size(h), lambda h, v: _lib.tokotachi_button_set_icon_size(h, v))


_register(Button)


class CheckBox(ButtonBase):
    """C++ の tokotachi::CheckBox(include/tokotachi/widgets/check_box.hpp)。"""

    def __init__(self, parent=None, text=''):
        self._attach(_lib.tokotachi_check_box_new(_in_obj(parent, 'Widget'), _in_string(text)), True)
        _repin(parent)

    checked = _Property(lambda h: _lib.tokotachi_check_box_get_checked(h), lambda h, v: _lib.tokotachi_check_box_set_checked(h, v))
    on_change = _Handler('notify', 'tokotachi_check_box_set_on_change')


_register(CheckBox)


class RadioButton(ButtonBase):
    """C++ の tokotachi::RadioButton(include/tokotachi/widgets/radio_button.hpp)。"""

    def __init__(self, parent=None, text='', value=0):
        self._attach(_lib.tokotachi_radio_button_new(_in_obj(parent, 'Widget'), _in_string(text), value), True)
        _repin(parent)

    value = _Property(lambda h: _lib.tokotachi_radio_button_get_value(h), lambda h, v: _lib.tokotachi_radio_button_set_value(h, v))
    group_value = _Property(lambda h: _lib.tokotachi_radio_button_get_group_value(h), lambda h, v: _lib.tokotachi_radio_button_set_group_value(h, v))
    checked = _Property(lambda h: _lib.tokotachi_radio_button_get_checked(h), None)
    on_change = _Handler('notify', 'tokotachi_radio_button_set_on_change')


_register(RadioButton)


class ComboBox(Widget):
    """C++ の tokotachi::ComboBox(include/tokotachi/widgets/combo_box.hpp)。"""

    def __init__(self, parent=None):
        self._attach(_lib.tokotachi_combo_box_new(_in_obj(parent, 'Widget')), True)
        _repin(parent)

    items = _Property(lambda h: _out_strings(_lib.tokotachi_combo_box_get_items(h)), lambda h, v: _lib.tokotachi_combo_box_set_items(h, *_in_strings(v)))
    current_index = _Property(lambda h: _lib.tokotachi_combo_box_get_current_index(h), lambda h, v: _lib.tokotachi_combo_box_set_current_index(h, v))
    current_text = _Property(lambda h: _out_string(_lib.tokotachi_combo_box_get_current_text(h)), None)
    placeholder_text = _Property(lambda h: _out_string(_lib.tokotachi_combo_box_get_placeholder_text(h)), lambda h, v: _lib.tokotachi_combo_box_set_placeholder_text(h, _in_string(v)))
    font = _Property(lambda h: _out_Font(_lib.tokotachi_combo_box_get_font(h)), lambda h, v: _lib.tokotachi_combo_box_set_font(h, _in_Font(v)))
    popup_open = _Property(lambda h: _lib.tokotachi_combo_box_get_popup_open(h), None)
    on_change = _Handler('notify', 'tokotachi_combo_box_set_on_change')

    def open_popup(self):
        _lib.tokotachi_combo_box_open_popup(self._h)

    def close_popup(self):
        _lib.tokotachi_combo_box_close_popup(self._h)


_register(ComboBox)


class Label(Widget):
    """C++ の tokotachi::Label(include/tokotachi/widgets/label.hpp)。"""

    def __init__(self, parent=None, text=''):
        self._attach(_lib.tokotachi_label_new(_in_obj(parent, 'Widget'), _in_string(text)), True)
        _repin(parent)

    text = _Property(lambda h: _out_string(_lib.tokotachi_label_get_text(h)), lambda h, v: _lib.tokotachi_label_set_text(h, _in_string(v)))
    font = _Property(lambda h: _out_Font(_lib.tokotachi_label_get_font(h)), lambda h, v: _lib.tokotachi_label_set_font(h, _in_Font(v)))
    text_color = _Property(lambda h: _out_ThemeColor(_lib.tokotachi_label_get_text_color(h)), lambda h, v: _lib.tokotachi_label_set_text_color(h, _in_ThemeColor(v)))
    alignment = _Property(lambda h: Align(_lib.tokotachi_label_get_alignment(h)), lambda h, v: _lib.tokotachi_label_set_alignment(h, int(v)))
    word_wrap = _Property(lambda h: _lib.tokotachi_label_get_word_wrap(h), lambda h, v: _lib.tokotachi_label_set_word_wrap(h, v))
    icon = _Property(lambda h: _out_Image(_lib.tokotachi_label_get_icon(h)), lambda h, v: _lib.tokotachi_label_set_icon(h, _in_Image(v)))
    icon_size = _Property(lambda h: _lib.tokotachi_label_get_icon_size(h), lambda h, v: _lib.tokotachi_label_set_icon_size(h, v))
    buddy = _Property(lambda h: _wrap(_lib.tokotachi_label_get_buddy(h)), lambda h, v: _lib.tokotachi_label_set_buddy(h, _in_obj(v, 'Widget')), repin=True)


_register(Label)


class TextInputBase(Widget):
    """C++ の tokotachi::TextInputBase(include/tokotachi/widgets/text_input_base.hpp)。"""
    __init__ = _ObjectBase.__init__  # 直接は作れない

    text = _Property(lambda h: _out_string(_lib.tokotachi_text_input_base_get_text(h)), lambda h, v: _lib.tokotachi_text_input_base_set_text(h, _in_string(v)))
    placeholder_text = _Property(lambda h: _out_string(_lib.tokotachi_text_input_base_get_placeholder_text(h)), lambda h, v: _lib.tokotachi_text_input_base_set_placeholder_text(h, _in_string(v)))
    font = _Property(lambda h: _out_Font(_lib.tokotachi_text_input_base_get_font(h)), lambda h, v: _lib.tokotachi_text_input_base_set_font(h, _in_Font(v)))
    cursor_position = _Property(lambda h: _lib.tokotachi_text_input_base_get_cursor_position(h), lambda h, v: _lib.tokotachi_text_input_base_set_cursor_position(h, v))
    has_selection = _Property(lambda h: _lib.tokotachi_text_input_base_get_has_selection(h), None)
    selected_text = _Property(lambda h: _out_string(_lib.tokotachi_text_input_base_get_selected_text(h)), None)
    can_undo = _Property(lambda h: _lib.tokotachi_text_input_base_get_can_undo(h), None)
    can_redo = _Property(lambda h: _lib.tokotachi_text_input_base_get_can_redo(h), None)
    on_change = _Handler('notify', 'tokotachi_text_input_base_set_on_change')

    def select_all(self):
        _lib.tokotachi_text_input_base_select_all(self._h)

    def clear_selection(self):
        _lib.tokotachi_text_input_base_clear_selection(self._h)

    def cut(self):
        _lib.tokotachi_text_input_base_cut(self._h)

    def copy(self):
        _lib.tokotachi_text_input_base_copy(self._h)

    def paste(self):
        _lib.tokotachi_text_input_base_paste(self._h)

    def insert(self, s):
        _lib.tokotachi_text_input_base_insert(self._h, _in_string(s))

    def undo(self):
        _lib.tokotachi_text_input_base_undo(self._h)

    def redo(self):
        _lib.tokotachi_text_input_base_redo(self._h)

    def clear_undo_history(self):
        _lib.tokotachi_text_input_base_clear_undo_history(self._h)


_register(TextInputBase)


class LineEdit(TextInputBase):
    """C++ の tokotachi::LineEdit(include/tokotachi/widgets/line_edit.hpp)。"""

    def __init__(self, parent=None, text=''):
        self._attach(_lib.tokotachi_line_edit_new(_in_obj(parent, 'Widget'), _in_string(text)), True)
        _repin(parent)

    on_submit = _Handler('notify', 'tokotachi_line_edit_set_on_submit')


_register(LineEdit)


class TextEdit(TextInputBase):
    """C++ の tokotachi::TextEdit(include/tokotachi/widgets/text_edit.hpp)。"""

    def __init__(self, parent=None, text=''):
        self._attach(_lib.tokotachi_text_edit_new(_in_obj(parent, 'Widget'), _in_string(text)), True)
        _repin(parent)

    word_wrap = _Property(lambda h: _lib.tokotachi_text_edit_get_word_wrap(h), lambda h, v: _lib.tokotachi_text_edit_set_word_wrap(h, v))


_register(TextEdit)


class PaintBox(Widget):
    """C++ の tokotachi::PaintBox(include/tokotachi/widgets/paint_box.hpp)。"""

    def __init__(self, parent=None):
        self._attach(_lib.tokotachi_paint_box_new(_in_obj(parent, 'Widget')), True)
        _repin(parent)

    cached = _Property(lambda h: _lib.tokotachi_paint_box_get_cached(h), lambda h, v: _lib.tokotachi_paint_box_set_cached(h, v))

    def refresh(self):
        _lib.tokotachi_paint_box_refresh(self._h)


_register(PaintBox)


class ProgressBar(Widget):
    """C++ の tokotachi::ProgressBar(include/tokotachi/widgets/progress_bar.hpp)。"""

    def __init__(self, parent=None):
        self._attach(_lib.tokotachi_progress_bar_new(_in_obj(parent, 'Widget')), True)
        _repin(parent)

    minimum = _Property(lambda h: _lib.tokotachi_progress_bar_get_minimum(h), lambda h, v: _lib.tokotachi_progress_bar_set_minimum(h, v))
    maximum = _Property(lambda h: _lib.tokotachi_progress_bar_get_maximum(h), lambda h, v: _lib.tokotachi_progress_bar_set_maximum(h, v))
    value = _Property(lambda h: _lib.tokotachi_progress_bar_get_value(h), lambda h, v: _lib.tokotachi_progress_bar_set_value(h, v))
    text_visible = _Property(lambda h: _lib.tokotachi_progress_bar_get_text_visible(h), lambda h, v: _lib.tokotachi_progress_bar_set_text_visible(h, v))
    indeterminate = _Property(lambda h: _lib.tokotachi_progress_bar_get_indeterminate(h), lambda h, v: _lib.tokotachi_progress_bar_set_indeterminate(h, v))


_register(ProgressBar)


class RangeBase(Widget):
    """C++ の tokotachi::RangeBase(include/tokotachi/widgets/range_base.hpp)。"""
    __init__ = _ObjectBase.__init__  # 直接は作れない

    minimum = _Property(lambda h: _lib.tokotachi_range_base_get_minimum(h), lambda h, v: _lib.tokotachi_range_base_set_minimum(h, v))
    maximum = _Property(lambda h: _lib.tokotachi_range_base_get_maximum(h), lambda h, v: _lib.tokotachi_range_base_set_maximum(h, v))
    value = _Property(lambda h: _lib.tokotachi_range_base_get_value(h), lambda h, v: _lib.tokotachi_range_base_set_value(h, v))
    single_step = _Property(lambda h: _lib.tokotachi_range_base_get_single_step(h), lambda h, v: _lib.tokotachi_range_base_set_single_step(h, v))
    page_step = _Property(lambda h: _lib.tokotachi_range_base_get_page_step(h), lambda h, v: _lib.tokotachi_range_base_set_page_step(h, v))
    on_change = _Handler('notify', 'tokotachi_range_base_set_on_change')


_register(RangeBase)


class Slider(RangeBase):
    """C++ の tokotachi::Slider(include/tokotachi/widgets/slider.hpp)。"""

    def __init__(self, parent=None, orientation=Orientation.HORIZONTAL):
        self._attach(_lib.tokotachi_slider_new(_in_obj(parent, 'Widget'), int(orientation)), True)
        _repin(parent)

    orientation = _Property(lambda h: Orientation(_lib.tokotachi_slider_get_orientation(h)), lambda h, v: _lib.tokotachi_slider_set_orientation(h, int(v)))
    tick_interval = _Property(lambda h: _lib.tokotachi_slider_get_tick_interval(h), lambda h, v: _lib.tokotachi_slider_set_tick_interval(h, v))


_register(Slider)


class SpinBox(RangeBase):
    """C++ の tokotachi::SpinBox(include/tokotachi/widgets/spin_box.hpp)。"""

    def __init__(self, parent=None):
        self._attach(_lib.tokotachi_spin_box_new(_in_obj(parent, 'Widget')), True)
        _repin(parent)

    decimals = _Property(lambda h: _lib.tokotachi_spin_box_get_decimals(h), lambda h, v: _lib.tokotachi_spin_box_set_decimals(h, v))
    prefix = _Property(lambda h: _out_string(_lib.tokotachi_spin_box_get_prefix(h)), lambda h, v: _lib.tokotachi_spin_box_set_prefix(h, _in_string(v)))
    suffix = _Property(lambda h: _out_string(_lib.tokotachi_spin_box_get_suffix(h)), lambda h, v: _lib.tokotachi_spin_box_set_suffix(h, _in_string(v)))
    text = _Property(lambda h: _out_string(_lib.tokotachi_spin_box_get_text(h)), None)

    def commit(self):
        _lib.tokotachi_spin_box_commit(self._h)


_register(SpinBox)


class ScrollArea(Widget):
    """C++ の tokotachi::ScrollArea(include/tokotachi/widgets/scroll_area.hpp)。"""

    def __init__(self, parent=None):
        self._attach(_lib.tokotachi_scroll_area_new(_in_obj(parent, 'Widget')), True)
        _repin(parent)

    content = _Property(lambda h: _wrap(_lib.tokotachi_scroll_area_get_content(h)), lambda h, v: _lib.tokotachi_scroll_area_set_content(h, _in_obj(v, 'Widget')), repin=True)
    fit_width = _Property(lambda h: _lib.tokotachi_scroll_area_get_fit_width(h), lambda h, v: _lib.tokotachi_scroll_area_set_fit_width(h, v))
    scroll_position = _Property(lambda h: _out_Point(_lib.tokotachi_scroll_area_get_scroll_position(h)), lambda h, v: _lib.tokotachi_scroll_area_set_scroll_position(h, _in_Point(v)))

    def scroll_to_visible(self, rect):
        _lib.tokotachi_scroll_area_scroll_to_visible(self._h, _in_Rect(rect))


_register(ScrollArea)


class ScrollBar(Widget):
    """C++ の tokotachi::ScrollBar(include/tokotachi/widgets/scroll_bar.hpp)。"""

    def __init__(self, parent=None, orientation=Orientation.VERTICAL):
        self._attach(_lib.tokotachi_scroll_bar_new(_in_obj(parent, 'Widget'), int(orientation)), True)
        _repin(parent)

    orientation = _Property(lambda h: Orientation(_lib.tokotachi_scroll_bar_get_orientation(h)), lambda h, v: _lib.tokotachi_scroll_bar_set_orientation(h, int(v)))
    value = _Property(lambda h: _lib.tokotachi_scroll_bar_get_value(h), lambda h, v: _lib.tokotachi_scroll_bar_set_value(h, v))
    maximum = _Property(lambda h: _lib.tokotachi_scroll_bar_get_maximum(h), lambda h, v: _lib.tokotachi_scroll_bar_set_maximum(h, v))
    page_step = _Property(lambda h: _lib.tokotachi_scroll_bar_get_page_step(h), lambda h, v: _lib.tokotachi_scroll_bar_set_page_step(h, v))
    single_step = _Property(lambda h: _lib.tokotachi_scroll_bar_get_single_step(h), lambda h, v: _lib.tokotachi_scroll_bar_set_single_step(h, v))
    on_change = _Handler('notify', 'tokotachi_scroll_bar_set_on_change')


_register(ScrollBar)


class SplitterPane(Object):
    """C++ の tokotachi::SplitterPane(include/tokotachi/widgets/splitter.hpp)。Splitter が作り、所有する"""
    __init__ = _ObjectBase.__init__  # 直接は作れない

    stretch = _Property(lambda h: _lib.tokotachi_splitter_pane_get_stretch(h), lambda h, v: _lib.tokotachi_splitter_pane_set_stretch(h, v))
    collapsible = _Property(lambda h: _lib.tokotachi_splitter_pane_get_collapsible(h), lambda h, v: _lib.tokotachi_splitter_pane_set_collapsible(h, v))
    collapsed = _Property(lambda h: _lib.tokotachi_splitter_pane_get_collapsed(h), None)
    widget = _Property(lambda h: _wrap(_lib.tokotachi_splitter_pane_get_widget(h)), None)


_register(SplitterPane)


class Splitter(Widget):
    """C++ の tokotachi::Splitter(include/tokotachi/widgets/splitter.hpp)。"""

    def __init__(self, parent=None, orientation=Orientation.HORIZONTAL):
        self._attach(_lib.tokotachi_splitter_new(_in_obj(parent, 'Widget'), int(orientation)), True)
        _repin(parent)

    orientation = _Property(lambda h: Orientation(_lib.tokotachi_splitter_get_orientation(h)), lambda h, v: _lib.tokotachi_splitter_set_orientation(h, int(v)))
    panes = _Property(lambda h: _get_objects(_lib.tokotachi_splitter_get_panes, h), None)
    sizes = _Property(lambda h: _get_floats(_lib.tokotachi_splitter_get_sizes, h), lambda h, v: _lib.tokotachi_splitter_set_sizes(h, *_in_floats(v)))
    on_change = _Handler('notify', 'tokotachi_splitter_set_on_change')

    def add_widget(self, widget, stretch=1):
        result = _wrap(_lib.tokotachi_splitter_add_widget(self._h, _in_obj(widget, 'Widget'), stretch))
        _repin(widget)
        _repin(result)
        return result

    def insert_widget(self, index, widget, stretch=1):
        result = _wrap(_lib.tokotachi_splitter_insert_widget(self._h, index, _in_obj(widget, 'Widget'), stretch))
        _repin(widget)
        _repin(result)
        return result


_register(Splitter)


class Tab(Object):
    """C++ の tokotachi::Tab(include/tokotachi/widgets/tab_widget.hpp)。TabWidget が作り、所有する"""
    __init__ = _ObjectBase.__init__  # 直接は作れない

    title = _Property(lambda h: _out_string(_lib.tokotachi_tab_get_title(h)), lambda h, v: _lib.tokotachi_tab_set_title(h, _in_string(v)))
    enabled = _Property(lambda h: _lib.tokotachi_tab_get_enabled(h), lambda h, v: _lib.tokotachi_tab_set_enabled(h, v))
    page = _Property(lambda h: _wrap(_lib.tokotachi_tab_get_page(h)), None)


_register(Tab)


class TabWidget(Widget):
    """C++ の tokotachi::TabWidget(include/tokotachi/widgets/tab_widget.hpp)。"""

    def __init__(self, parent=None):
        self._attach(_lib.tokotachi_tab_widget_new(_in_obj(parent, 'Widget')), True)
        _repin(parent)

    tabs = _Property(lambda h: _get_objects(_lib.tokotachi_tab_widget_get_tabs, h), None)
    current_index = _Property(lambda h: _lib.tokotachi_tab_widget_get_current_index(h), lambda h, v: _lib.tokotachi_tab_widget_set_current_index(h, v))
    current_tab = _Property(lambda h: _wrap(_lib.tokotachi_tab_widget_get_current_tab(h)), None)
    on_change = _Handler('notify', 'tokotachi_tab_widget_set_on_change')

    def add_tab(self, page, title):
        result = _wrap(_lib.tokotachi_tab_widget_add_tab(self._h, _in_obj(page, 'Widget'), _in_string(title)))
        _repin(page)
        _repin(result)
        return result

    def insert_tab(self, index, page, title):
        result = _wrap(_lib.tokotachi_tab_widget_insert_tab(self._h, index, _in_obj(page, 'Widget'), _in_string(title)))
        _repin(page)
        _repin(result)
        return result

    def remove_tab(self, tab):
        """タブを除き、ページを返す(ページは親が無くなり、呼んだ側が破棄する)"""
        result = _wrap(_lib.tokotachi_tab_widget_remove_tab(self._h, _in_obj(tab, 'Tab')))
        _repin(tab)
        _repin(result)
        return result


_register(TabWidget)


class GroupBox(Widget):
    """C++ の tokotachi::GroupBox(include/tokotachi/widgets/group_box.hpp)。"""

    def __init__(self, parent=None, title=''):
        self._attach(_lib.tokotachi_group_box_new(_in_obj(parent, 'Widget'), _in_string(title)), True)
        _repin(parent)

    title = _Property(lambda h: _out_string(_lib.tokotachi_group_box_get_title(h)), lambda h, v: _lib.tokotachi_group_box_set_title(h, _in_string(v)))
    content_rect = _Property(lambda h: _out_Rect(_lib.tokotachi_group_box_get_content_rect(h)), None)


_register(GroupBox)


class ImageBox(Widget):
    """C++ の tokotachi::ImageBox(include/tokotachi/widgets/image_box.hpp)。"""

    def __init__(self, parent=None, image=None):
        self._attach(_lib.tokotachi_image_box_new(_in_obj(parent, 'Widget'), _in_Image(image)), True)
        _repin(parent)

    image = _Property(lambda h: _out_Image(_lib.tokotachi_image_box_get_image(h)), lambda h, v: _lib.tokotachi_image_box_set_image(h, _in_Image(v)))
    fit = _Property(lambda h: ImageFit(_lib.tokotachi_image_box_get_fit(h)), lambda h, v: _lib.tokotachi_image_box_set_fit(h, int(v)))


_register(ImageBox)


class Calendar(Widget):
    """C++ の tokotachi::Calendar(include/tokotachi/widgets/calendar.hpp)。"""

    def __init__(self, parent=None):
        self._attach(_lib.tokotachi_calendar_new(_in_obj(parent, 'Widget')), True)
        _repin(parent)

    date = _Property(lambda h: _out_Date(_lib.tokotachi_calendar_get_date(h)), lambda h, v: _lib.tokotachi_calendar_set_date(h, _in_Date(v)))
    minimum = _Property(lambda h: _out_Date(_lib.tokotachi_calendar_get_minimum(h)), lambda h, v: _lib.tokotachi_calendar_set_minimum(h, _in_Date(v)))
    maximum = _Property(lambda h: _out_Date(_lib.tokotachi_calendar_get_maximum(h)), lambda h, v: _lib.tokotachi_calendar_set_maximum(h, _in_Date(v)))
    first_day_of_week = _Property(lambda h: DayOfWeek(_lib.tokotachi_calendar_get_first_day_of_week(h)), lambda h, v: _lib.tokotachi_calendar_set_first_day_of_week(h, int(v)))
    shown_year = _Property(lambda h: _lib.tokotachi_calendar_get_shown_year(h), None)
    shown_month = _Property(lambda h: _lib.tokotachi_calendar_get_shown_month(h), None)
    on_change = _Handler('notify', 'tokotachi_calendar_set_on_change')
    on_activate = _Handler('notify', 'tokotachi_calendar_set_on_activate')

    def show_month(self, year, month):
        _lib.tokotachi_calendar_show_month(self._h, year, month)


_register(Calendar)


class DateEdit(Widget):
    """C++ の tokotachi::DateEdit(include/tokotachi/widgets/date_edit.hpp)。"""

    def __init__(self, parent=None):
        self._attach(_lib.tokotachi_date_edit_new(_in_obj(parent, 'Widget')), True)
        _repin(parent)

    date = _Property(lambda h: _out_Date(_lib.tokotachi_date_edit_get_date(h)), lambda h, v: _lib.tokotachi_date_edit_set_date(h, _in_Date(v)))
    minimum = _Property(lambda h: _out_Date(_lib.tokotachi_date_edit_get_minimum(h)), lambda h, v: _lib.tokotachi_date_edit_set_minimum(h, _in_Date(v)))
    maximum = _Property(lambda h: _out_Date(_lib.tokotachi_date_edit_get_maximum(h)), lambda h, v: _lib.tokotachi_date_edit_set_maximum(h, _in_Date(v)))
    format = _Property(lambda h: _out_string(_lib.tokotachi_date_edit_get_format(h)), lambda h, v: _lib.tokotachi_date_edit_set_format(h, _in_string(v)))
    first_day_of_week = _Property(lambda h: DayOfWeek(_lib.tokotachi_date_edit_get_first_day_of_week(h)), lambda h, v: _lib.tokotachi_date_edit_set_first_day_of_week(h, int(v)))
    placeholder_text = _Property(lambda h: _out_string(_lib.tokotachi_date_edit_get_placeholder_text(h)), lambda h, v: _lib.tokotachi_date_edit_set_placeholder_text(h, _in_string(v)))
    text = _Property(lambda h: _out_string(_lib.tokotachi_date_edit_get_text(h)), None)
    calendar_open = _Property(lambda h: _lib.tokotachi_date_edit_get_calendar_open(h), None)
    on_change = _Handler('notify', 'tokotachi_date_edit_set_on_change')

    def commit(self):
        _lib.tokotachi_date_edit_commit(self._h)

    def open_calendar(self):
        _lib.tokotachi_date_edit_open_calendar(self._h)

    def close_calendar(self):
        _lib.tokotachi_date_edit_close_calendar(self._h)


_register(DateEdit)


class ColorButton(ButtonBase):
    """C++ の tokotachi::ColorButton(include/tokotachi/widgets/color_button.hpp)。"""

    def __init__(self, parent=None, color=Color(0, 0, 0)):
        self._attach(_lib.tokotachi_color_button_new(_in_obj(parent, 'Widget'), _in_Color(color)), True)
        _repin(parent)

    color = _Property(lambda h: _out_Color(_lib.tokotachi_color_button_get_color(h)), lambda h, v: _lib.tokotachi_color_button_set_color(h, _in_Color(v)))
    alpha_enabled = _Property(lambda h: _lib.tokotachi_color_button_get_alpha_enabled(h), lambda h, v: _lib.tokotachi_color_button_set_alpha_enabled(h, v))
    dialog_title = _Property(lambda h: _out_string(_lib.tokotachi_color_button_get_dialog_title(h)), lambda h, v: _lib.tokotachi_color_button_set_dialog_title(h, _in_string(v)))
    on_change = _Handler('notify', 'tokotachi_color_button_set_on_change')

    def open_dialog(self):
        _lib.tokotachi_color_button_open_dialog(self._h)


_register(ColorButton)


class Window(_WindowMixin, Widget):
    """C++ の tokotachi::Window(include/tokotachi/windows/window.hpp)。Application を作った後に作る。Application が所有する(docs/adr/0062)"""
    title = _Property(lambda h: _out_string(_lib.tokotachi_window_get_title(h)), lambda h, v: _lib.tokotachi_window_set_title(h, _in_string(v)))
    background = _Property(lambda h: _out_ThemeColor(_lib.tokotachi_window_get_background(h)), lambda h, v: _lib.tokotachi_window_set_background(h, _in_ThemeColor(v)))
    focused_widget = _Property(lambda h: _wrap(_lib.tokotachi_window_get_focused_widget(h)), None)
    menu_bar = _Property(lambda h: _wrap(_lib.tokotachi_window_get_menu_bar(h)), lambda h, v: _lib.tokotachi_window_set_menu_bar(h, _in_obj(v, 'MenuBar')), repin=True)
    tool_bar = _Property(lambda h: _wrap(_lib.tokotachi_window_get_tool_bar(h)), lambda h, v: _lib.tokotachi_window_set_tool_bar(h, _in_obj(v, 'ToolBar')), repin=True)
    status_bar = _Property(lambda h: _wrap(_lib.tokotachi_window_get_status_bar(h)), lambda h, v: _lib.tokotachi_window_set_status_bar(h, _in_obj(v, 'StatusBar')), repin=True)
    destroy_on_close = _Property(lambda h: _lib.tokotachi_window_get_destroy_on_close(h), lambda h, v: _lib.tokotachi_window_set_destroy_on_close(h, v))
    on_close = _Handler('close', 'tokotachi_window_set_on_close')

    def close(self):
        _lib.tokotachi_window_close(self._h)

    def save_png(self, path):
        result = _lib.tokotachi_window_save_png(self._h, _in_string(path))
        return result

    def capture(self):
        """全体を描いて Bitmap にする(tokotachi_bitmap_free で解放する)"""
        result = _out_Bitmap(_lib.tokotachi_window_capture(self._h))
        return result


_register(Window)


class Dialog(Window):
    """C++ の tokotachi::Dialog(include/tokotachi/windows/dialog.hpp)。parent が所有する"""

    def __init__(self, parent, title=''):
        self._attach(_lib.tokotachi_dialog_new(_in_obj(parent, 'Window'), _in_string(title)), True)
        _repin(parent)

    result = _Property(lambda h: DialogButton(_lib.tokotachi_dialog_get_result(h)), None)
    on_finish = _Handler('notify', 'tokotachi_dialog_set_on_finish')

    def add_button(self, button, text=''):
        """text が空なら、言語に合わせた標準の文字"""
        result = _wrap(_lib.tokotachi_dialog_add_button(self._h, int(button), _in_string(text)))
        _repin(result)
        return result

    def open(self):
        _lib.tokotachi_dialog_open(self._h)

    def finish(self, result):
        _lib.tokotachi_dialog_finish(self._h, int(result))


_register(Dialog)


class MessageDialog(_MessageDialogMixin, Dialog):
    """C++ の tokotachi::MessageDialog(include/tokotachi/windows/message_dialog.hpp)。"""

    def __init__(self, parent, title='', text='', type=MessageType.INFORMATION):
        self._attach(_lib.tokotachi_message_dialog_new(_in_obj(parent, 'Window'), _in_string(title), _in_string(text), int(type)), True)
        _repin(parent)

    text = _Property(lambda h: _out_string(_lib.tokotachi_message_dialog_get_text(h)), lambda h, v: _lib.tokotachi_message_dialog_set_text(h, _in_string(v)))
    type = _Property(lambda h: MessageType(_lib.tokotachi_message_dialog_get_type(h)), lambda h, v: _lib.tokotachi_message_dialog_set_type(h, int(v)))


_register(MessageDialog)


class ColorDialog(_ColorDialogMixin, Dialog):
    """C++ の tokotachi::ColorDialog(include/tokotachi/windows/color_dialog.hpp)。"""

    def __init__(self, parent, title=''):
        self._attach(_lib.tokotachi_color_dialog_new(_in_obj(parent, 'Window'), _in_string(title)), True)
        _repin(parent)

    color = _Property(lambda h: _out_Color(_lib.tokotachi_color_dialog_get_color(h)), lambda h, v: _lib.tokotachi_color_dialog_set_color(h, _in_Color(v)))
    alpha_enabled = _Property(lambda h: _lib.tokotachi_color_dialog_get_alpha_enabled(h), lambda h, v: _lib.tokotachi_color_dialog_set_alpha_enabled(h, v))


_register(ColorDialog)


class FileDialog(Object):
    """C++ の tokotachi::FileDialog(include/tokotachi/windows/file_dialog.hpp)。"""

    def __init__(self, parent=None, mode=FileDialogMode.OPEN):
        self._attach(_lib.tokotachi_file_dialog_new(_in_obj(parent, 'Window'), int(mode)), True)
        _repin(parent)

    mode = _Property(lambda h: FileDialogMode(_lib.tokotachi_file_dialog_get_mode(h)), lambda h, v: _lib.tokotachi_file_dialog_set_mode(h, int(v)))
    title = _Property(lambda h: _out_string(_lib.tokotachi_file_dialog_get_title(h)), lambda h, v: _lib.tokotachi_file_dialog_set_title(h, _in_string(v)))
    filters = _Property(lambda h: _out_strings(_lib.tokotachi_file_dialog_get_filters(h)), lambda h, v: _lib.tokotachi_file_dialog_set_filters(h, *_in_strings(v)))
    location = _Property(lambda h: _out_string(_lib.tokotachi_file_dialog_get_location(h)), lambda h, v: _lib.tokotachi_file_dialog_set_location(h, _in_string(v)))
    multi_enabled = _Property(lambda h: _lib.tokotachi_file_dialog_get_multi_enabled(h), lambda h, v: _lib.tokotachi_file_dialog_set_multi_enabled(h, v))
    paths = _Property(lambda h: _out_strings(_lib.tokotachi_file_dialog_get_paths(h)), None)
    error = _Property(lambda h: _out_string(_lib.tokotachi_file_dialog_get_error(h)), None)
    selected_filter = _Property(lambda h: _lib.tokotachi_file_dialog_get_selected_filter(h), None)
    running = _Property(lambda h: _lib.tokotachi_file_dialog_get_running(h), None)
    on_finish = _Handler('notify', 'tokotachi_file_dialog_set_on_finish')

    def open(self):
        _lib.tokotachi_file_dialog_open(self._h)


_register(FileDialog)


class MenuBar(Widget):
    """C++ の tokotachi::MenuBar(include/tokotachi/windows/menu_bar.hpp)。"""

    def __init__(self, parent=None):
        self._attach(_lib.tokotachi_menu_bar_new(_in_obj(parent, 'Widget')), True)
        _repin(parent)

    menus = _Property(lambda h: _get_objects(_lib.tokotachi_menu_bar_get_menus, h), None)

    def add_menu(self, title):
        result = _wrap(_lib.tokotachi_menu_bar_add_menu(self._h, _in_string(title)))
        _repin(result)
        return result

    def remove_menu(self, menu):
        _lib.tokotachi_menu_bar_remove_menu(self._h, _in_obj(menu, 'Menu'))
        _repin(menu)


_register(MenuBar)


class Popup(Window):
    """C++ の tokotachi::Popup(include/tokotachi/windows/popup.hpp)。"""

    def __init__(self, owner_widget, kind=PopupKind.MENU, owner_object=None):
        self._attach(_lib.tokotachi_popup_new(_in_obj(owner_widget, 'Widget'), int(kind), _in_obj(owner_object, 'Object')), True)
        _repin(owner_widget)
        _repin(owner_object)

    owner_widget = _Property(lambda h: _wrap(_lib.tokotachi_popup_get_owner_widget(h)), None)
    kind = _Property(lambda h: PopupKind(_lib.tokotachi_popup_get_kind(h)), None)
    on_closed = _Handler('notify', 'tokotachi_popup_set_on_closed')

    def open(self, anchor, size, placement=PopupPlacement.BELOW):
        _lib.tokotachi_popup_open(self._h, _in_Rect(anchor), _in_Size(size), int(placement))


_register(Popup)


class StatusBar(Widget):
    """C++ の tokotachi::StatusBar(include/tokotachi/windows/status_bar.hpp)。"""

    def __init__(self, parent=None):
        self._attach(_lib.tokotachi_status_bar_new(_in_obj(parent, 'Widget')), True)
        _repin(parent)

    text = _Property(lambda h: _out_string(_lib.tokotachi_status_bar_get_text(h)), lambda h, v: _lib.tokotachi_status_bar_set_text(h, _in_string(v)))
    message = _Property(lambda h: _out_string(_lib.tokotachi_status_bar_get_message(h)), None)

    def show_message(self, message, timeout_ms=0):
        _lib.tokotachi_status_bar_show_message(self._h, _in_string(message), timeout_ms)

    def clear_message(self):
        _lib.tokotachi_status_bar_clear_message(self._h)

    def add_widget(self, widget):
        _lib.tokotachi_status_bar_add_widget(self._h, _in_obj(widget, 'Widget'))
        _repin(widget)


_register(StatusBar)


class ToolBar(Widget):
    """C++ の tokotachi::ToolBar(include/tokotachi/windows/tool_bar.hpp)。"""

    def __init__(self, parent=None):
        self._attach(_lib.tokotachi_tool_bar_new(_in_obj(parent, 'Widget')), True)
        _repin(parent)

    button_style = _Property(lambda h: ToolButtonStyle(_lib.tokotachi_tool_bar_get_button_style(h)), lambda h, v: _lib.tokotachi_tool_bar_set_button_style(h, int(v)))

    def add_item(self, text):
        """Action を作ってボタンとして加える(ツールバーが所有する)"""
        result = _wrap(_lib.tokotachi_tool_bar_add_item(self._h, _in_string(text)))
        _repin(result)
        return result

    def add_separator(self):
        _lib.tokotachi_tool_bar_add_separator(self._h)

    def add_widget(self, widget):
        _lib.tokotachi_tool_bar_add_widget(self._h, _in_obj(widget, 'Widget'))
        _repin(widget)


_register(ToolBar)


class Layout(Object):
    """C++ の tokotachi::Layout(include/tokotachi/layout/layout.hpp)。target が所有する(target が前のレイアウトを持っていれば、それは破棄される)"""
    __init__ = _ObjectBase.__init__  # 直接は作れない

    widget = _Property(lambda h: _wrap(_lib.tokotachi_layout_get_widget(h)), None)
    margins = _Property(lambda h: _out_Margins(_lib.tokotachi_layout_get_margins(h)), lambda h, v: _lib.tokotachi_layout_set_margins(h, _in_Margins(v)))

    def remove(self, widget):
        _lib.tokotachi_layout_remove(self._h, _in_obj(widget, 'Widget'))
        _repin(widget)


_register(Layout)


class BoxLayout(Layout):
    """C++ の tokotachi::BoxLayout(include/tokotachi/layout/box_layout.hpp)。"""

    def __init__(self, target, orientation=Orientation.VERTICAL):
        self._attach(_lib.tokotachi_box_layout_new(_in_obj(target, 'Widget'), int(orientation)), True)
        _repin(target)

    orientation = _Property(lambda h: Orientation(_lib.tokotachi_box_layout_get_orientation(h)), lambda h, v: _lib.tokotachi_box_layout_set_orientation(h, int(v)))
    spacing = _Property(lambda h: _lib.tokotachi_box_layout_get_spacing(h), lambda h, v: _lib.tokotachi_box_layout_set_spacing(h, v))

    def add(self, widget, stretch=0, cross_alignment=Align.FILL):
        _lib.tokotachi_box_layout_add(self._h, _in_obj(widget, 'Widget'), stretch, int(cross_alignment))
        _repin(widget)

    def add_spacing(self, size):
        _lib.tokotachi_box_layout_add_spacing(self._h, size)

    def add_stretch(self, stretch=1):
        _lib.tokotachi_box_layout_add_stretch(self._h, stretch)

    def set_stretch(self, widget, stretch):
        _lib.tokotachi_box_layout_set_stretch(self._h, _in_obj(widget, 'Widget'), stretch)
        _repin(widget)

    def set_alignment(self, widget, cross_alignment):
        _lib.tokotachi_box_layout_set_alignment(self._h, _in_obj(widget, 'Widget'), int(cross_alignment))
        _repin(widget)


_register(BoxLayout)


class GridLayout(Layout):
    """C++ の tokotachi::GridLayout(include/tokotachi/layout/grid_layout.hpp)。"""

    def __init__(self, target):
        self._attach(_lib.tokotachi_grid_layout_new(_in_obj(target, 'Widget')), True)
        _repin(target)

    row_spacing = _Property(lambda h: _lib.tokotachi_grid_layout_get_row_spacing(h), lambda h, v: _lib.tokotachi_grid_layout_set_row_spacing(h, v))
    column_spacing = _Property(lambda h: _lib.tokotachi_grid_layout_get_column_spacing(h), lambda h, v: _lib.tokotachi_grid_layout_set_column_spacing(h, v))

    def add(self, widget, row, column, row_span=1, column_span=1, horizontal=Align.FILL, vertical=Align.CENTER):
        _lib.tokotachi_grid_layout_add(self._h, _in_obj(widget, 'Widget'), row, column, row_span, column_span, int(horizontal), int(vertical))
        _repin(widget)

    def set_row_stretch(self, row, stretch):
        _lib.tokotachi_grid_layout_set_row_stretch(self._h, row, stretch)

    def set_column_stretch(self, column, stretch):
        _lib.tokotachi_grid_layout_set_column_stretch(self._h, column, stretch)

    def set_row_minimum(self, row, size):
        _lib.tokotachi_grid_layout_set_row_minimum(self._h, row, size)

    def set_column_minimum(self, column, size):
        _lib.tokotachi_grid_layout_set_column_minimum(self._h, column, size)


_register(GridLayout)


class PlaceLayout(Layout):
    """C++ の tokotachi::PlaceLayout(include/tokotachi/layout/place_layout.hpp)。"""

    def __init__(self, target):
        self._attach(_lib.tokotachi_place_layout_new(_in_obj(target, 'Widget')), True)
        _repin(target)


    def add(self, widget, placement):
        _lib.tokotachi_place_layout_add(self._h, _in_obj(widget, 'Widget'), _in_Placement(placement))
        _repin(widget)


_register(PlaceLayout)


class Action(Object):
    """C++ の tokotachi::Action(include/tokotachi/actions/action.hpp)。"""

    def __init__(self, owner=None, text=''):
        self._attach(_lib.tokotachi_action_new(_in_obj(owner, 'Object'), _in_string(text)), True)
        _repin(owner)

    text = _Property(lambda h: _out_string(_lib.tokotachi_action_get_text(h)), lambda h, v: _lib.tokotachi_action_set_text(h, _in_string(v)))
    shortcut = _Property(lambda h: _out_Shortcut(_lib.tokotachi_action_get_shortcut(h)), lambda h, v: _lib.tokotachi_action_set_shortcut(h, _in_Shortcut(v)))
    icon = _Property(lambda h: _out_Image(_lib.tokotachi_action_get_icon(h)), lambda h, v: _lib.tokotachi_action_set_icon(h, _in_Image(v)))
    tooltip = _Property(lambda h: _out_string(_lib.tokotachi_action_get_tooltip(h)), lambda h, v: _lib.tokotachi_action_set_tooltip(h, _in_string(v)))
    enabled = _Property(lambda h: _lib.tokotachi_action_get_enabled(h), lambda h, v: _lib.tokotachi_action_set_enabled(h, v))
    checkable = _Property(lambda h: _lib.tokotachi_action_get_checkable(h), lambda h, v: _lib.tokotachi_action_set_checkable(h, v))
    checked = _Property(lambda h: _lib.tokotachi_action_get_checked(h), lambda h, v: _lib.tokotachi_action_set_checked(h, v))
    on_trigger = _Handler('notify', 'tokotachi_action_set_on_trigger')

    def trigger(self):
        _lib.tokotachi_action_trigger(self._h)


_register(Action)


class Menu(Object):
    """C++ の tokotachi::Menu(include/tokotachi/actions/menu.hpp)。"""

    def __init__(self, owner=None, title=''):
        self._attach(_lib.tokotachi_menu_new(_in_obj(owner, 'Object'), _in_string(title)), True)
        _repin(owner)

    title = _Property(lambda h: _out_string(_lib.tokotachi_menu_get_title(h)), lambda h, v: _lib.tokotachi_menu_set_title(h, _in_string(v)))
    actions = _Property(lambda h: _get_objects(_lib.tokotachi_menu_get_actions, h), None)
    opened = _Property(lambda h: _lib.tokotachi_menu_get_opened(h), None)

    def add_item(self, text):
        """Action を作って最後に加える(メニューが所有する)"""
        result = _wrap(_lib.tokotachi_menu_add_item(self._h, _in_string(text)))
        _repin(result)
        return result

    def add_action(self, action):
        _lib.tokotachi_menu_add_action(self._h, _in_obj(action, 'Action'))
        _repin(action)

    def remove_action(self, action):
        _lib.tokotachi_menu_remove_action(self._h, _in_obj(action, 'Action'))
        _repin(action)

    def add_separator(self):
        _lib.tokotachi_menu_add_separator(self._h)

    def add_submenu(self, title):
        result = _wrap(_lib.tokotachi_menu_add_submenu(self._h, _in_string(title)))
        _repin(result)
        return result

    def popup(self, anchor_widget, at):
        _lib.tokotachi_menu_popup(self._h, _in_obj(anchor_widget, 'Widget'), _in_Point(at))
        _repin(anchor_widget)

    def close(self):
        _lib.tokotachi_menu_close(self._h)


_register(Menu)


class ItemModel(Object):
    """C++ の tokotachi::ItemModel(include/tokotachi/views/item_model.hpp)。一覧・木のモデルの基底。独自のモデルは tokotachi_custom_model_new で作る"""
    __init__ = _ObjectBase.__init__  # 直接は作れない


    def row_count(self, parent=None):
        result = _lib.tokotachi_item_model_row_count(self._h, _in_ModelIndex(parent))
        return result

    def index(self, row, parent=None):
        result = _out_ModelIndex(_lib.tokotachi_item_model_index(self._h, row, _in_ModelIndex(parent)))
        return result

    def parent_of(self, index):
        result = _out_ModelIndex(_lib.tokotachi_item_model_parent_of(self._h, _in_ModelIndex(index)))
        return result

    def text(self, index, column=0):
        result = _out_string(_lib.tokotachi_item_model_text(self._h, _in_ModelIndex(index), column))
        return result

    def icon(self, index):
        result = _out_Image(_lib.tokotachi_item_model_icon(self._h, _in_ModelIndex(index)))
        return result

    def enabled(self, index):
        result = _lib.tokotachi_item_model_enabled(self._h, _in_ModelIndex(index))
        return result

    def checkable(self, index):
        result = _lib.tokotachi_item_model_checkable(self._h, _in_ModelIndex(index))
        return result

    def checked(self, index):
        result = _lib.tokotachi_item_model_checked(self._h, _in_ModelIndex(index))
        return result

    def set_checked(self, index, checked):
        _lib.tokotachi_item_model_set_checked(self._h, _in_ModelIndex(index), checked)

    def text_color(self, index, column=0):
        result = _out_ThemeColor(_lib.tokotachi_item_model_text_color(self._h, _in_ModelIndex(index), column))
        return result

    def font(self, index, column=0):
        result = _out_Font(_lib.tokotachi_item_model_font(self._h, _in_ModelIndex(index), column))
        return result

    def editable(self, index, column=0):
        result = _lib.tokotachi_item_model_editable(self._h, _in_ModelIndex(index), column)
        return result

    def set_text(self, index, column, text):
        _lib.tokotachi_item_model_set_text(self._h, _in_ModelIndex(index), column, _in_string(text))

    def can_move_rows(self, indexes, parent, row):
        result = _lib.tokotachi_item_model_can_move_rows(self._h, *_in_indexes(indexes), _in_ModelIndex(parent), row)
        return result

    def move_rows(self, indexes, parent, row):
        result = _lib.tokotachi_item_model_move_rows(self._h, *_in_indexes(indexes), _in_ModelIndex(parent), row)
        return result

    def can_drop(self, data, parent, row):
        result = _lib.tokotachi_item_model_can_drop(self._h, _in_DragData(data), _in_ModelIndex(parent), row)
        return result

    def drop_data(self, data, action, parent, row):
        result = _lib.tokotachi_item_model_drop_data(self._h, _in_DragData(data), int(action), _in_ModelIndex(parent), row)
        return result

    def sort(self, column, order):
        _lib.tokotachi_item_model_sort(self._h, column, int(order))

    def locate(self, index):
        result = _out_ModelIndex(_lib.tokotachi_item_model_locate(self._h, _in_ModelIndex(index)))
        return result


_register(ItemModel)


class Item(Object):
    """C++ の tokotachi::Item(include/tokotachi/views/item.hpp)。ListModel・TreeModel の項目(モデルが所有する)"""
    __init__ = _ObjectBase.__init__  # 直接は作れない

    text = _Property(lambda h: _out_string(_lib.tokotachi_item_get_text(h)), lambda h, v: _lib.tokotachi_item_set_text(h, _in_string(v)))
    column_texts = _Property(lambda h: _out_strings(_lib.tokotachi_item_get_column_texts(h)), lambda h, v: _lib.tokotachi_item_set_column_texts(h, *_in_strings(v)))
    icon = _Property(lambda h: _out_Image(_lib.tokotachi_item_get_icon(h)), lambda h, v: _lib.tokotachi_item_set_icon(h, _in_Image(v)))
    enabled = _Property(lambda h: _lib.tokotachi_item_get_enabled(h), lambda h, v: _lib.tokotachi_item_set_enabled(h, v))
    checkable = _Property(lambda h: _lib.tokotachi_item_get_checkable(h), lambda h, v: _lib.tokotachi_item_set_checkable(h, v))
    checked = _Property(lambda h: _lib.tokotachi_item_get_checked(h), lambda h, v: _lib.tokotachi_item_set_checked(h, v))
    text_color = _Property(lambda h: _out_ThemeColor(_lib.tokotachi_item_get_text_color(h)), lambda h, v: _lib.tokotachi_item_set_text_color(h, _in_ThemeColor(v)))
    font = _Property(lambda h: _out_Font(_lib.tokotachi_item_get_font(h)), lambda h, v: _lib.tokotachi_item_set_font(h, _in_Font(v)))
    editable = _Property(lambda h: _lib.tokotachi_item_get_editable(h), lambda h, v: _lib.tokotachi_item_set_editable(h, v))

    def column_text(self, column):
        result = _out_string(_lib.tokotachi_item_column_text(self._h, column))
        return result

    def set_column_text(self, column, value):
        _lib.tokotachi_item_set_column_text(self._h, column, _in_string(value))


_register(Item)


class ListModel(ItemModel):
    """C++ の tokotachi::ListModel(include/tokotachi/views/list_model.hpp)。"""

    def __init__(self, owner=None):
        self._attach(_lib.tokotachi_list_model_new(_in_obj(owner, 'Object')), True)
        _repin(owner)

    items = _Property(lambda h: _get_objects(_lib.tokotachi_list_model_get_items, h), None)

    def add_item(self, text):
        result = _wrap(_lib.tokotachi_list_model_add_item(self._h, _in_string(text)))
        _repin(result)
        return result

    def insert_item(self, index, text):
        result = _wrap(_lib.tokotachi_list_model_insert_item(self._h, index, _in_string(text)))
        _repin(result)
        return result

    def remove_item(self, item):
        _lib.tokotachi_list_model_remove_item(self._h, _in_obj(item, 'Item'))
        _repin(item)

    def move_item(self, item, index):
        _lib.tokotachi_list_model_move_item(self._h, _in_obj(item, 'Item'), index)
        _repin(item)

    def clear(self):
        _lib.tokotachi_list_model_clear(self._h)

    def index_of(self, item):
        result = _out_ModelIndex(_lib.tokotachi_list_model_index_of(self._h, _in_obj(item, 'Item')))
        _repin(item)
        return result

    def item_at(self, index):
        result = _wrap(_lib.tokotachi_list_model_item_at(self._h, _in_ModelIndex(index)))
        _repin(result)
        return result


_register(ListModel)


class TreeItem(Item):
    """C++ の tokotachi::TreeItem(include/tokotachi/views/tree_model.hpp)。"""
    __init__ = _ObjectBase.__init__  # 直接は作れない

    parent_item = _Property(lambda h: _wrap(_lib.tokotachi_tree_item_get_parent_item(h)), None)
    children = _Property(lambda h: _get_objects(_lib.tokotachi_tree_item_get_children, h), None)

    def add_child(self, text):
        result = _wrap(_lib.tokotachi_tree_item_add_child(self._h, _in_string(text)))
        _repin(result)
        return result

    def insert_child(self, index, text):
        result = _wrap(_lib.tokotachi_tree_item_insert_child(self._h, index, _in_string(text)))
        _repin(result)
        return result


_register(TreeItem)


class TreeModel(ItemModel):
    """C++ の tokotachi::TreeModel(include/tokotachi/views/tree_model.hpp)。"""

    def __init__(self, owner=None):
        self._attach(_lib.tokotachi_tree_model_new(_in_obj(owner, 'Object')), True)
        _repin(owner)

    items = _Property(lambda h: _get_objects(_lib.tokotachi_tree_model_get_items, h), None)

    def add_item(self, text):
        result = _wrap(_lib.tokotachi_tree_model_add_item(self._h, _in_string(text)))
        _repin(result)
        return result

    def insert_item(self, index, text):
        result = _wrap(_lib.tokotachi_tree_model_insert_item(self._h, index, _in_string(text)))
        _repin(result)
        return result

    def remove_item(self, item):
        _lib.tokotachi_tree_model_remove_item(self._h, _in_obj(item, 'TreeItem'))
        _repin(item)

    def move_item(self, item, parent, index):
        _lib.tokotachi_tree_model_move_item(self._h, _in_obj(item, 'TreeItem'), _in_obj(parent, 'TreeItem'), index)
        _repin(item)
        _repin(parent)

    def clear(self):
        _lib.tokotachi_tree_model_clear(self._h)

    def index_of(self, item):
        result = _out_ModelIndex(_lib.tokotachi_tree_model_index_of(self._h, _in_obj(item, 'TreeItem')))
        _repin(item)
        return result

    def item_at(self, index):
        result = _wrap(_lib.tokotachi_tree_model_item_at(self._h, _in_ModelIndex(index)))
        _repin(result)
        return result


_register(TreeModel)


class Column(Object):
    """C++ の tokotachi::Column(include/tokotachi/views/item_view.hpp)。ItemView が作り、所有する"""
    __init__ = _ObjectBase.__init__  # 直接は作れない

    title = _Property(lambda h: _out_string(_lib.tokotachi_column_get_title(h)), lambda h, v: _lib.tokotachi_column_set_title(h, _in_string(v)))
    width = _Property(lambda h: _lib.tokotachi_column_get_width(h), lambda h, v: _lib.tokotachi_column_set_width(h, v))
    alignment = _Property(lambda h: Align(_lib.tokotachi_column_get_alignment(h)), lambda h, v: _lib.tokotachi_column_set_alignment(h, int(v)))


_register(Column)


class ItemView(Widget):
    """C++ の tokotachi::ItemView(include/tokotachi/views/item_view.hpp)。ListView・TreeView の基底(モデルは所有しない)"""
    __init__ = _ObjectBase.__init__  # 直接は作れない

    model = _Property(lambda h: _wrap(_lib.tokotachi_item_view_get_model(h)), lambda h, v: _lib.tokotachi_item_view_set_model(h, _in_obj(v, 'ItemModel')), repin=True)
    columns = _Property(lambda h: _get_objects(_lib.tokotachi_item_view_get_columns, h), None)
    header_visible = _Property(lambda h: _lib.tokotachi_item_view_get_header_visible(h), lambda h, v: _lib.tokotachi_item_view_set_header_visible(h, v))
    sorting_enabled = _Property(lambda h: _lib.tokotachi_item_view_get_sorting_enabled(h), lambda h, v: _lib.tokotachi_item_view_set_sorting_enabled(h, v))
    sort_column = _Property(lambda h: _lib.tokotachi_item_view_get_sort_column(h), lambda h, v: _lib.tokotachi_item_view_set_sort_column(h, v))
    sort_order = _Property(lambda h: SortOrder(_lib.tokotachi_item_view_get_sort_order(h)), lambda h, v: _lib.tokotachi_item_view_set_sort_order(h, int(v)))
    selection_mode = _Property(lambda h: SelectionMode(_lib.tokotachi_item_view_get_selection_mode(h)), lambda h, v: _lib.tokotachi_item_view_set_selection_mode(h, int(v)))
    current = _Property(lambda h: _out_ModelIndex(_lib.tokotachi_item_view_get_current(h)), lambda h, v: _lib.tokotachi_item_view_set_current(h, _in_ModelIndex(v)))
    selection = _Property(lambda h: _get_indexes(_lib.tokotachi_item_view_get_selection, h), None)
    editing = _Property(lambda h: _lib.tokotachi_item_view_get_editing(h), None)
    drag_enabled = _Property(lambda h: _lib.tokotachi_item_view_get_drag_enabled(h), lambda h, v: _lib.tokotachi_item_view_set_drag_enabled(h, v))
    drop_enabled = _Property(lambda h: _lib.tokotachi_item_view_get_drop_enabled(h), lambda h, v: _lib.tokotachi_item_view_set_drop_enabled(h, v))
    on_selection_change = _Handler('notify', 'tokotachi_item_view_set_on_selection_change')
    on_activate = _Handler('notify', 'tokotachi_item_view_set_on_activate')

    def add_column(self, title, width=120):
        result = _wrap(_lib.tokotachi_item_view_add_column(self._h, _in_string(title), width))
        _repin(result)
        return result

    def remove_column(self, column):
        _lib.tokotachi_item_view_remove_column(self._h, _in_obj(column, 'Column'))
        _repin(column)

    def is_selected(self, index):
        result = _lib.tokotachi_item_view_is_selected(self._h, _in_ModelIndex(index))
        return result

    def select(self, index, selected=True):
        _lib.tokotachi_item_view_select(self._h, _in_ModelIndex(index), selected)

    def select_all(self):
        _lib.tokotachi_item_view_select_all(self._h)

    def clear_selection(self):
        _lib.tokotachi_item_view_clear_selection(self._h)

    def scroll_to(self, index):
        _lib.tokotachi_item_view_scroll_to(self._h, _in_ModelIndex(index))

    def index_at(self, p):
        result = _out_ModelIndex(_lib.tokotachi_item_view_index_at(self._h, _in_Point(p)))
        return result

    def edit(self, index, column=0):
        _lib.tokotachi_item_view_edit(self._h, _in_ModelIndex(index), column)

    def finish_editing(self, commit=True):
        _lib.tokotachi_item_view_finish_editing(self._h, commit)


_register(ItemView)


class ListView(ItemView):
    """C++ の tokotachi::ListView(include/tokotachi/views/list_view.hpp)。"""

    def __init__(self, parent=None):
        self._attach(_lib.tokotachi_list_view_new(_in_obj(parent, 'Widget')), True)
        _repin(parent)



_register(ListView)


class TableView(ItemView):
    """C++ の tokotachi::TableView(include/tokotachi/views/table_view.hpp)。"""

    def __init__(self, parent=None):
        self._attach(_lib.tokotachi_table_view_new(_in_obj(parent, 'Widget')), True)
        _repin(parent)

    selection_behavior = _Property(lambda h: SelectionBehavior(_lib.tokotachi_table_view_get_selection_behavior(h)), lambda h, v: _lib.tokotachi_table_view_set_selection_behavior(h, int(v)))
    current_column = _Property(lambda h: _lib.tokotachi_table_view_get_current_column(h), lambda h, v: _lib.tokotachi_table_view_set_current_column(h, v))
    selected_ranges = _Property(lambda h: _get_ranges(_lib.tokotachi_table_view_get_selected_ranges, h), None)
    grid_visible = _Property(lambda h: _lib.tokotachi_table_view_get_grid_visible(h), lambda h, v: _lib.tokotachi_table_view_set_grid_visible(h, v))
    row_header_visible = _Property(lambda h: _lib.tokotachi_table_view_get_row_header_visible(h), lambda h, v: _lib.tokotachi_table_view_set_row_header_visible(h, v))

    def is_cell_selected(self, row, column):
        result = _lib.tokotachi_table_view_is_cell_selected(self._h, row, column)
        return result

    def select_range(self, range, add=False):
        _lib.tokotachi_table_view_select_range(self._h, _in_CellRange(range), add)

    def selected_text(self):
        result = _out_string(_lib.tokotachi_table_view_selected_text(self._h))
        return result

    def copy(self):
        _lib.tokotachi_table_view_copy(self._h)

    def paste(self):
        _lib.tokotachi_table_view_paste(self._h)

    def clear_selected_cells(self):
        _lib.tokotachi_table_view_clear_selected_cells(self._h)


_register(TableView)


class TreeView(ItemView):
    """C++ の tokotachi::TreeView(include/tokotachi/views/tree_view.hpp)。"""

    def __init__(self, parent=None):
        self._attach(_lib.tokotachi_tree_view_new(_in_obj(parent, 'Widget')), True)
        _repin(parent)

    indentation = _Property(lambda h: _lib.tokotachi_tree_view_get_indentation(h), lambda h, v: _lib.tokotachi_tree_view_set_indentation(h, v))

    def expand(self, index):
        _lib.tokotachi_tree_view_expand(self._h, _in_ModelIndex(index))

    def collapse(self, index):
        _lib.tokotachi_tree_view_collapse(self._h, _in_ModelIndex(index))

    def is_expanded(self, index):
        result = _lib.tokotachi_tree_view_is_expanded(self._h, _in_ModelIndex(index))
        return result

    def expand_all(self):
        _lib.tokotachi_tree_view_expand_all(self._h)

    def collapse_all(self):
        _lib.tokotachi_tree_view_collapse_all(self._h)


_register(TreeView)


class Variable(_VariableMixin, Object):
    """C++ の tokotachi::Object(include/tokotachi/core/variable.hpp)。値を 1 つ持つオブジェクト(docs/adr/0016)。プロパティ "value" を tokotachi_bind で結ぶ。 型と合わない get・set は失敗する"""
    pass


_register(Variable)


class Task(_TaskMixin, Object):
    """C++ の tokotachi::TaskBase(include/tokotachi/core/task.hpp)。別のスレッドで行う、終わりのある仕事(docs/adr/0029)"""
    running = _Property(lambda h: _lib.tokotachi_task_get_running(h), None)
    progress = _Property(lambda h: _lib.tokotachi_task_get_progress(h), None)
    cancelled = _Property(lambda h: _lib.tokotachi_task_get_cancelled(h), None)
    error = _Property(lambda h: _out_string(_lib.tokotachi_task_get_error(h)), None)
    on_progress = _Handler('notify', 'tokotachi_task_set_on_progress')
    on_finish = _Handler('notify', 'tokotachi_task_set_on_finish')

    def cancel(self):
        _lib.tokotachi_task_cancel(self._h)


_register(Task)


class CustomModel(_CustomModelMixin, ItemModel):
    """C++ の tokotachi::ItemModel(include/tokotachi/views/item_model.hpp)。関数で中身を答えるモデル(C++ では ItemModel を継承して作るもの)"""
    pass


_register(CustomModel)

__all__ = ['MouseButton', 'Key', 'FocusPolicy', 'MouseCursor', 'Orientation', 'Align', 'Anchor', 'ColorScheme', 'ColorRole', 'ButtonStyle', 'DialogButton', 'FileDialogMode', 'MessageType', 'PopupKind', 'PopupPlacement', 'ToolButtonStyle', 'SelectionMode', 'SortOrder', 'LineCap', 'LineJoin', 'PaintType', 'DayOfWeek', 'DropAction', 'SelectionBehavior', 'ImageFit', 'Icon', 'VariableType', 'Object', 'Application', 'Timer', 'Widget', 'ButtonBase', 'Button', 'CheckBox', 'RadioButton', 'ComboBox', 'Label', 'TextInputBase', 'LineEdit', 'TextEdit', 'PaintBox', 'ProgressBar', 'RangeBase', 'Slider', 'SpinBox', 'ScrollArea', 'ScrollBar', 'SplitterPane', 'Splitter', 'Tab', 'TabWidget', 'GroupBox', 'ImageBox', 'Calendar', 'DateEdit', 'ColorButton', 'Window', 'Dialog', 'MessageDialog', 'ColorDialog', 'FileDialog', 'MenuBar', 'Popup', 'StatusBar', 'ToolBar', 'Layout', 'BoxLayout', 'GridLayout', 'PlaceLayout', 'Action', 'Menu', 'ItemModel', 'Item', 'ListModel', 'TreeItem', 'TreeModel', 'Column', 'ItemView', 'ListView', 'TableView', 'TreeView', 'Variable', 'Task', 'CustomModel']
