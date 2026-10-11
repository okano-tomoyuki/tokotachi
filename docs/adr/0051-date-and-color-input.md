# 0051. 日付の値 Date と、日付の入力欄 DateEdit・暦 Calendar・色を選ぶダイアログ ColorDialog・色のボタン ColorButton を加える

- 状態: 承認
- 日付: 2026-10-11

## 背景

フォームでよく使う、日付と色の入力が無かった。利用者は LineEdit に日付を打ち込ませて自分で読むか、色を数値の欄で入れてもらうしかなかった。
日付を表す値の型も無かった。

## 検討した選択肢

- 日付の入力欄: 書式を固定する / 書式を指定できる(表示と、打ち込んだ文字を読む順)。
- 色: ダイアログだけ / ダイアログと、押すとそれを開く色のボタン。
- デザイナーでの値の書き方: 日付・色をメンバ(年・月・日、r・g・b・a)のオブジェクトで書く / 文字(「2026-10-11」「#2563EB」)で書く。

## 決定

書式を指定できる DateEdit と、ColorDialog・ColorButton を作る。デザイナーでは文字で書く。

- **Date**(`core/date.hpp`): 年・月・日(グレゴリオ暦。1〜9999 年)。すべて 0 は空(`is_null`)。`today`・`day_of_week`・`add_days`・
  `add_months`(日が無くなったら月末)・`add_years`・`days_to`・`format`(yyyy・yy・MM・M・dd・d)・`parse`(数字の並び 3 つを書式の順に読む。
  区切りは何でもよい。2 桁以下の年は 2000 年代)。比べる演算子はメンバでない関数にし、`Property<Date>` とも比べられる。
- **Calendar**(`widgets/calendar.hpp`): 月ごとの暦。`date`・`minimum`・`maximum`・`first_day_of_week`(既定は日曜日)・`shown_year`・`shown_month`。
  日を押すと選んで `on_activate`、「<」「>」・ホイール・Page Up/Down で月、Shift+Page Up/Down で年を変え、矢印で日を動かす。
  今日の日は輪で示す。月の名前・曜日は言語に合わせる(日本語なら「2026年10月」「日 月 …」)。
- **DateEdit**(`widgets/date_edit.hpp`): 入力欄と暦のボタン。`date`(既定は空)・`minimum`・`maximum`・`format`(既定 "yyyy/MM/dd")・
  `first_day_of_week`・`placeholder_text`。Enter・フォーカスが外れたときに読み、空なら空の日付、読めなければ元に戻し、範囲の外は範囲に収める。
  ↑↓・ホイールはキャレットのある欄(年・月・日)を増減し、Page Up/Down は月を増減する。暦はボタン・Alt+↓・F4 で開き、Esc で閉じる。
- **ColorDialog**(`windows/color_dialog.hpp`): 彩度・明度の四角と色相の帯、16 進数・R・G・B の数値、よく使う色の見本 16 色、選んでいる色と
  開いたときの色の比較(下を押すと戻す)。`alpha_enabled` なら不透明度の帯と数値も出す(16 進数は #RRGGBBAA)。灰色を選んでも色相を覚えておく。
  `show` は MessageDialog と同じく、閉じたら `on_finish` を呼んで破棄する。
- **ColorButton**(`widgets/color_button.hpp`): 色の見本と文字(空なら 16 進数)。押すと ColorDialog を開き、OK で `color` にする。
  `alpha_enabled` はダイアログで不透明度を選ぶかだけで、代入した色の不透明度は変えない(生成するコードの代入の順に依らないように)。
- 利用者の言語を `Application::language`(既定は OS の設定の最初の言語。「ja」「en-US」など)にし、標準の文字(ダイアログのボタン・
  暦の月と曜日)はこれで選ぶ(「ja」で始まれば日本語、ほかは英語)。アプリが言語を固定することもできる。
  WebAssembly(デザイナーのキャンバス)では SDL の SDL_GetPreferredLocales が navigator の無い環境(VS Code の拡張の Node.js)で止まるので、
  JavaScript の navigator か Intl から調べ、拡張からも言語を渡す。
- ウィンドウを中身の最小の大きさまで広げたときは、OS からの知らせを待たずに大きさを合わせて配置し直す
  (知らせを処理しない capture・デザイナーのキャンバスで、広がった部分が描かれなかったため)。
- デザイナー: Date は `"2026-10-11"`(空の文字は空の日付)、Color は `"#RRGGBB"`・`"#RRGGBBAA"`(メンバのオブジェクトも書ける)で書く。
  インスペクタは日付の欄・色を選ぶ欄を出す。DateEdit・Calendar・ColorButton はパレットの「入力」に置く。

## 影響

- 時刻・日時、和暦の表示、月の名前・曜日の英語・日本語以外の言語は対象外(必要になったら追加する)。
- 暦の今日の印・既定で見せる月は、実行する日に依る(デザイナーのキャンバスでも、今日の月を見せる)。
- 画面の色を拾う(スポイト)・最近使った色を覚える機能は対象外。
