# C の API

Tokotachi を C から使うための API([ADR 0041](../adr/0041-c-api-and-python-bindings.md))。他の言語のバインディングの土台でもある
(Python は [python.md](python.md))。

- ヘッダ: [bindings/c/include/tokotachi/tokotachi.h](../../bindings/c/include/tokotachi/tokotachi.h)(`#include <tokotachi/tokotachi.h>`)
- ライブラリ: `tokotachi_c`(共有ライブラリ。ライブラリ本体と依存ライブラリを中に入れ、`tokotachi_*` の関数だけを公開する)
- 意味は C++ の API と同じ。各関数の説明は、C++ のヘッダ(`include/tokotachi/` の同じ名前のクラス)を見る。

## ビルドと使い方

`TOKOTACHI_BUILD_C_API`(単独でビルドするときは ON)で作られる。

```bash
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build                         # build/bindings/c/libtokotachi_c.so
ctest --test-dir build --output-on-failure  # C と Python のテストも動く
cmake --install build --prefix <prefix>     # include/tokotachi/tokotachi.h・lib/libtokotachi_c.so・tokotachi_c.pc
```

```cmake
find_package(tokotachi 0.1 REQUIRED)
target_link_libraries(app PRIVATE tokotachi::tokotachi_c)
```

```bash
cc app.c $(pkg-config --cflags --libs tokotachi_c)
```

## 例

```c
#include <tokotachi/tokotachi.h>

static void on_click(tokotachi_object* sender, void* user_data)
{
    (void)sender;
    tokotachi_window_close((tokotachi_window*)user_data);
}

int main(int argc, char** argv)
{
    tokotachi_application* app = tokotachi_application_new(argc, argv);
    tokotachi_window* window = tokotachi_window_new("hello", 400, 200);

    tokotachi_box_layout* layout = tokotachi_box_layout_new(window, TOKOTACHI_ORIENTATION_VERTICAL);
    tokotachi_label* label = tokotachi_label_new(window, "こんにちは");
    tokotachi_button* button = tokotachi_button_new(window, "閉じる");
    tokotachi_box_layout_add(layout, label, 0, TOKOTACHI_ALIGN_FILL);
    tokotachi_box_layout_add(layout, button, 0, TOKOTACHI_ALIGN_END);
    tokotachi_button_base_set_on_click(button, on_click, window, NULL);  /* Button の on_click は ButtonBase のもの */

    tokotachi_widget_set_visible(window, true);
    int code = tokotachi_application_exec(app);
    tokotachi_object_delete(window);  /* 所有者の無いものは利用者が破棄する(子も一緒に破棄される) */
    tokotachi_object_delete(app);     /* Application は最後 */
    return code;
}
```

## 決まり

| 項目 | 決まり |
|---|---|
| 名前 | `tokotachi_<クラス>_<操作>`。プロパティは `_get_<名前>`・`_set_<名前>`(読み取り専用は get だけ)、ハンドラは `_set_on_xxx`、コンストラクタは `_new`。C++ の引数の既定値は無いので、すべて渡す |
| ハンドル | Object の派生はすべて `tokotachi_object*`(`tokotachi_label` などは別名)。基底のクラスの関数(`tokotachi_widget_set_x` など)に派生のものをそのまま渡せる。型の違うものを渡すと何もせずに失敗する |
| 所有 | C++ と同じ([ADR 0015](../adr/0015-object-ownership.md))。所有者のあるものは所有者と一緒に破棄される。所有者の無いものは `tokotachi_object_delete` で破棄する。Application は最後に破棄する |
| 値の型 | `tokotachi_point`・`tokotachi_rect`・`tokotachi_color` などの構造体を値で渡す。`tokotachi_theme_color` は役割(`is_role`)か具体的な色、`tokotachi_model_index` は行(負なら無効)と識別子 |
| 文字列 | UTF-8。返された `char*` は `tokotachi_free` で、`char**`(NULL で終わる並び)は `tokotachi_strv_free` で解放する。渡す並びは配列と数 |
| 読み取りだけの並び | `size_t f(self, out, capacity)`。全体の数を返し、`out` に `capacity` 個まで書く(`f(self, NULL, 0)` で数だけ得る) |
| ハンドラ | `(fn, user_data, destroy)` を渡す。置き換わる・外れる・Object が破棄されるときに `destroy(user_data)` を呼ぶ(`destroy` は NULL でもよい)。`fn` に NULL でハンドラを外す。イベントの構造体の `accepted` を false にすると親へ送る |
| 失敗 | 戻り値を既定の値(0・NULL・false)にし、`tokotachi_last_error()` で説明を読める(次の失敗か `tokotachi_clear_error` まで)。`tokotachi_set_error_handler` で、失敗したときに呼ぶ関数を付けられる |
| スレッド | Object・ウィジェットはメインスレッド(Application を作ったスレッド)だけで扱う。別のスレッドからは `tokotachi_post` で頼む([ADR 0029](../adr/0029-threads-and-post.md)) |

## C++ と違うところ

| C++ | C |
|---|---|
| `tokotachi::bind(edit->text, name->value)` | `tokotachi_bind(edit, "text", name, "value")`(名前で指す。型が違う・読み取り専用へは false) |
| `Variable<T>` | `tokotachi_variable_new(owner, TOKOTACHI_VARIABLE_TYPE_STRING)`(bool・int・float・double・string)と `tokotachi_variable_get_string` など |
| `Task<T>` | `tokotachi_task_new`・`tokotachi_task_start(task, work, user_data, destroy)`(結果は user_data で受け渡す。失敗は `tokotachi_task_context_fail`) |
| `ItemModel` を継承したモデル | `tokotachi_custom_model_new(owner, &callbacks, user_data, destroy)`(`tokotachi_model_callbacks` の関数で答える。変更は `tokotachi_custom_model_notify_*`) |
| `Widget::invalidate(const Rect&)` | `tokotachi_widget_invalidate_rect` |
| `Canvas&`(`on_paint` の引数) | `tokotachi_canvas*`(ハンドラの中だけ有効) |
| `Image`・`Bitmap`・`Path`・`TextLayout` | ハンドル(`tokotachi_image_free` などで解放する)。`Image` を返す関数は新しいハンドルを返し、空の画像は NULL |
| `Paint`・`StrokeStyle` | `tokotachi_paint`・`tokotachi_stroke_style` の構造体へのポインタ |
| 継承した仮想関数(`paint`・`mouse_down` など) | 無い(PaintBox とハンドラを使う) |
| `MessageDialog::show(...)` | `tokotachi_message_dialog_show(...)`(ボタンは配列と数) |
| `FileDialog::filters` | 名前と拡張子を交互に並べた文字列の並び |

## API を変えるとき

ヘッダと実装の大部分は [bindings/c/api.py](../../bindings/c/api.py) から [generate.py](../../bindings/c/generate.py) で作る
(Python の宣言 `bindings/python/src/tokotachi/_api.py` も同時に作る)。

1. C++ の公開の API を変えたら、`api.py` のクラス(`P`・`H`・`M`)を直す。値の型として扱いが違うものは `src/manual.cpp` に手で書き、宣言を `F` で書く。
2. `python3 bindings/c/generate.py` で作り直す(CI は `--check` で合っているか確かめる)。
3. ビルドして `ctest` を通す(C と Python のテスト)。
