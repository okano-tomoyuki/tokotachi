# サンプルの一覧 {#gallery}

[TOC]

部品ごとの小さなサンプルです。ソースはリポジトリの `examples/gallery/` にあり、`TOKOTACHI_BUILD_EXAMPLES` でビルドされます。
画面の画像は、リファレンスを作るときにサンプルを実際に動かして撮っています。

## ボタン

Button(PRIMARY・SECONDARY・無効)、CheckBox、RadioButton(変数に結んだグループ)を GroupBox(見出しの付いた枠)にまとめています。
「(&S)」などは Alt と一緒に押すキー(ニーモニック)です。

![ボタン](buttons.png)

@include buttons.cpp

## 入力

LineEdit、ComboBox、SpinBox(単位)、TextEdit、Slider(目盛り)と ProgressBar(同じ変数に結ぶ)、進み具合の分からない ProgressBar。
GridLayout で項目名と並べ、項目名のニーモニック(Alt+N など)で入力欄へ移ります。

![入力](inputs.png)

@include inputs.cpp

## 一覧と木

TreeView と ListView(列・並べ替え・チェック・複数選択)を Splitter で並べています。

![一覧と木](views.png)

@include views.cpp

## 表

TableView(セルの矩形の選択・その場での編集・写す・貼る)。点を書き換えると、モデルが合計を計算し直します。

![表](table.png)

@include table.cpp

## ウィンドウの部品

メニューバー・ツールバー・ステータスバー・タブ。メニューとツールバーは同じ Action を共有しています。

![ウィンドウの部品](window.png)

@include window.cpp

## ダイアログ

![ダイアログ](dialog.png)

@include dialog.cpp

## 描画

PaintBox の Canvas に、棒グラフ・円グラフ・回転とグラデーション・グループの透過を描いています。

![描画](drawing.png)

@include drawing.cpp

## ドラッグ&ドロップ

エクスプローラー・ファイルマネージャーからファイルや文字を落とします。左の枠はドラッグが来ると色を変え、落としたものを表示します。
右の一覧は落としたファイルを加え、中の項目はドラッグで並べ替えられます。

![ドラッグ&ドロップ](drop.png)

@include drop.cpp

## 画像

`ImageBox` の合わせ方(`fit`)を並べ、ラベル・ボタン・メニュー・ツールバーに画像を出します。

![画像](images.png)

@include images.cpp
