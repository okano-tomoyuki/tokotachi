# ウィンドウ・メニュー・ダイアログ {#windows_guide}

[TOC]

![メニューバー・ツールバー・ステータスバー・タブ](window.png)

## 複数のウィンドウ

ウィンドウは `new` で作ります。所有者を渡さなければ `Application` が所有し、`Application` の破棄で破棄されます。

```cpp
tokotachi::Window* window_main = new tokotachi::Window("エディタ", 800, 600);         // Application が所有する
app.main_window = window_main;                                                     // 閉じたら終える
tokotachi::Window* window_find = new tokotachi::Window(window_main, "検索", 360, 120);  // window_main が所有し、その上に出る
tokotachi::Window* window_help = new tokotachi::Window("ヘルプ", 480, 360);
window_help->destroy_on_close = true;                                              // 閉じたら破棄する
```

- **所有者のウィンドウ**を渡すと、表示したときに所有者の上に出ます(モーダルではありません。モーダルにするには `Dialog`)。
  所有者を閉じると一緒に隠れ、所有者を破棄すると一緒に破棄されます。
- 閉じる(`close()`・閉じるボタン)と、ウィンドウは隠れるだけです。また表示できます。`destroy_on_close` なら閉じたら破棄します
  (必要なときに作る補助のウィンドウに使います)。
- **終わり方**: `Application::main_window` を決めると、そのウィンドウを閉じたら(ほかのウィンドウが開いていても)`exec()` から戻ります。
  決めなければ、最後のウィンドウを閉じたときに戻ります。`auto_quit_enabled` を false にすると、`quit()` を呼ぶまで戻りません。

## Action とショートカット

利用者が実行できる操作は **Action** で表します。メニューの項目とツールバーのボタンが同じ Action を指すので、
無効にすれば両方が無効になります。

```cpp
tokotachi::Action* action_save = new tokotachi::Action(window_main, "保存(&S)");
action_save->shortcut = "Ctrl+S";
action_save->on_trigger = [](tokotachi::Object*) { ... };
action_save->enabled = false;      // メニューもボタンもキーも無効
```

- text の `&` の次の文字はニーモニックです(Alt+ニーモニック・メニューの中でそのキー)。
- ラベル・ボタン・チェックボックス・ラジオボタンの text と、GroupBox の title の `&` もニーモニックです(`&` を出すには `&&`)。
  Alt と一緒に押すと、ラベルはタブの順で次の部品(`buddy` を指定すればその部品)へ移り、ボタンなどは押したことになります。
  Alt を押している間は下線が出ます。タブの順は部品を作った順なので、入力欄をラベルより先に作ったときは `buddy` を指定します。
- `checkable` な Action は、実行するたびに `checked` が反転します(メニューではチェックの印、ツールバーでは押し込んだ見た目)。
- ショートカットが効くのは、メニューバー・ツールバーに入れた Action(ウィンドウ全体)と、部品の `add_action` で加えた Action
  (フォーカスがその中にあるとき)です。フォーカスのある部品が受け取ったキー(入力欄の Ctrl+C など)が優先します。

## メニューバー・ツールバー・ステータスバー

```cpp
tokotachi::MenuBar* menu_bar = new tokotachi::MenuBar();
tokotachi::Menu* menu_file = menu_bar->add_menu("ファイル(&F)");
menu_file->add_action(action_save);
menu_file->add_separator();
menu_file->add_item("終了(&X)", [&](tokotachi::Object*) { window_main->close(); });
window_main->menu_bar = menu_bar;                       // 上端に置く

tokotachi::ToolBar* tool_bar = new tokotachi::ToolBar();
tool_bar->add_action(action_save);                     // 画像(icon)があれば画像、無ければ文字
window_main->tool_bar = tool_bar;                     // メニューバーの下

tokotachi::StatusBar* status_bar = new tokotachi::StatusBar();
status_bar->text = "準備完了";
status_bar->show_message("保存した", 3000);       // 3 秒だけ
window_main->status_bar = status_bar;                  // 下端
```

レイアウトの領域は、これらの間になります。右クリックのメニューは `Menu::popup(部品, 位置)` で開きます。

## ダイアログ

ダイアログは親のウィンドウの上に開くモーダルのウィンドウです。**`open()` はすぐに戻り、結果は `on_finish` で受け取ります**。

```cpp
tokotachi::MessageDialog::show(window_main, "確認", "変更を保存しますか？",
    std::vector<tokotachi::DialogButton>{tokotachi::DialogButton::SAVE, tokotachi::DialogButton::DISCARD,
                                         tokotachi::DialogButton::CANCEL},
    [](tokotachi::Object* sender)
    {
        if (static_cast<tokotachi::Dialog*>(sender)->result == tokotachi::DialogButton::SAVE) { ... }
    },
    tokotachi::MessageType::QUESTION);
```

![メッセージのダイアログ](dialog.png)

- 自分で中身を作るときは `Dialog` にレイアウトを付けて部品を並べ、`add_button` でボタンを加えます。
- ボタンは OS の慣習の順に並びます(Windows・KDE は「OK キャンセル」、GNOME などは「キャンセル OK」)。
- Enter は決定のボタン、Esc・閉じるボタンは取り消しのボタンを押します。
- ファイルの選択は `FileDialog`(OS のダイアログ)。`paths` で選んだパスを、`error` で失敗の理由を読みます。
- 色の選択は `ColorDialog`(`ColorDialog::show(window_main, 今の色, on_finish)`。`alpha_enabled` で不透明度も)。押すと開く `ColorButton` もあります。
