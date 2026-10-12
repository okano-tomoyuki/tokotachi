# 0041. C の API を仕様から生成して単独の共有ライブラリで配り、Python のバインディングはその上に ctypes で作る

- 状態: 承認
- 日付: 2026-10-12

## 背景

Tokotachi は C++11 のライブラリで、公開の API はプロパティ(`Property<T>`)・ハンドラ(`std::function`)・テンプレート
(`Variable<T>`・`Task<T>`)で表している(ADR 0013・0016・0029)。他の言語(まず Python)からも使えるようにしたい。

他の言語から C++ の API を直接は呼べない。C++ の ABI はコンパイラ・標準ライブラリの版で変わり、`std::string`・`std::function`・
例外は言語の境界を越えられない。多くの言語は C の関数なら呼べる(Python の ctypes・cffi、C#、Rust、Go など)ので、
C の API を共通の土台にし、各言語のバインディングをその上に作る。

決めること:

- C の API の形(名前・ハンドル・所有・文字列・ハンドラ・失敗の伝え方)と、作り方(手で書くか生成するか)。
- 配る形(どのライブラリに入れるか)。
- Python のバインディングの作り方と、Object の寿命の扱い。

## 検討した選択肢

### C の API の作り方

- 選択肢A: すべて手で書く。プロパティだけで 300 を超え、ヘッダ・実装・各言語の宣言の 3 か所に同じことを書くことになる。
- 選択肢B: C++ のヘッダ(Doxygen の XML)から自動で作る。名前・型は取れるが、所有・既定値・NULL の可否などの意味は取れず、
  例外の指定が多くなる。
- 選択肢C: 仕様(`bindings/c/api.py`)に、クラス・プロパティ・ハンドラ・メソッドを書き、生成器(`generate.py`)が
  C のヘッダ・C++ の実装・Python の宣言を作る。型ごとの変換は生成器に 1 回だけ書く。値の型として扱いの違うもの
  (Canvas・Image・Bitmap・Path・TextLayout・Variable・Task・独自のモデル・結びつけ)は手で書き、宣言だけを仕様に書く。

### ハンドル

- 選択肢A: クラスごとに別の型(`tokotachi_label*`)にし、基底の関数に渡すときにキャストする(GTK の形)。
- 選択肢B: Object の派生はすべて 1 つの型(`tokotachi_object*`)で指し、クラスの名前はその別名にする。型の確かめは実行時に
  `dynamic_cast` で行い、違えば失敗にする。

### 配る形

- 選択肢A: C の関数を `libtokotachi` に入れる。
- 選択肢B: 別の共有ライブラリ `libtokotachi_c` にし、`libtokotachi` に依存する。他の言語から使うには 2 つを配り、C++ の標準ライブラリも合わせる必要がある。
- 選択肢C: 別の共有ライブラリ `libtokotachi_c` に、ライブラリの C++ のオブジェクトと依存ライブラリを中に入れ、`tokotachi_*` の C の関数だけを公開する。

### Python のバインディング

- 選択肢A: pybind11・nanobind で C++ を直接包む。C の API を使わず、Python のための C++ の拡張モジュールを Python の版ごとにビルドする。
- 選択肢B: CPython の C の拡張モジュールで C の API を包む。速いが、Python の版ごとにビルドが要る。
- 選択肢C: ctypes で C の API を読む(純粋な Python)。呼び出しは遅め(1 回 1 µs 程度)だが、GUI の操作では問題にならない。
  Python の版に依らず、C の API の共有ライブラリ 1 つだけを配ればよい。
- 選択肢D: cffi。ctypes と同じ利点だが、利用者に依存を 1 つ増やす。

## 決定

**C の API は選択肢C(仕様から生成)、ハンドルは選択肢B、配る形は選択肢C、Python は選択肢C(ctypes)とする。**

### C の API の決まり(`bindings/c/include/tokotachi/tokotachi.h`)

- 名前は `tokotachi_<クラス>_<操作>`(C++ の名前空間と同じ接頭辞。依存している plutovg と同じく、ライブラリの名前をそのまま使い、
  FreeType の `TT_` などとぶつからないようにする)。プロパティは `_get_`・`_set_`、ハンドラは `_set_on_xxx`、コンストラクタは `_new`。
  列挙は `TOKOTACHI_ALIGN_START` の形で、値は C++ の `enum class` と同じ順(生成した実装の `static_assert` で確かめる)。
- Object の派生は `tokotachi_object*` で指し、`tokotachi_label` などはその別名。基底のクラスの関数に派生のものをそのまま渡せる。
  型の違うハンドルは失敗にする(落ちない)。
- 所有は C++ と同じ(ADR 0015)。所有者の無いものは `tokotachi_object_delete` で破棄する。`tokotachi_object_set_user_data` で、
  Object が破棄されるときに呼ばれる関数を付けられる(対象の Object が所有する見えない Object が持つ)。バインディングは
  これで、破棄された Object に触らないようにする。
- 文字列は UTF-8。返す文字列は malloc したもので `tokotachi_free` で解放する。読み取りだけの並びは、呼んだ側の配列に書いて全体の数を返す。
- ハンドラは関数・利用者のデータ・解放の関数を受け取り、ハンドラが置き換わる・外れる・Object が破棄されるときに解放の関数を呼ぶ
  (`std::function` の最後の写しが無くなるとき)。
- C++ の例外は C の関数の外に出さず、既定の値を返して失敗を記録する(`tokotachi_last_error`)。失敗のハンドラ
  (`tokotachi_set_error_handler`)を付けると、失敗したスレッドで呼ぶ。
- テンプレートのクラスは、型を選んで作る関数にする(`tokotachi_variable_new(owner, TOKOTACHI_VARIABLE_TYPE_STRING)`、
  `Task` は結果を持たない `Task<void>` だけ。結果は利用者のデータで受け渡す)。
- `tokotachi::bind` は名前で指す(`tokotachi_bind(edit, "text", name, "value")`)。生成した実装が、名前・型・C++ のプロパティへの表を持つ。
- 独自のモデル(C++ では `ItemModel` を継承するもの)は、関数の表(`tokotachi_model_callbacks`)で答えるモデルにする。
- 継承して仮想関数を上書きする使い方(独自のウィジェット)は C の API には出さない。描くのは PaintBox と `on_paint` で行う。

### 配る形

- C の API は、CMake のオプション `TOKOTACHI_BUILD_C_API`(単独のとき ON。Emscripten では OFF)で作る、単独の共有ライブラリ
  `tokotachi_c` とする。ライブラリの C++ のオブジェクト(`tokotachi_objects`)と依存ライブラリを中に入れ、`tokotachi_*` だけを公開する
  (Linux はバージョンスクリプト、MinGW は `dllexport`)。MinGW では C++ のランタイムも静的にリンクする。
  利用者は C++ の標準ライブラリの違いを気にせずに読み込める。
- C++ のライブラリと C の API を 1 つのプロセスで両方読み込むと、ライブラリが 2 つ入る(Application も別々)。どちらかだけを使う。
- インストールすると、`include/tokotachi/tokotachi.h`・`tokotachi::tokotachi_c`(CMake のパッケージ)・`tokotachi_c.pc` が入る。

### Python のバインディング(`bindings/python/`)

- パッケージ `tokotachi` は純粋な Python で、C の API を ctypes で読む。クラス・プロパティ・ハンドラ・列挙の名前は C++ と同じ
  (`label.text = '...'`、`button.on_click = lambda sender: ...`、`tt.Align.CENTER`)。値の型は NamedTuple(`Rect`・`Color` など)で、
  タプルもそのまま渡せる。
- ctypes の宣言・列挙・クラスは、C の API と同じ仕様から生成する(`_api.py`)。手で書くのは寿命・変換・描画の型(`_core.py`)。
- **寿命**: C++ の Object 1 つに Python の Object 1 つ。所有者のある Object は C++ の所有者が寿命を決め、Python の Object は
  C++ の Object が破棄されるまで保つ(ハンドラの sender・children で同じものが返り、利用者が付けた属性も残る)。
  Python で作った所有者の無い Object(Window など)は、Python の Object が無くなったとき(または `delete()`)に破棄する(PyQt と同じ)。
  別のスレッドで回収されたときは、メインスレッドで破棄するよう post する。Python が終わるときは、所有者の無い Object を破棄してから
  Application を破棄する。C++ の Object が破棄された後に触ると `RuntimeError`(落ちない)。
- C の API の失敗は `TokotachiError`、ハンドラの中の例外は `sys.excepthook` で知らせて続ける。
- `pip install ./bindings/python` で、scikit-build-core が CMake で `tokotachi_c` をビルドしてパッケージに入れる(Python の版に依らない wheel)。
  ビルドした C の API を使うときは `TOKOTACHI_C_LIBRARY` でパスを指す。

### 確かめること

- C のテスト(`bindings/c/tests/test_c.c`)を C99 としてコンパイルして動かす(ヘッダが C として通ること・ハンドラの解放・結びつけ・
  モデル・描画・イベントループ・Task)。Python のテスト(`bindings/python/tests/`)を ctest から動かす。ctypes の構造体の大きさは
  `tokotachi_struct_size` で C と比べる。どちらも AddressSanitizer・UndefinedBehaviorSanitizer で確かめた。
- 生成したファイルが仕様と合っていることを CI で確かめる(`generate.py --check`)。

## 影響

- C++ の公開の API を変えたら、`bindings/c/api.py` も直して生成し直す(合わなければ C++ の実装のコンパイルが通らない)。
  C++ の API の追加を C の API に出すかは、そのつど決める。
- C の API の版はライブラリの版と同じ。0.x の間は、小さい版で C の ABI も変わりうる。
- C の API には継承による拡張が無いので、独自のウィジェット・レイアウトは C++ で書く。
- デザイナー(ADR 0039)が生成するのは C++ だけのまま。Python のコードの生成は後で検討する。
- Windows(MinGW)での C の API と Python のバインディングは、CI では確かめていない(Linux だけ)。
