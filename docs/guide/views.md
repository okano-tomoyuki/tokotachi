# 一覧と木 {#views_guide}

[TOC]

![一覧と木](views.png)

一覧(`ListView`)と木(`TreeView`)は、データを**モデル**に持ち、ビューがそれを表示します。

## 手軽に使う: ListModel・TreeModel

項目をオブジェクト(`Item`・`TreeItem`)で持つモデルです。項目のプロパティを変えると、表示しているビューが描き直します。

```cpp
tokotachi::ListModel* files = new tokotachi::ListModel(&window);
tokotachi::Item* item = files->add_item("報告書.txt");
item->column_texts = std::vector<std::string>{"12 KB", "2026-10-01"};   // 2 列目以降
item->checkable = true;

tokotachi::ListView* list_view_files = new tokotachi::ListView(&window);
list_view_files->add_column("名前", 200);
list_view_files->add_column("大きさ", 80)->alignment = tokotachi::Align::END;
list_view_files->model = files;
list_view_files->selection_mode = tokotachi::SelectionMode::MULTI;
list_view_files->sorting_enabled = true;        // 見出しを押すと並べ替える
list_view_files->on_activate = [=](tokotachi::Object*) { ... files->item_at(list_view_files->current) ... };
```

```cpp
tokotachi::TreeModel* tree = new tokotachi::TreeModel(&window);
tokotachi::TreeItem* docs = tree->add_item("文書");
docs->add_child("報告書.txt");
tokotachi::TreeView* tree_view_docs = new tokotachi::TreeView(&window);
tree_view_docs->model = tree;
tree_view_docs->expand(tree->index_of(docs));
```

## 選択

- `selection_mode`: NONE・SINGLE・MULTI(Ctrl・Shift+クリック、Shift+矢印、Ctrl+A)。
- `current` はキーで動かす位置、`selection` は選んでいる項目(表示の順)。`on_selection_change`・`on_activate`(ダブルクリック・Enter)。
- 項目を加える・除く・並べ替えても、選択は同じ項目に付いたままです。

## 文字の色・太さ、文字での検索、その場での編集

- 項目ごとに文字の色(`text_color`)とフォント(`font`。太字・斜体も)を変えられます。フォーカスのあるビューで選んでいる行・無効な項目は、
  ビューの色で描きます。
- 項目を選んでいるときに文字のキーを押すと、その文字で始まる項目へ動きます(続けて押すと前方一致、同じ文字を押すと次の項目へ)。
- `editable` の項目は、F2 か、選んでいる項目のクリックで文字を編集できます。Enter・ほかへのフォーカスの移動・スクロールで確定し、Esc でやめます。
  コードからは `edit(index, column)` で始めます。

```cpp
item->font = tokotachi::Font(14, true);                                        // 太字
item->text_color = tokotachi::ThemeColor(tokotachi::ColorRole::TEXT_SECONDARY);  // 薄い文字
item->editable = true;                                                          // 名前を変えられる
```

`ItemModel` を派生したときは、`text_color`・`font`・`editable`・`set_text` を上書きします。

## 大量のデータ: ItemModel を派生する

ビューは**見えている行だけ**を描き、そのときにモデルへ問い合わせます。数十万行のデータは、項目のオブジェクトを作らずに
`ItemModel` を派生して `row_count` と `text` を実装します。

```cpp
class LogModel : public tokotachi::ItemModel
{
public:
    int row_count(const tokotachi::ModelIndex& parent) const override
    {
        return parent.is_valid() ? 0 : static_cast<int>(lines.size());
    }
    std::string text(const tokotachi::ModelIndex& index, int column) const override
    {
        return lines[index.row()];
    }
    std::vector<std::string> lines;
};
```

データを変えたら、`notify_rows_inserted`・`notify_rows_removing`・`notify_data_changed`・`notify_reset` で知らせます。
