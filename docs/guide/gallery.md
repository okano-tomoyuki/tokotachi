# サンプルの一覧 {#gallery}

[TOC]

部品ごとの小さなサンプルです。ソースはリポジトリの `examples/gallery/` にあり、`TOKOTACHI_BUILD_EXAMPLES` でビルドされます。
画面の画像は、リファレンスを作るときにサンプルを実際に動かして撮っています。

## ボタン

Button(PRIMARY・SECONDARY・無効)、CheckBox、RadioButton(変数に結んだグループ)。

![ボタン](buttons.png)

@include buttons.cpp

## 入力

LineEdit、ComboBox、SpinBox、TextEdit、Slider と ProgressBar(同じ変数に結ぶ)。GridLayout で項目名と並べています。

![入力](inputs.png)

@include inputs.cpp

## 一覧と木

TreeView と ListView(列・並べ替え・チェック・複数選択)を Splitter で並べています。

![一覧と木](views.png)

@include views.cpp

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
