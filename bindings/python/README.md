# Tokotachi for Python

SDL3 を土台にした GUI ライブラリ [Tokotachi](https://github.com/okano-tomoyuki/tokotachi) の Python のバインディング。
C API(`tokotachi_c`)を ctypes で読む。クラス・プロパティ・ハンドラの名前は C++ と同じ。

```python
import tokotachi as tt

app = tt.Application()
window = tt.Window('hello', 320, 200)
layout = tt.BoxLayout(window)
label = tt.Label(window, 'こんにちは')
button = tt.Button(window, '閉じる')
button.on_click = lambda sender: window.close()
layout.add(label)
layout.add(button)
window.visible = True
app.exec()
```

インストール(リポジトリの中から。C API と依存ライブラリをビルドするので、初めては数分かかる):

```sh
pip install ./bindings/python
```

ビルドした C API を使うときは、環境変数 `TOKOTACHI_C_LIBRARY` に共有ライブラリのパスを入れる。
詳しくは [docs/bindings/python.md](../../docs/bindings/python.md)。
