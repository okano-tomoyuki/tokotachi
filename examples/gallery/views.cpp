// 一覧と木: ListView(列・並べ替え・チェック・文字の色と太さ・編集)と TreeView を Splitter で並べる。
// 項目は F2 か、選んでいる項目のクリックで名前を変えられる。押した文字で始まる項目へも動ける。
// どちらもドラッグで並べ替えられる(木では項目の上に落とすと子にする)。
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
    documents->add_child("家計簿")->text_color = tokotachi::ThemeColor(tokotachi::Color(0xDC, 0x26, 0x26));
    folders->add_item("画像")->add_child("旅行");
    folders->add_item("音楽");
    tokotachi::TreeView* tree_view_folders = new tokotachi::TreeView();
    tree_view_folders->model = folders;
    tree_view_folders->drag_enabled = true;
    tree_view_folders->drop_enabled = true;
    tree_view_folders->expand_all();

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
        item->editable = true;
    }
    // 項目ごとの文字の色・太さ(docs/adr/0046)
    files->items.get()[0]->font = tokotachi::Font(14, true);
    files->items.get()[3]->text_color = tokotachi::ThemeColor(tokotachi::ColorRole::TEXT_SECONDARY);
    tokotachi::ListView* list_view_files = new tokotachi::ListView();
    list_view_files->add_column("名前", 150);
    list_view_files->add_column("大きさ", 70)->alignment = tokotachi::Align::END;
    list_view_files->add_column("更新日", 100);
    list_view_files->sorting_enabled = true;
    list_view_files->selection_mode = tokotachi::SelectionMode::MULTI;
    list_view_files->model = files;
    list_view_files->drag_enabled = true;
    list_view_files->drop_enabled = true;
    list_view_files->select(files->index(2));

    tokotachi::Splitter* splitter_main = new tokotachi::Splitter(&window);
    splitter_main->add_widget(tree_view_folders, 0);
    splitter_main->add_widget(list_view_files, 1);
    splitter_main->sizes = std::vector<float>{160, 360};
    box->add(splitter_main, 1);
    return run_gallery(app, window, argc, argv);
}
