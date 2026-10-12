"""Tokotachi の Python のサンプル(examples/hello の小さい版)。

  python3 hello.py                     ウィンドウを開く
  python3 hello.py --screenshot a.png  1 回描いて PNG に保存して終える
"""

import sys

import tokotachi as tt


def main():
    app = tt.Application()
    window = tt.Window('hello - Tokotachi (Python)', 420, 300)
    status_bar = tt.StatusBar()
    status_bar.text = '準備完了'
    window.status_bar = status_bar

    column = tt.BoxLayout(window)
    column.margins = 24
    column.spacing = 12

    title = tt.Label(window, 'Tokotachi のサンプル')
    title.font = 22
    column.add(title)

    # Variable を介して、入力欄とラベルの文字をそろえる(docs/adr/0016)
    name = tt.Variable(window, '世界')
    row = tt.Widget(window)
    row_box = tt.BoxLayout(row, tt.Orientation.HORIZONTAL)
    row_box.margins = 0
    row_box.add(tt.Label(row, '名前:'))
    edit = tt.LineEdit(row)
    row_box.add(edit, 1)
    column.add(row)
    greeting = tt.Label(window)
    greeting.color = tt.ColorRole.TEXT_SECONDARY
    column.add(greeting)
    tt.bind(edit, 'text', name, 'value')
    name.on_change = lambda sender: status_bar.show_message('名前を変えました', 2000)
    greeting.text = 'こんにちは、' + name.value + ' さん'
    edit.on_change = lambda sender: setattr(greeting, 'text', 'こんにちは、' + edit.text + ' さん')

    # 描く部品
    box = tt.PaintBox(window)
    box.minimum_size = (0, 60)

    def paint(sender, canvas):
        r = sender.local_rect
        canvas.fill_round_rect(r, 8, tt.Paint.linear((0, 0), (r.width, 0),
                                                     [(0, tt.Color.rgb(0x2563eb)), (1, tt.Color.rgb(0x059669))]))
        canvas.draw_text('PaintBox', (12, 20), tt.Color(255, 255, 255), 16)

    box.on_paint = paint
    column.add(box, 1)

    button = tt.Button(window, '閉じる')
    button.on_click = lambda sender: window.close()
    column.add(button, 0, tt.Align.END)

    window.visible = True
    if len(sys.argv) >= 3 and sys.argv[1] == '--screenshot':
        return 0 if window.save_png(sys.argv[2]) else 1
    return app.exec()


if __name__ == '__main__':
    sys.exit(main())
