// WebAssembly の試作(docs/adr/0039): Tokotachi を Node.js の中で動かし、画面なしでウィンドウを描けるかを確かめる。
//   node spike_wasm.js <out.png>
// 1. ウィンドウを組んで PNG に保存する
// 2. 各部品の位置(ウィンドウの座標)を JSON で標準出力に出す(デザイナーの選択の枠に使う情報)
// 3. 描き直し・組み直しにかかる時間を測る
#include <chrono>
#include <cstdio>
#include <string>
#include <vector>

#include <tokotachi/tokotachi.hpp>

using namespace tokotachi;

namespace
{

double now_ms()
{
    return std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now().time_since_epoch()).count();
}

// examples/gallery の views と inputs を合わせたような画面。
Window* build(Window* window)
{
    BoxLayout* box = new BoxLayout(window);
    box->margins = Margins(12);

    Widget* row = new Widget();
    BoxLayout* row_box = new BoxLayout(row, Orientation::HORIZONTAL);
    LineEdit* edit = new LineEdit();
    edit->placeholder_text = "検索する言葉";
    row_box->add(edit, 1);
    Button* search = new Button(nullptr, "検索");
    row_box->add(search);
    box->add(row);

    TreeModel* folders = new TreeModel(window);
    TreeItem* documents = folders->add_item("文書");
    documents->add_child("仕事")->add_child("報告書");
    documents->add_child("家計簿");
    folders->add_item("画像")->add_child("旅行");
    TreeView* tree = new TreeView();
    tree->model = folders;
    tree->expand_all();

    ListModel* files = new ListModel(window);
    const char* const ROWS[][3] = {{"報告書.txt", "12 KB", "2026-10-01"},
                                   {"見積もり.xlsx", "48 KB", "2026-09-30"},
                                   {"议事录 / 회의록 / محضر", "31 KB", "2026-10-03"}};
    for (int i = 0; i < 3; ++i)
    {
        Item* item = files->add_item(ROWS[i][0]);
        item->column_texts = std::vector<std::string>{ROWS[i][1], ROWS[i][2]};
        item->checkable = true;
        item->checked = i == 1;
    }
    ListView* list = new ListView();
    list->add_column("名前", 180);
    list->add_column("大きさ", 70)->alignment = Align::END;
    list->add_column("更新日", 100);
    list->model = files;
    list->select(files->index(1));

    Splitter* split = new Splitter(window);
    split->add_widget(tree, 0);
    split->add_widget(list, 1);
    split->sizes = std::vector<float>{150, 380};
    box->add(split, 1);

    Widget* buttons = new Widget();
    BoxLayout* buttons_box = new BoxLayout(buttons, Orientation::HORIZONTAL);
    buttons_box->add_stretch();
    buttons_box->add(new CheckBox(nullptr, "確認する"));
    Button* ok = new Button(nullptr, "OK");
    buttons_box->add(ok);
    box->add(buttons);
    return window;
}

void print_rects(Widget* w, float ox, float oy, int depth)
{
    const Rect b = w->bounds;
    const float x = ox + b.x;
    const float y = oy + b.y;
    std::printf("%*s{\"x\": %.1f, \"y\": %.1f, \"w\": %.1f, \"h\": %.1f, \"children\": %d}\n", depth * 2, "", x, y,
                b.width, b.height, static_cast<int>(w->children.get().size()));
    const std::vector<Widget*> children = w->children;
    for (std::size_t i = 0; i < children.size(); ++i)
    {
        print_rects(children[i], x, y, depth + 1);
    }
}

}  // namespace

int main(int argc, char** argv)
{
    const std::string out = argc > 1 ? argv[1] : "spike_wasm.png";
    double t0 = now_ms();
    Application app(argc, argv);
    app.color_scheme = ColorScheme::LIGHT;
    double t1 = now_ms();
    std::printf("Application: %.1f ms\n", t1 - t0);

    Window* window = build(new Window("試作", 560, 320));
    window->visible = true;
    double t2 = now_ms();
    std::printf("build + show: %.1f ms\n", t2 - t1);

    if (!window->save_png(out))
    {
        return 1;
    }
    double t3 = now_ms();
    std::printf("first render + png: %.1f ms\n", t3 - t2);

    std::printf("rects (window coordinates, dp):\n");
    const std::vector<Widget*> top = window->children;
    for (std::size_t i = 0; i < top.size(); ++i)
    {
        print_rects(top[i], 0, 0, 1);
    }

    // 描き直し(デザイナーでプロパティを 1 つ変えたとき)。
    const int N = 20;
    t0 = now_ms();
    for (int i = 0; i < N; ++i)
    {
        window->title = "試作 " + std::to_string(i);
        window->save_png(out);
    }
    std::printf("re-render + png: %.1f ms / frame\n", (now_ms() - t0) / N);

    // 組み直し(デザイナーで部品を足す・消すたびに画面を組み直す場合)。
    t0 = now_ms();
    for (int i = 0; i < N; ++i)
    {
        Window* w = build(new Window("試作", 560, 320));
        w->visible = true;
        w->save_png(out + ".tmp.png");
        delete w;
    }
    std::printf("rebuild + render + png: %.1f ms / frame\n", (now_ms() - t0) / N);
    delete window;
    return 0;
}
