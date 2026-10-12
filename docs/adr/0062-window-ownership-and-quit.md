# 0062. ウィンドウの所有・所有者のウィンドウ・メインのウィンドウと終わり方、オブジェクトは new で作る書き方に揃える

- 状態: 承認
- 日付: 2026-10-13

## 背景

マルチウィンドウ(複数のトップレベルのウィンドウ・モーダルのダイアログ・ポップアップ)は動くが、ウィンドウの所有と終わり方が決まっていない。

- トップレベルの `Window` は作ると Application の内部の一覧に加わるが、所有者が無い。`new` して `delete` し忘れると漏れ、Application が
  先に破棄されると、SDL を終えた後にウィンドウを破棄することになる。
- 所有者を持てるのはモーダルの `Dialog` と内部のポップアップだけで、ツールのウィンドウや検索の窓のような「親の上に出て、親と一緒に
  破棄される、モーダルでないウィンドウ」が作れない。
- 表示しているウィンドウ(ポップアップを除く)が無くなると `exec()` を抜ける。これを変えられず、「メインのウィンドウを閉じたら終える」
  (C++ Builder の MainForm)も、「quit() するまで終えない」(常駐するものなど)もできない。
- 閉じる(`close()`)は隠すだけで、必要なときに作る補助のウィンドウを閉じたら破棄する、ということを利用者が書く必要がある。
- サンプル・手引きのコードで、`Application` と `Window` はスタックに置き、ほかの部品は `new` で作っていて、書き方が混ざっている。

比較:

| | 登録 | 所有 | 終わり方 |
|---|---|---|---|
| C++ Builder | `Application->CreateForm` で明示 | Application が所有し、終わるときに破棄する | メインのフォームを閉じたら終える |
| Qt | 作れば暗黙に加わる | 親が無ければ利用者 | 最後のウィンドウを閉じたら終える(切り替えられる) |
| Tokotachi(これまで) | 作れば暗黙に加わる | 利用者 | 最後のウィンドウを閉じたら終える(切り替えられない) |

## 検討した選択肢

- 登録: 明示する(`app.add_window(w)`・C++ Builder の `CreateForm`)/作れば暗黙に加わる(これまでどおり)。`CreateForm` は、設計時の
  データからフォームを作り、グローバルな変数に入れるための仕組みに近い。C++ では作った時点で加わる方が RAII と合い、加え忘れも起きないので、
  暗黙のままにする。
- 所有: 利用者(これまで)/Application(C++ Builder)。Application が所有すれば、`new` したウィンドウが漏れず、破棄の順も保てる。
- 書き方: すべて `new`(Application も)/Application だけスタック、ほかの Object は `new`/混ざったまま。Application も `new` すると、
  最後に `delete` を書くことになり(混ざり方が後ろに移るだけ)、書き忘れや途中の return・例外で後始末(残ったウィンドウの破棄・SDL の終了)が
  抜ける。根の Application だけはスタックに置き、RAII に任せる。

## 決定

### A. トップレベルのウィンドウは Application が所有する

- 所有者を渡さずに作った `Window`(`new tokotachi::Window("題", 640, 480)`)は、Application が所有する(`Object` の所有者が Application)。
  Application の破棄で、残っているウィンドウを破棄する。
- Application のデストラクタは、所有するもの(ウィンドウ・Application を所有者にしたタイマーなど)を、タイマーの停止・SDL の終了より
  **前に**破棄する。
- スタックに置いたウィンドウは、これまでどおり使える(先に破棄され、Application の所有から外れる)。

### B. 所有者のウィンドウを渡して作る

- `Window(Window* owner, const std::string& title, float width = 640, float height = 480)` を加える。引数の順は `Dialog(parent, title)` と同じ。
- 所有者のウィンドウが所有し、一緒に破棄される。表示すると所有者の上に出る(`SDL_SetWindowParent`。モーダルにはしない)。所有者を最小化すると
  一緒に隠れるかどうかは OS による。
- 所有者を隠す(閉じる)と、所有するウィンドウも隠す。所有者をもう一度表示しても、所有するウィンドウは自動では表示しない。
- 所有者は作るときに決め、後から変えない。`Dialog` は、所有者のウィンドウのモーダル版という位置づけになる。

### C. メインのウィンドウと終わり方

- `Application::main_window`(`Property<Window*>`。既定 nullptr)を加える。
- `Application::auto_quit_enabled`(`Property<bool>`。既定 true)を加える。
- 終わり方:
  - `auto_quit_enabled` が false: `quit()` を呼ぶまで終えない。
  - `main_window` を決めている: そのウィンドウを閉じたら(隠れたら・破棄されたら)終える。ほかのウィンドウが表示されていても終える。
  - 決めていない: これまでどおり、表示しているウィンドウ(ポップアップを除く)が無くなったら終える。
- `main_window` のウィンドウが破棄されると、`main_window` は nullptr になる(知らせる)。
- `exec()` を呼んだときに条件を満たしていれば(メインのウィンドウを表示していないなど)、すぐに戻る(これまでと同じ)。

### D. 閉じたら破棄する

- `Window::destroy_on_close`(`Property<bool>`。既定 false)を加える。true なら、閉じた(隠れた)後に `delete_later()` で破棄する。
  `Dialog` では `on_finish` を呼んだ後に破棄する。
- `new` で作ったウィンドウにだけ使う(スタックのウィンドウを破棄することになるため)。説明に書く。

### E. オブジェクトは new で作る書き方に揃える

- **Object から派生したもの(ウィンドウ・部品・タイマー・変数・モデル・Action など)は `new` で作り、所有者に持たせる。スタックに置くのは
  根の `Application` だけにする。** 値の型(`Rect`・`Color`・`Image`・`Bitmap`・`Font`・`Path`・`Theme` など)は、これまでどおり値で使う。
- サンプル・手引き・README・ヘッダの例、デザイナーが生成する `main.cpp` をこの形に揃え、手引きの「基本の考え方」に規則を書く。

```cpp
int main(int argc, char** argv)
{
    tokotachi::Application app(argc, argv);                  // 根。これだけスタックに置く
    MainWindow* window_main = new MainWindow();              // Application が所有する
    app.main_window = window_main;                           // 閉じたら終える
    window_main->visible = true;
    return app.exec();
}                                                            // app の破棄で、残っているウィンドウも破棄される
```

- `Window` をスタックに置く書き方・`Application` を `new` する書き方も、引き続き使える(勧めない)。

## 影響

- どれも足すだけの変更で、これまでの利用者のコード(スタックのウィンドウを含む)はそのまま動く。
- ABI は変わる(Application・Window にプロパティを足す)。0.x の間なので、次の小さい版(0.3.0)で出す。
- デザイナー: 生成する `main.cpp` はプロジェクトの `main_window` を `new` で作り、`app.main_window` に代入する。既に生成した `main.cpp` の
  起動部分は区間(`tokotachi_main`)なので、再生成で新しい形になる。
- 2 つ以上のトップレベルのウィンドウを使うサンプル(所有者のウィンドウ・閉じたら破棄するウィンドウ)をギャラリーに加える。

## 実装

- `Window` のコンストラクタは、所有者のウィンドウが無ければ `Application` を所有者にする(`detail::change_owner`)。所有者のウィンドウは
  `Window::Impl::owner_window_` に持ち、表示するときに(所有者が表示されていれば)`SDL_SetWindowParent` で親子にする。隠すときに、
  所有するウィンドウ(`owner_window_` が自分のもの)も隠す。`Dialog` は所有者のウィンドウを持たない(親子・モーダルは `open` で決める)。
- `Application` のデストラクタは、最初に `delete_owned()` で所有するものを破棄する(タイマーの停止・SDL の終了より前)。
- 終わり方の判断は `Application::Impl::quit_if_done()`(`Window::close()` と `exec()` の始めで呼ぶ)。`visible` に false を代入して
  隠したときは判断しない(これまでと同じ。閉じる操作だけが終わりにつながる)。メインのウィンドウの破棄は `window_destroyed()` で知る。
- `destroy_on_close` は `Window::close()` の最後に `delete_later()` を呼ぶ。`Dialog::finish` は `Window::close()` の後に `on_finish` を
  呼ぶので、破棄は `on_finish` の後になる。
- テスト: 所有(Application・所有者のウィンドウ)と一緒に隠す・破棄する、終わり方の 3 つの決まりとメインのウィンドウの破棄、閉じたら破棄する
  (ウィンドウ・ダイアログ)。サンプル `examples/gallery/windows.cpp` を加えた。
- デザイナーが生成する Window のクラスは、所有者のウィンドウを任意で受け取る(`explicit MainWindow(tokotachi::Window* owner = nullptr)`、
  `: tokotachi::Window(owner, std::string())`)。利用者が、Application に持たせるか所有者のウィンドウに持たせるかを決める。
  コンストラクタは生成の区間の外にあるので、既に生成したファイルは利用者が直す。
