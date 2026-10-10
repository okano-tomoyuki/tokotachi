// 一覧と木: ListView(列・並べ替え・チェック)と TreeView を Splitter で並べる。
#include "gallery.hpp"

int main(int argc, char** argv)
{
    tokotachi::Application app(argc, argv);
    tokotachi::Window window("一覧と木", 560, 280);
    tokotachi::BoxLayout* box = new tokotachi::BoxLayout(&window);
    box->margins = tokotachi::Margins(12);

    // 木: 項目をオブジェクトで持つ TreeModel。
    tokotachi::TreeModel* folders = new tokotachi::TreeModel(&window);
    tokotachi::TreeItem* documents = folders->add_item("文書");
    documents->add_child("仕事")->add_child("報告書");
    documents->add_child("家計簿");
    folders->add_item("画像")->add_child("旅行");
    folders->add_item("音楽");
    tokotachi::TreeView* tree = new tokotachi::TreeView();
    tree->model = folders;
    tree->expand_all();

    // 一覧: 列と見出し。見出しを押すと並べ替える。
    tokotachi::ListModel* files = new tokotachi::ListModel(&window);
    const char* const ROWS[][3] = {{"報告書.txt", "12 KB", "2026-10-01"},
                                   {"見積もり.xlsx", "48 KB", "2026-09-30"},
                                   {"議事録.docx", "31 KB", "2026-10-03"},
                                   {"README.md", "3 KB", "2026-08-05"}};
    for (int i = 0; i < 4; ++i)
    {
        tokotachi::Item* item = files->add_item(ROWS[i][0]);
        item->column_texts = std::vector<std::string>{ROWS[i][1], ROWS[i][2]};
        item->checkable = true;
        item->checked = i == 1;
    }
    tokotachi::ListView* list = new tokotachi::ListView();
    list->add_column("名前", 150);
    list->add_column("大きさ", 70)->alignment = tokotachi::Align::END;
    list->add_column("更新日", 100);
    list->sorting_enabled = true;
    list->selection_mode = tokotachi::SelectionMode::MULTI;
    list->model = files;
    list->select(files->index(2));

    tokotachi::Splitter* split = new tokotachi::Splitter(&window);
    split->add_widget(tree, 0);
    split->add_widget(list, 1);
    split->sizes = std::vector<float>{160, 360};
    box->add(split, 1);
    return run_gallery(app, window, argc, argv);
}
