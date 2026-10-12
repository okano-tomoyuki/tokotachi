# スレッド {#threads_guide}

[TOC]

## 決まり

**部品・Object・プロパティ・テーマは、メインスレッド(Application を作ったスレッド)からだけ触ります。** ロックは持っていません。
別のスレッドで重い処理をしたら、結果を画面に出す処理をメインスレッドへ頼みます。`tokotachi::is_main_thread()` で確かめられます。

## Task: 終わりのある仕事

重い処理を別のスレッドで行い、進み具合と結果をメインスレッドで受け取ります。

```cpp
tokotachi::Task<int>* count = new tokotachi::Task<int>(window_main);   // window_main が破棄されたら取り消し、結果は捨てる
count->on_progress = [=](tokotachi::Object*) { progress_bar_count->value = count->progress; };
count->on_finish = [=](tokotachi::Object*)
{
    if (!count->error.get().empty()) { ... }       // 仕事が例外を投げた
    else if (count->cancelled) { ... }             // 取り消した
    else { label_result->text = std::to_string(count->result.get()); }
};
count->start([](tokotachi::TaskContext& ctx) -> int
{
    int n = 0;
    for (int i = 0; i < 100 && !ctx.cancel_requested(); ++i)   // 別のスレッド: 部品には触らない
    {
        n += heavy_work(i);
        ctx.report_progress(i + 1);                            // 頻繁に呼んでよい(間引いて届く)
    }
    return n;
});
count->cancel();       // 取り消しを頼む(待たない)
```

## post と WeakRef: 自分でスレッドを作るとき

常駐するスレッド(通信の受信など)は `std::thread` で作り、画面に出す処理を `tokotachi::post` で頼みます。
**頼んだ処理が呼ばれる前に部品が破棄されることがある**ので、部品は `WeakRef` で渡します(メインスレッドで作り、別のスレッドへはコピーで渡す)。

```cpp
tokotachi::WeakRef<tokotachi::Label> status(label_result);      // メインスレッドで作る
std::thread([status]()
{
    const std::string result = compute();               // 別のスレッド
    tokotachi::post(status, [result](tokotachi::Label* l) { l->text = result; });   // 残っていれば呼ぶ
}).detach();
```

- 頼んだ処理は、メインスレッドのイベントの処理の中で、頼んだ順に呼ばれます。
- 終わるまで待つ呼び出しはありません(デッドロックを避けるため)。

## 別のスレッドでできること

- `Bitmap` とその Canvas での描画(文字も)、`TextLayout` の作成、`Image` の読み込み。
- テーマの色は別のスレッドから読めないので、メインスレッドで決めて渡します。
