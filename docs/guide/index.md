# Tokotachi {#mainpage}

Tokotachi は、SDL3 を土台にした C++11 のクロスプラットフォーム GUI ライブラリです。
既存の GUI の層には頼らず、OS との境界には必要最小限のもの(SDL3)だけを使い、その上に高機能な部品を一から積み上げています。
ライセンスは MIT です。

- 対象: Windows(MinGW-w64)、Linux(Wayland・X11。x86_64・aarch64)
- 部品: ボタン・入力欄・値の部品・タブ・分割・一覧と木・メニューバー・ツールバー・ダイアログ・自由な描画など
- 日本語: IME・禁則での折り返し・右から左へ書く文字・ダークのテーマ・高 DPI

名前は、日本の神話で国の土台そのものとして最初に現れる国之常立神(くにのとこたちのかみ)の「常立(とこたち)」から取りました。

![サンプル](window.png)

## 最初のプログラム

```cpp
#include <tokotachi/tokotachi.hpp>

int main(int argc, char** argv)
{
    tokotachi::Application app(argc, argv);
    tokotachi::Window* window_main = new tokotachi::Window("hello", 320, 160);

    tokotachi::BoxLayout* box = new tokotachi::BoxLayout(window_main);   // 並べ方はレイアウトに任せる
    tokotachi::Label* label_count = new tokotachi::Label(window_main, "こんにちは");
    tokotachi::Button* button_push = new tokotachi::Button(window_main, "押す");  // 親(window_main)が所有する
    box->add(label_count);
    box->add(button_push);

    int count = 0;
    button_push->on_click = [&](tokotachi::Object*)                         // イベントは 1 つのハンドラ
    {
        label_count->text = "押した回数: " + std::to_string(++count);           // 状態はプロパティで読み書きする
    };

    app.main_window = window_main;      // 閉じたら終える
    window_main->visible = true;
    return app.exec();
}
```

## 手引き

| ページ | 内容 |
|---|---|
| @subpage getting_started | ビルド・インストール・自分のプロジェクトから使う |
| @subpage basics | 親子と所有・プロパティ・ハンドラ・変数と結びつけ・テーマ |
| @subpage layout_guide | BoxLayout・GridLayout・PlaceLayout・スクロール・分割 |
| @subpage windows_guide | メニューバー・ツールバー・ステータスバー・Action とショートカット・ダイアログ |
| @subpage views_guide | ListView・TreeView・TableView とモデル |
| @subpage drawing_guide | Canvas・PaintBox・Bitmap・変換・透過 |
| @subpage custom_widgets_guide | 自作の部品: どのクラスから作るか・上書きする口・プロパティ・標準の見た目を描く関数 |
| @subpage threads_guide | メインスレッドの決まり・post・WeakRef・Task |
| @subpage gallery | 部品ごとのサンプルと画面 |

## API リファレンス

左の「トピック」から、分類ごとにクラスを引けます(部品は入力・表示・入れ物などに分けています)。「クラス」「ファイル」からは、すべてのクラスと関数を引けます。
部品のページには、その部品の画面の画像を載せています。

| 分類 | 内容 |
|---|---|
| @ref core | オブジェクトとプロパティ・イベント・スレッド・基本の値 |
| @ref graphics | Canvas・Image・Bitmap・文字のレイアウト・テーマ |
| @ref layout | BoxLayout・GridLayout・PlaceLayout |
| @ref widgets | 部品(入力・表示・入れ物・描画・基底) |
| @ref windows | Window・ダイアログ・メニューバー・ツールバー・ステータスバー |
| @ref actions | Action・Menu・Shortcut |
| @ref views | 一覧と木と表のビューとモデル |

ヘッダは役割ごとのディレクトリに分かれています。

| インクルード | 内容 |
|---|---|
| `<tokotachi/tokotachi.hpp>` | すべて |
| `<tokotachi/core.hpp>` | Object・プロパティ・結びつけ・変数・イベント・Application・タイマー・スレッド・基本の値 |
| `<tokotachi/graphics.hpp>` | Canvas・Image・Bitmap・文字のレイアウト・テーマ |
| `<tokotachi/layouts.hpp>` | Layout・BoxLayout・GridLayout・PlaceLayout |
| `<tokotachi/widgets.hpp>` | Widget と部品 |
| `<tokotachi/windows.hpp>` | Window・ダイアログ・メニューバー・ツールバー・ステータスバー |
| `<tokotachi/actions.hpp>` | Action・Shortcut・Menu |
| `<tokotachi/views.hpp>` | 一覧と木のモデルとビュー |
