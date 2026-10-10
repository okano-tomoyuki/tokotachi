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

tokotachi::ListView* view = new tokotachi::ListView(&window);
view->add_column("名前", 200);
view->add_column("大きさ", 80)->alignment = tokotachi::Align::END;
view->model = files;
view->selection_mode = tokotachi::SelectionMode::MULTI;
view->sorting_enabled = true;        // 見出しを押すと並べ替える
view->on_activate = [=](tokotachi::Object*) { ... files->item_at(view->current) ... };
```

```cpp
tokotachi::TreeModel* tree = new tokotachi::TreeModel(&window);
tokotachi::TreeItem* docs = tree->add_item("文書");
docs->add_child("報告書.txt");
tokotachi::TreeView* view = new tokotachi::TreeView(&window);
view->model = tree;
view->expand(tree->index_of(docs));
```

## 選択

- `selection_mode`: NONE・SINGLE・MULTI(Ctrl・Shift+クリック、Shift+矢印、Ctrl+A)。
- `current` はキーで動かす位置、`selection` は選んでいる項目(表示の順)。`on_selection_change`・`on_activate`(ダブルクリック・Enter)。
- 項目を加える・除く・並べ替えても、選択は同じ項目に付いたままです。

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
