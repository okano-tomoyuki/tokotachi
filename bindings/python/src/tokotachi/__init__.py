"""Tokotachi(SDL3 を土台にした GUI ライブラリ)の Python のバインディング(docs/bindings/python.md)。

C API の共有ライブラリ(tokotachi_c)を ctypes で読む。クラス・プロパティ・ハンドラの名前は C++ と同じ:

>>> import tokotachi as tt
>>> app = tt.Application()
>>> window = tt.Window('hello', 320, 200)
>>> button = tt.Button(window, 'OK')
>>> button.on_click = lambda sender: app.quit()
>>> window.visible = True
>>> app.exec()
"""

from ._core import (Bitmap, Canvas, CellRange, CloseEvent, Color, ControlState, Date, DragData, DragEvent, Font,
                    Image, KeyEvent, Margins, ModelIndex, Modifiers, MouseEvent, Paint, Path, Placement, Point,
                    Rect, Shortcut, Size, StrokeStyle, TaskContext, TextLayout, Theme, ThemeOverride,
                    TokotachiError, WheelEvent, bind, button_text_color,
                    clipboard, is_main_thread, post, resolve_color, unbind,
                    unbind_all, version)
from ._api import *  # noqa: F401,F403
from ._api import __all__ as _api_all

__version__ = version()

__all__ = ['Bitmap', 'Canvas', 'CellRange', 'CloseEvent', 'Color', 'ControlState', 'Date', 'DragData', 'DragEvent',
           'Font', 'Image', 'KeyEvent', 'Margins', 'ModelIndex', 'Modifiers', 'MouseEvent', 'Paint', 'Path',
           'Placement', 'Point', 'Rect', 'Shortcut', 'Size', 'StrokeStyle', 'TaskContext', 'TextLayout', 'Theme',
           'ThemeOverride', 'TokotachiError', 'WheelEvent', 'bind', 'button_text_color', 'clipboard', 'is_main_thread', 'post',
           'resolve_color', 'unbind', 'unbind_all', 'version'] + list(_api_all)
