"""Python のバインディングのテスト(画面なしで動かす: SDL_VIDEO_DRIVER=offscreen)。

  TOKOTACHI_C_LIBRARY=build/bindings/c/libtokotachi_c.so PYTHONPATH=bindings/python/src \\
      python3 -m unittest discover -s bindings/python/tests
"""

import ctypes
import gc
import os
import tempfile
import threading
import unittest

os.environ.setdefault('SDL_VIDEO_DRIVER', 'offscreen')

import tokotachi as tt  # noqa: E402
from tokotachi import _core  # noqa: E402

app = tt.Application(['test'])


class TestBasics(unittest.TestCase):
    def setUp(self):
        self.window = tt.Window('test', 400, 300)

    def tearDown(self):
        self.window.delete()

    def test_version_and_structs(self):
        self.assertRegex(tt.version(), r'^\d+\.\d+\.\d+$')
        # ctypes の構造体が C と同じ大きさ
        for name in ['Point', 'Size', 'Rect', 'Color', 'Margins', 'Font', 'Shortcut', 'ThemeColor', 'ModelIndex',
                     'Placement', 'Theme', 'MouseEvent', 'KeyEvent', 'WheelEvent', 'GradientStop', 'Paint',
                     'StrokeStyle', 'ModelCallbacks']:
            c_name = 'tokotachi_' + ''.join('_' + ch.lower() if ch.isupper() else ch for ch in name).lstrip('_')
            size = _core._lib.tokotachi_struct_size(c_name.encode())
            self.assertEqual(size, ctypes.sizeof(getattr(_core, 'C' + name)), name)

    def test_application(self):
        self.assertIs(tt.Application.instance(), app)
        self.assertTrue(tt.is_main_thread())
        self.assertIsInstance(app.theme, tt.Theme)

    def test_properties(self):
        label = tt.Label(self.window, 'こんにちは')
        self.assertEqual(label.text, 'こんにちは')
        label.text = 'abc'
        self.assertEqual(label.text, 'abc')
        self.assertIs(label.parent, self.window)
        self.assertIs(label.window, self.window)
        self.assertIn(label, self.window.children)
        label.bounds = (10, 20, 100, 30)
        self.assertEqual(label.bounds, tt.Rect(10, 20, 100, 30))
        label.x += 5
        self.assertEqual(label.x, 15)
        label.font = 22
        self.assertEqual(label.font, tt.Font(22))
        label.text_color = tt.ColorRole.TEXT_SECONDARY
        self.assertIs(label.text_color, tt.ColorRole.TEXT_SECONDARY)
        label.text_color = tt.Color.rgb(0x336699)
        self.assertEqual(label.text_color, tt.Color(0x33, 0x66, 0x99))
        label.alignment = tt.Align.CENTER
        self.assertIs(label.alignment, tt.Align.CENTER)
        with self.assertRaises(AttributeError):
            label.showing = True

        combo = tt.ComboBox(self.window)
        combo.items = ['赤', '緑', '青']
        combo.current_index = 2
        self.assertEqual(combo.items, ['赤', '緑', '青'])
        self.assertEqual(combo.current_text, '青')

        spin = tt.SpinBox(self.window)
        spin.maximum = 10
        spin.value = 20
        self.assertEqual(spin.value, 10)

        edit = tt.LineEdit(self.window, 'hello')
        edit.cursor_position = 2
        self.assertEqual(edit.cursor_position, 2)
        edit.select_all()
        self.assertEqual(edit.selected_text, 'hello')

        dialog = tt.FileDialog(self.window, tt.FileDialogMode.SAVE)
        dialog.filters = ['画像', 'png;jpg', 'すべて', '*']
        self.assertEqual(dialog.filters, ['画像', 'png;jpg', 'すべて', '*'])
        self.assertEqual(dialog.paths, [])

        action = tt.Action(self.window, '保存')
        action.shortcut = 'Ctrl+S'
        self.assertEqual(action.shortcut, tt.Shortcut(tt.Key.S, tt.Modifiers.CONTROL))
        self.assertEqual(str(action.shortcut), 'Ctrl+S')

    def test_errors(self):
        label = tt.Label(self.window)
        with self.assertRaises(tt.TokotachiError):
            label.alignment = 99
        with self.assertRaises(TypeError):
            tt.Label(tt.Timer())  # parent は Widget
        with self.assertRaises(TypeError):
            tt.ButtonBase()

    def test_handlers(self):
        action = tt.Action(self.window, 'a')
        calls = []
        action.on_trigger = lambda sender: calls.append(sender)
        action.trigger()
        self.assertEqual(calls, [action])
        action.checkable = True
        action.on_trigger = None
        action.trigger()
        self.assertEqual(len(calls), 1)
        self.assertTrue(action.checked)
        # 置き換え・Object の破棄でハンドラを放す
        before = len(_core._callbacks)
        action.on_trigger = lambda sender: None
        action.on_trigger = lambda sender: None
        self.assertEqual(len(_core._callbacks), before + 1)
        action.delete()
        self.assertEqual(len(_core._callbacks), before)

    def test_handler_exception_is_reported(self):
        action = tt.Action(self.window, 'a')
        reported = []
        hook = _core.sys.excepthook
        _core.sys.excepthook = lambda *exc: reported.append(exc[0])
        try:
            action.on_trigger = lambda sender: 1 / 0
            action.trigger()
        finally:
            _core.sys.excepthook = hook
        self.assertEqual(reported, [ZeroDivisionError])

    def test_lifetime(self):
        label = tt.Label(self.window, 'x')
        address = label._ptr
        del label
        gc.collect()
        # 所有者のあるものは、Python の参照が無くなっても残り、同じ Python の Object が返る
        children = self.window.children
        self.assertEqual(children[0]._ptr, address)
        label = children[0]
        label.custom = 1
        del children, label
        gc.collect()
        self.assertEqual(self.window.children[0].custom, 1)

        # 所有者を破棄すると、子は破棄済みになる
        window = tt.Window('w')
        child = tt.Label(window)
        window.delete()
        self.assertTrue(window.is_deleted)
        self.assertTrue(child.is_deleted)
        with self.assertRaises(RuntimeError):
            child.text = 'x'

        # Python で作った所有者の無いものは、参照が無くなると破棄する(所有されているものも一緒に)
        timer = tt.Timer()
        owned = tt.Action(timer, 'owned')
        del timer
        gc.collect()
        self.assertTrue(owned.is_deleted)

        # 所有者ができたものは、参照が無くなっても破棄しない
        lonely = tt.Label(None, 'lonely')
        tt.BoxLayout(self.window).add(lonely)
        address = lonely._ptr
        del lonely
        gc.collect()
        self.assertIn(address, [w._ptr for w in self.window.children])

        # 親を外したウィジェットは Python が持つ
        widget = tt.Widget(self.window)
        widget.parent = None
        self.assertNotIn(widget._ptr, _core._pinned)
        widget.parent = self.window
        self.assertIn(widget._ptr, _core._pinned)

    def test_window_ownership(self):
        # 所有者を渡さないウィンドウは Application が所有し、参照が無くなっても残る(docs/adr/0062)
        main = tt.Window('main', 200, 100)
        self.assertIs(main.owner, app)
        address = main._ptr
        del main
        gc.collect()
        main = _core._wrap(address)
        self.assertFalse(main.is_deleted)
        # 所有者のウィンドウを渡すと、それが所有する
        child = tt.Window(main, 'child', 100, 50)
        self.assertIs(child.owner, main)
        self.assertEqual(child.title, 'child')
        app.main_window = main
        self.assertIs(app.main_window, main)
        main.destroy_on_close = True
        self.assertTrue(main.destroy_on_close)
        main.delete()
        self.assertTrue(child.is_deleted)
        self.assertIsNone(app.main_window)

    def test_bind(self):
        name = tt.Variable(self.window, 'はじめ')
        edit = tt.LineEdit(self.window)
        label = tt.Label(self.window)
        self.assertIs(name.variable_type, tt.VariableType.STRING)
        tt.bind(edit, 'text', name, 'value')
        tt.bind(label, 'text', name, 'value')
        self.assertEqual(label.text, 'はじめ')
        edit.text = 'ゆき'
        self.assertEqual(label.text, 'ゆき')
        self.assertEqual(name.value, 'ゆき')
        changes = []
        name.on_change = lambda sender: changes.append(sender.value)
        name.value = 'はな'
        self.assertEqual(changes, ['はな'])
        self.assertEqual(edit.text, 'はな')
        tt.unbind_all(label, 'text')
        name.value = 'つき'
        self.assertEqual(label.text, 'はな')

        flag = tt.Variable(self.window, type=bool)
        with self.assertRaises(tt.TokotachiError):
            tt.bind(label, 'text', flag, 'value')
        check_box = tt.CheckBox(self.window)
        tt.bind(check_box, 'checked', flag, 'value')
        flag.value = True
        self.assertTrue(check_box.checked)

        number = tt.Variable(self.window, 1.5)
        slider = tt.Slider(self.window)
        tt.bind(slider, 'value', number, 'value')
        self.assertEqual(slider.value, 1.5)
        # Variable を C のハンドルから包み直しても型が分かる
        self.assertIs(tt._core._wrap(number._ptr), number)

    def test_layout(self):
        box = tt.BoxLayout(self.window)
        box.margins = 10
        a = tt.Label(self.window, 'a')
        b = tt.Label(self.window, 'b')
        box.add(a)
        box.add(b, 1)
        self.window.apply_layout()
        self.assertIs(self.window.layout, box)
        self.assertEqual(a.x, 10)
        self.assertAlmostEqual(a.width, 380)
        self.assertGreater(b.y, a.y)

        grid_owner = tt.Widget(self.window)
        grid = tt.GridLayout(grid_owner)
        grid.add(tt.Label(None, 'x'), 0, 1)
        self.assertEqual(len(grid_owner.children), 1)

    def test_containers(self):
        tabs = tt.TabWidget(self.window)
        page = tt.Widget()
        tab = tabs.add_tab(page, 'ページ')
        self.assertEqual(tab.title, 'ページ')
        self.assertIs(tab.page, page)
        self.assertIs(tabs.current_tab, tab)
        removed = tabs.remove_tab(tab)
        self.assertIs(removed, page)
        self.assertIsNone(page.parent)
        page.delete()

        splitter = tt.Splitter(self.window)
        pane = splitter.add_widget(tt.Widget(), 2)
        self.assertEqual(pane.stretch, 2)
        self.assertEqual(len(splitter.sizes), 1)

        scroll = tt.ScrollArea(self.window)
        content = tt.Widget()
        scroll.content = content
        self.assertIs(scroll.content, content)

        menu_bar = tt.MenuBar()
        self.window.menu_bar = menu_bar
        menu = menu_bar.add_menu('ファイル')
        item = menu.add_item('開く')
        self.assertEqual(menu.actions, [item])
        self.assertIs(self.window.menu_bar, menu_bar)


class TestViews(unittest.TestCase):
    def setUp(self):
        self.window = tt.Window('views', 400, 300)

    def tearDown(self):
        self.window.delete()

    def test_list_and_tree_models(self):
        model = tt.ListModel(self.window)
        a = model.add_item('a')
        model.add_item('b')
        a.column_texts = ['1 KB']
        self.assertEqual(a.column_text(1), '1 KB')
        self.assertEqual(model.items[0], a)
        view = tt.ListView(self.window)
        view.model = model
        view.add_column('名前', 200)
        self.assertEqual([c.title for c in view.columns], ['名前'])
        second = model.index(1)
        self.assertEqual(model.text(second), 'b')
        view.current = second
        self.assertEqual(view.selection, [second])
        self.assertEqual(model.item_at(second).text, 'b')

        tree = tt.TreeModel(self.window)
        root = tree.add_item('root')
        child = root.add_child('child')
        self.assertIs(child.parent_item, root)
        self.assertEqual(tree.row_count(tree.index_of(root)), 1)
        tree_view = tt.TreeView(self.window)
        tree_view.model = tree
        tree_view.expand(tree.index_of(root))
        self.assertTrue(tree_view.is_expanded(tree.index_of(root)))

    def test_custom_model(self):
        nodes = {1: ('果物', [2, 3]), 2: ('りんご', []), 3: ('みかん', [])}
        parents = {2: 1, 3: 1}

        class Fruits(tt.CustomModel):
            def row_count(self, parent):
                return 1 if not parent.is_valid() else len(nodes[parent.id][1])

            def index_id(self, row, parent):
                return 1 if not parent.is_valid() else nodes[parent.id][1][row]

            def parent_of(self, index):
                p = parents.get(index.id)
                return tt.ModelIndex(0, p) if p else None

            def text(self, index, column):
                return nodes[index.id][0]

            def checkable(self, index):
                return True

        model = Fruits(self.window)
        root = model.index(0)
        self.assertEqual(model.text(root, 0), '果物')  # Python のメソッド
        self.assertEqual(_core._out_string(_core._lib.tokotachi_item_model_text(model._ptr, _core._in_ModelIndex(root), 0)),
                         '果物')  # C++ から呼んだとき
        self.assertEqual(_core._lib.tokotachi_item_model_row_count(model._ptr, _core._in_ModelIndex(root)), 2)
        child = _core._out_ModelIndex(_core._lib.tokotachi_item_model_index(model._ptr, 1, _core._in_ModelIndex(root)))
        self.assertEqual(child.id, 3)
        parent = _core._out_ModelIndex(_core._lib.tokotachi_item_model_parent_of(model._ptr, _core._in_ModelIndex(child)))
        self.assertEqual(parent, root)
        self.assertTrue(_core._lib.tokotachi_item_model_checkable(model._ptr, _core._in_ModelIndex(child)))
        # 上書きしていないものは既定の答え
        self.assertTrue(_core._lib.tokotachi_item_model_enabled(model._ptr, _core._in_ModelIndex(child)))
        view = tt.TreeView(self.window)
        view.model = model
        view.expand_all()
        model.notify_reset()
        self.assertIs(view.model, model)

        with self.assertRaises(TypeError):
            class Broken(tt.CustomModel):
                def row_count(self, parent):
                    return 0
            Broken()


class TestDrawing(unittest.TestCase):
    def test_paint_handler_and_capture(self):
        window = tt.Window('drawing', 120, 80)
        box = tt.PaintBox(window)
        box.bounds = (10, 10, 40, 40)
        seen = []

        def paint(sender, canvas):
            seen.append(canvas)
            canvas.fill_rect((0, 0, 40, 40), tt.Color(255, 0, 0))
            canvas.fill_ellipse((10, 10, 20, 20), tt.Color(0, 0, 255))

        box.on_paint = paint
        window.visible = True
        shot = window.capture()
        s = shot.scale
        self.assertEqual(shot.pixel(int(12 * s), int(12 * s)), tt.Color(255, 0, 0))
        self.assertEqual(shot.pixel(int(30 * s), int(30 * s)), tt.Color(0, 0, 255))
        with self.assertRaises(RuntimeError):
            seen[0].fill_rect((0, 0, 1, 1), tt.Color())  # ハンドラの外では使えない
        with tempfile.TemporaryDirectory() as d:
            path = os.path.join(d, 'shot.png')
            self.assertTrue(shot.save_png(path))
            image = tt.Image.load(path)
            self.assertEqual((image.width, image.height), (shot.width, shot.height))
            self.assertEqual(image.pixel(int(12 * s), int(12 * s)), tt.Color(255, 0, 0))
        window.delete()

    def test_bitmap(self):
        bitmap = tt.Bitmap(64, 32)
        bitmap.clear(tt.Color(255, 255, 255))
        canvas = bitmap.canvas
        canvas.fill_round_rect((0, 0, 64, 32), 0,
                               tt.Paint.linear((0, 0), (64, 0), [(0, tt.Color(0, 0, 255)), (1, tt.Color(0, 255, 0))]))
        self.assertGreater(bitmap.pixel(1, 16).b, 200)
        self.assertGreater(bitmap.pixel(62, 16).g, 200)
        path = tt.Path().move_to(0, 0).line_to(64, 32)
        canvas.stroke_path(path, tt.StrokeStyle(3, tt.LineCap.ROUND, dashes=[4, 2]), tt.Color(0, 0, 0))
        layout = tt.TextLayout('Tokotachi', 14)
        self.assertGreater(layout.width, 10)
        self.assertEqual(layout.line_count, 1)
        canvas.draw_text(layout, (0, 0), tt.Color(0, 0, 0))
        canvas.draw_text('こんにちは', (0, 0), tt.Color(0, 0, 0), 12)
        pixels = bitmap.pixels()
        self.assertEqual(len(pixels), 64 * 32)
        image = bitmap.to_image()
        self.assertEqual(image, image)
        icon = tt.Image.from_pixels(2, 1, [tt.Color(255, 0, 0), tt.Color(0, 255, 0)])
        self.assertEqual(icon.pixel(1, 0), tt.Color(0, 255, 0))
        self.assertIsNone(tt.Image.from_pixels(2, 2, [tt.Color()]))
        self.assertIsNone(tt.Image.load('/no/such/file.png'))
        action = tt.Action(None, 'a')
        action.icon = icon
        self.assertEqual(action.icon, icon)
        action.icon = None
        self.assertIsNone(action.icon)
        action.delete()

    def test_theme(self):
        light = tt.Theme.light()
        dark = tt.Theme.dark()
        self.assertTrue(dark.dark_colors)
        light[tt.ColorRole.ACCENT] = tt.Color.rgb(0x059669)
        app.light_theme = light
        self.assertEqual(app.light_theme[tt.ColorRole.ACCENT], tt.Color.rgb(0x059669))
        app.color_scheme = tt.ColorScheme.DARK
        self.assertTrue(app.dark)
        self.assertEqual(app.theme, app.dark_theme)
        self.assertEqual(tt.resolve_color(tt.ColorRole.WINDOW), app.dark_theme[tt.ColorRole.WINDOW])
        app.color_scheme = tt.ColorScheme.LIGHT
        app.light_theme = tt.Theme.light()


class TestEventLoop(unittest.TestCase):
    def test_post_task_and_timer(self):
        window = tt.Window('loop', 100, 100)
        window.visible = True
        events = []

        def worker():
            tt.post(lambda: events.append(('post', tt.is_main_thread())))

        thread = threading.Thread(target=worker)
        thread.start()
        thread.join()

        task = tt.Task(window)

        def finished(sender):
            events.append(('task', sender.result, sender.error))
            failing = tt.Task(window)
            failing.on_finish = lambda t: (events.append(('fail', t.error)), app.quit(3))
            failing.start(lambda ctx: 1 / 0)

        task.on_finish = finished
        self.assertTrue(task.start(lambda ctx: sum(range(10))))

        timeout = tt.Timer(window)
        timeout.interval = 5000
        timeout.on_timeout = lambda sender: (events.append(('timeout',)), app.quit(1))
        timeout.enabled = True
        self.assertEqual(app.exec(), 3)
        self.assertIn(('post', True), events)
        self.assertIn(('task', 45, ''), events)
        self.assertIn(('fail', 'ZeroDivisionError: division by zero'), events)
        window.delete()

    def test_close_handler(self):
        window = tt.Window('close', 100, 100)
        window.visible = True
        asked = []

        def on_close(sender, e):
            asked.append(sender)
            e.can_close = False

        window.on_close = on_close
        # 閉じるボタンの代わりに on_close を直接呼ぶ手段は無いので、ハンドラの設定と取得だけ確かめる
        self.assertIs(window.on_close, on_close)
        window.delete()


if __name__ == '__main__':
    unittest.main()
