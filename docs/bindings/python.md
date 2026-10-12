# Python のバインディング

Tokotachi を Python から使う([ADR 0065](../adr/0065-c-api-and-python-bindings.md))。パッケージ `tokotachi`
([bindings/python/](../../bindings/python/))は、C の API([c-api.md](c-api.md))の共有ライブラリ `tokotachi_c` を ctypes で読む。
Python 3.9 以上。

## インストール

リポジトリの中から pip で入れる。C の API と依存ライブラリを CMake でビルドしてパッケージに入れる(初めては数分かかる。
ビルドに要るものは [README](../../README.md) の「ビルド」と同じ)。

```bash
pip install ./bindings/python
```

ビルド済みの C の API を使うときは、パッケージのソースを `PYTHONPATH` に入れ、環境変数 `TOKOTACHI_C_LIBRARY` に共有ライブラリのパスを入れる。

```bash
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release && cmake --build build
export TOKOTACHI_C_LIBRARY=$PWD/build/bindings/c/libtokotachi_c.so
export PYTHONPATH=$PWD/bindings/python/src
python3 bindings/python/examples/hello.py
```

## 例

```python
import tokotachi as tt

app = tt.Application()
window = tt.Window('hello', 400, 240)

layout = tt.BoxLayout(window)
layout.margins = 24
title = tt.Label(window, 'こんにちは')
title.font = 22
edit = tt.LineEdit(window)
greeting = tt.Label(window)
button = tt.Button(window, '閉じる')
for w in (title, edit, greeting):
    layout.add(w)
layout.add(button, 0, tt.Align.END)

name = tt.Variable(window, '世界')           # 値の型は値から決める(str)
tt.bind(edit, 'text', name, 'value')          # 入力欄と Variable を結ぶ
tt.bind(greeting, 'text', name, 'value')
button.on_click = lambda sender: window.close()

app.main_window = window                      # 閉じたら終える
window.visible = True
app.exec()
```

## C++ との対応

名前は C++ と同じ。

| C++ | Python |
|---|---|
| `new tokotachi::Label(window, "abc")` | `tt.Label(window, 'abc')`(親・所有者は省略できる) |
| `label->text = "abc"` / `std::string s = label->text` | `label.text = 'abc'` / `s = label.text`(読み取り専用に代入すると AttributeError) |
| `button->on_click = [](tokotachi::Object* sender) { ... }` | `button.on_click = lambda sender: ...`(None で外す) |
| `on_mouse_down`(`MouseEvent& e`) | `def f(sender, e)`。`e.pos`・`e.button`・`e.accepted = False` など |
| `on_paint`(`Canvas& canvas`) | `def f(sender, canvas)`。canvas はハンドラの中だけで使える |
| `on_close`(`bool& can_close`) | `def f(sender, e)`。`e.can_close = False` で閉じない |
| `tokotachi::Rect(0, 0, 10, 10)`・`Color::rgb(0x336699)` | `tt.Rect(0, 0, 10, 10)`(タプルも可)・`tt.Color.rgb(0x336699)` |
| `label->color = ColorRole::TEXT_SECONDARY` | `label.text_color = tt.ColorRole.TEXT_SECONDARY`(Color も可) |
| `label->font = Font(22)`・`layout->margins = Margins(24)` | `label.font = 22`・`layout.margins = 24`(数でもよい) |
| `action->shortcut = "Ctrl+S"` | `action.shortcut = 'Ctrl+S'` |
| `tokotachi::bind(edit->text, name->value)` | `tt.bind(edit, 'text', name, 'value')` |
| `Variable<std::string>`・`Variable<bool>` | `tt.Variable(owner, '初期値')`・`tt.Variable(owner, type=bool)`(float は C++ の double) |
| `Task<T>`・`start([](TaskContext& ctx) { ... })` | `task = tt.Task(owner)`・`task.start(lambda ctx: ...)`(戻り値は `task.result`、例外は `task.error`) |
| `tokotachi::post(fn)` | `tt.post(fn)`(どのスレッドからでも) |
| `ItemModel` を継承したモデル | `tt.CustomModel` を継承して `row_count(parent)`・`text(index, column)` を上書きする(木なら `index_id`・`parent_of`、編集なら `editable`・`set_text`、ドラッグなら `can_move_rows`・`move_rows`・`can_drop`・`drop_data` も) |
| `Image::load`・`Bitmap`・`Path`・`TextLayout` | `tt.Image.load`・`tt.Bitmap`・`tt.Path`・`tt.TextLayout` |
| `Paint::linear(...)`・`StrokeStyle(2, LineCap::ROUND)` | `tt.Paint.linear(start, end, [(0, c0), (1, c1)])`・`tt.StrokeStyle(2, tt.LineCap.ROUND)`。塗りには Color もそのまま渡せる |
| `Image::icon(Icon::SAVE)` | `tt.Image.icon(tt.Icon.SAVE)`(`tt.Image.icon(tt.Icon.SAVE, tt.Color(...))` で色を決める) |
| `Font(16, true)` | `tt.Font(16, bold=True)` |
| `Date(2026, 10, 12)`・`Date::parse("2026/10/12")` | `tt.Date(2026, 10, 12)`・`tt.Date.parse('2026/10/12')`(`add_days`・`format` なども) |
| `ThemeOverride().set(ColorRole::ACCENT, c)` | `tt.ThemeOverride().set(tt.ColorRole.ACCENT, c)` |
| `on_drop`(`DragEvent& e`) | `def f(sender, e)`。`e.data.text`・`e.data.files`・`e.action`・`e.accepted = False`(`e.data` はハンドラの中だけ) |
| `start_drag(DragData)` | `widget.start_drag(tt.DragData('文字', files=[...]))` |
| `tokotachi::draw::button_face(canvas, r, state)` | `canvas.draw_button_face(r, tt.ControlState(hovered=True))` |

## 寿命

- C++ の Object 1 つに、Python の Object は 1 つ(`label.parent is window`)。
- **所有者のある Object**(親のあるウィジェット・レイアウト・所有者を渡した Timer など)は、C++ の所有者が寿命を決める。Python の変数が
  無くなっても残り、`window.children` などから同じ Python の Object(付けた属性ごと)が返る。
- **ウィンドウ**は、所有者を渡さなければ Application が、`tt.Window(owner, '題', 360, 120)` なら owner が所有する
  ([ADR 0062](../adr/0062-window-ownership-and-quit.md))。Python の変数が無くなっても残る。閉じた後に破棄するときは
  `destroy_on_close = True`、閉じたら終えるウィンドウは `app.main_window` に入れる。
- **Python で作った所有者の無い Object**(親の無いウィジェット・所有者を渡さない Timer など)は、Python の Object が無くなったときに
  破棄する(PyQt と同じ)。
- `obj.delete()` ですぐに破棄する。C++ の Object が破棄された後に触ると `RuntimeError`(`obj.is_deleted` で確かめられる)。
- Python が終わるときに、所有者の無い Object を破棄してから Application を破棄する(残っているウィンドウも一緒に破棄される)。

## 失敗とスレッド

- C の API の失敗(型の違う Object・範囲の外の値など)は `tt.TokotachiError`。
- ハンドラの中で起きた例外は `sys.excepthook` で表示し、イベントの処理は続ける。
- Object・ウィジェットはメインスレッド(Application を作ったスレッド)だけで扱う。別のスレッドからは `tt.post(fn)` で頼む。
  `app.exec()` の間は GIL を放すので、Python のスレッドも動く。

## テスト

```bash
SDL_VIDEO_DRIVER=offscreen TOKOTACHI_C_LIBRARY=build/bindings/c/libtokotachi_c.so PYTHONPATH=bindings/python/src \
    python3 -m unittest discover -s bindings/python/tests -v
```

`ctest` からも動く(`tokotachi_python_tests`)。クラス・プロパティの宣言(`_api.py`)は C の API と同じ仕様から作る
([c-api.md](c-api.md) の「API を変えるとき」)。
