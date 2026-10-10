# ウィンドウ・メニュー・ダイアログ {#windows_guide}

[TOC]

![メニューバー・ツールバー・ステータスバー・タブ](window.png)

## Action とショートカット

利用者が実行できる操作は **Action** で表します。メニューの項目とツールバーのボタンが同じ Action を指すので、
無効にすれば両方が無効になります。

```cpp
tokotachi::Action* save = new tokotachi::Action(&window, "保存(&S)");
save->shortcut = "Ctrl+S";
save->on_trigger = [](tokotachi::Object*) { ... };
save->enabled = false;      // メニューもボタンもキーも無効
```

- text の `&` の次の文字はニーモニックです(Alt+ニーモニック・メニューの中でそのキー)。
- `checkable` な Action は、実行するたびに `checked` が反転します(メニューではチェックの印、ツールバーでは押し込んだ見た目)。
- ショートカットが効くのは、メニューバー・ツールバーに入れた Action(ウィンドウ全体)と、部品の `add_action` で加えた Action
  (フォーカスがその中にあるとき)です。フォーカスのある部品が受け取ったキー(入力欄の Ctrl+C など)が優先します。

## メニューバー・ツールバー・ステータスバー

```cpp
tokotachi::MenuBar* bar = new tokotachi::MenuBar();
tokotachi::Menu* file = bar->add_menu("ファイル(&F)");
file->add_action(save);
file->add_separator();
file->add_item("終了(&X)", [&](tokotachi::Object*) { window.close(); });
window.menu_bar = bar;                       // 上端に置く

tokotachi::ToolBar* tools = new tokotachi::ToolBar();
tools->add_action(save);                     // 画像(icon)があれば画像、無ければ文字
window.tool_bar = tools;                     // メニューバーの下

tokotachi::StatusBar* status = new tokotachi::StatusBar();
status->text = "準備完了";
status->show_message("保存した", 3000);       // 3 秒だけ
window.status_bar = status;                  // 下端
```

レイアウトの領域は、これらの間になります。右クリックのメニューは `Menu::popup(部品, 位置)` で開きます。

## ダイアログ

ダイアログは親のウィンドウの上に開くモーダルのウィンドウです。**`open()` はすぐに戻り、結果は `on_finish` で受け取ります**。

```cpp
tokotachi::MessageDialog::show(&window, "確認", "変更を保存しますか？",
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
