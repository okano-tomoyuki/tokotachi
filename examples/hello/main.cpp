// gui ライブラリのサンプル: Label・Button・LineEdit・CheckBox・RadioButton・TabWidget など、変数による結びつけ、レイアウト。
//   hello                     ウィンドウを開く
//   hello --screenshot a.png  1 回描いて PNG に保存して終える(--dark を先に付けるとダークのテーマで描く。
//                             --tab N を先に付けると N 番目のタブを選んで描く)
#include <gui/gui.hpp>

#include <cstdint>
#include <cstdlib>
#include <string>
#include <vector>

// 16 × 16 の簡単なアイコン(フォルダー・ファイル)を画素から作る(普通は gui::Image::load で画像ファイルを読む)。
gui::Image make_icon(bool folder)
{
    std::vector<gui::Color> pixels(16 * 16, gui::Color(0, 0, 0, 0));
    const gui::Color fill = folder ? gui::Color::rgb(0xf6c344) : gui::Color::rgb(0xffffff);
    const gui::Color edge = folder ? gui::Color::rgb(0xc98f0a) : gui::Color::rgb(0x6b7280);
    for (int y = 0; y < 16; ++y)
    {
        for (int x = 0; x < 16; ++x)
        {
            bool inside, border;
            if (folder)
            {
                inside = (y >= 4 && y <= 13 && x >= 1 && x <= 14) || (y >= 2 && y < 4 && x >= 1 && x <= 6);
                border = inside && (y == 13 || x == 1 || x == 14 || (y == 2 && x <= 6) || (y == 4 && x >= 6) || (x == 6 && y < 4));
            }
            else
            {
                inside = y >= 1 && y <= 14 && x >= 3 && x <= 12;
                border = inside && (y == 1 || y == 14 || x == 3 || x == 12);
                if (inside && !border && (y == 5 || y == 8 || y == 11) && x >= 5 && x <= 10)
                {
                    border = true;  // 文字の行
                }
            }
            if (inside)
            {
                pixels[y * 16 + x] = border ? edge : fill;
            }
        }
    }
    return gui::Image::from_pixels(16, 16, pixels);
}

int main(int argc, char** argv)
{
    gui::Application app(argc, argv);
    gui::Window window("hello - gui", 560, 620);

    // 配置はレイアウトに任せる(docs/adr/0017)。ウィンドウは縦に並べ、横に並べる行は入れ物のウィジェットを介して入れ子にする。
    // 行の中のウィジェットは行を親にして作る(別の親で作っても、add が行の子に付け替える。docs/adr/0018)。
    // フォームはスクロール領域に入れる(ウィンドウを小さくすると縦にスクロールする。docs/adr/0021)。
    gui::BoxLayout* window_box = new gui::BoxLayout(&window);
    window_box->margins = gui::Margins(0);
    gui::ScrollArea* scroll = new gui::ScrollArea(&window);
    window_box->add(scroll, 1);
    gui::Widget* form = new gui::Widget();
    scroll->content = form;
    gui::BoxLayout* column = new gui::BoxLayout(form);
    column->margins = gui::Margins(24);
    column->spacing = 12;

    gui::Label* title = new gui::Label(&window, "gui ライブラリのサンプル");
    title->font = gui::Font(22);
    gui::Label* description = new gui::Label(
        &window, "このラベルは word_wrap で、ウィンドウの幅に合わせて折り返す。日本語の禁則(行の始めに「、」や「。」を置かない)にも従う。"
                 " English text wraps at spaces.");
    description->word_wrap = true;
    description->color = gui::ColorRole::TEXT_SECONDARY;  // テーマの「控えめな文字」(ダークでも読める)

    gui::Widget* click_row = new gui::Widget(&window);
    gui::BoxLayout* click_box = new gui::BoxLayout(click_row, gui::Orientation::HORIZONTAL);
    click_box->spacing = 16;
    gui::Button* button = new gui::Button(click_row, "クリック");
    button->minimum_size = gui::Size(140, 36);
    gui::Label* count = new gui::Label(click_row, "クリック回数: 0");
    click_box->add(button, 0, gui::Align::CENTER);
    click_box->add(count, 1);

    gui::Label* caption = new gui::Label(&window, "名前(IME で日本語も入力できる。Tab でフォーカスを移す)");
    caption->font = gui::Font(12);
    caption->color = gui::ColorRole::TEXT_SECONDARY;
    gui::LineEdit* edit = new gui::LineEdit(&window);
    edit->placeholder_text = "ここに入力";

    // 敬称はラジオボタンのグループ。各ボタンの group_value を同じ変数に結ぶ。
    gui::Widget* option_row = new gui::Widget(&window);
    gui::BoxLayout* option_box = new gui::BoxLayout(option_row, gui::Orientation::HORIZONTAL);
    option_box->spacing = 16;
    gui::RadioButton* san = new gui::RadioButton(option_row, "さん", 1);
    gui::RadioButton* sama = new gui::RadioButton(option_row, "様", 2);
    gui::RadioButton* kun = new gui::RadioButton(option_row, "くん", 3);
    gui::CheckBox* big = new gui::CheckBox(option_row, "大きい文字");
    option_box->add(san);
    option_box->add(sama);
    option_box->add(kun);
    option_box->add_spacing(16);
    option_box->add(big);
    option_box->add_stretch();

    gui::Label* greeting = new gui::Label(&window);

    gui::Button* disabled = new gui::Button(&window, "無効なボタン");
    disabled->minimum_size = gui::Size(140, 36);
    disabled->enabled = false;

    column->add(title);
    column->add(description);
    column->add(click_row);
    column->add(caption);
    column->add(edit);
    column->add(option_row);
    gui::Widget* color_row = new gui::Widget(&window);
    gui::BoxLayout* color_box = new gui::BoxLayout(color_row, gui::Orientation::HORIZONTAL);
    color_box->add(new gui::Label(color_row, "挨拶の色"), 0, gui::Align::CENTER);
    gui::ComboBox* color = new gui::ComboBox(color_row);
    std::vector<std::string> colors;
    colors.push_back("黒");
    colors.push_back("青");
    colors.push_back("緑");
    colors.push_back("赤");
    color->items = colors;
    color->current_index = 0;
    color_box->add(color);
    color_box->add_spacing(24);
    // テーマ(docs/adr/0023)。既定は OS の設定に従う。
    color_box->add(new gui::Label(color_row, "テーマ"), 0, gui::Align::CENTER);
    gui::ComboBox* scheme = new gui::ComboBox(color_row);
    std::vector<std::string> schemes;
    schemes.push_back("OS に従う");
    schemes.push_back("ライト");
    schemes.push_back("ダーク");
    scheme->items = schemes;
    scheme->current_index = 0;
    scheme->on_change = [=](gui::Object*)
    {
        static const gui::ColorScheme SCHEMES[] = {gui::ColorScheme::SYSTEM, gui::ColorScheme::LIGHT,
                                                   gui::ColorScheme::DARK};
        const int i = scheme->current_index;
        gui::Application::instance()->color_scheme = SCHEMES[i >= 0 ? i : 0];
    };
    color_box->add(scheme);
    color_box->add_stretch();
    column->add(color_row);
    column->add(greeting);

    // タブ(docs/adr/0025)。ページごとにレイアウトを持たせ、TabWidget に加える。Ctrl+Tab で切り替えられる。
    gui::TabWidget* tabs = new gui::TabWidget(&window);
    gui::Widget* memo_page = new gui::Widget();
    gui::BoxLayout* memo_box = new gui::BoxLayout(memo_page);
    memo_box->margins = gui::Margins(0, 12, 0, 0);
    memo_box->spacing = 8;
    memo_box->add(new gui::Label(memo_page, "メモ(複数行。Enter で改行。右クリックでメニュー)"));
    gui::TextEdit* memo =
        new gui::TextEdit(memo_page, "TextEdit は複数行の入力欄。折り返し・縦のスクロール・IME に対応する。\n2 行目");
    memo_box->add(memo, 1);
    tabs->add_tab(memo_page, "メモ");

    // 値の部品(docs/adr/0024)。Slider・SpinBox・ProgressBar を同じ変数に結び、どれを動かしても揃う。
    gui::Widget* level_page = new gui::Widget();
    gui::BoxLayout* level_box = new gui::BoxLayout(level_page, gui::Orientation::HORIZONTAL);
    level_box->margins = gui::Margins(0, 12, 0, 0);
    level_box->spacing = 12;
    gui::Slider* level_slider = new gui::Slider(level_page);
    gui::SpinBox* level_spin = new gui::SpinBox(level_page);
    gui::ProgressBar* level_bar = new gui::ProgressBar(level_page);
    level_bar->text_visible = true;
    level_box->add(new gui::Label(level_page, "進み具合"), 0, gui::Align::START);
    level_box->add(level_slider, 1, gui::Align::START);
    level_box->add(level_spin, 0, gui::Align::START);
    level_box->add(level_bar, 1, gui::Align::START);
    gui::DoubleVariable* level = new gui::DoubleVariable(&window, 40);
    gui::bind(level_slider->value, level->value);
    gui::bind(level_spin->value, level->value);
    level->on_change = [=](gui::Object*) { level_bar->value = level->value.get(); };
    level_bar->value = level->value.get();
    level_slider->tooltip = "ドラッグ・矢印キーで動かす";
    tabs->add_tab(level_page, "進み具合");

    // 一覧と木(docs/adr/0026)。項目はモデル(ListModel・TreeModel)に持たせ、ビューに表示させる。
    const gui::Image folder_icon = make_icon(true);
    const gui::Image file_icon = make_icon(false);
    gui::ListModel* files = new gui::ListModel(&window);
    static const char* const FILES[][3] = {{"報告書.txt", "12 KB", "2026-10-01"},   {"写真.png", "2.4 MB", "2026-09-12"},
                                           {"見積もり.xlsx", "48 KB", "2026-09-30"}, {"README.md", "3 KB", "2026-08-05"},
                                           {"議事録.docx", "31 KB", "2026-10-03"},   {"build.log", "880 KB", "2026-10-04"},
                                           {"予定表.ics", "6 KB", "2026-07-21"},     {"archive.zip", "15 MB", "2026-06-30"}};
    for (std::size_t i = 0; i < sizeof(FILES) / sizeof(FILES[0]); ++i)
    {
        gui::Item* item = files->add_item(FILES[i][0]);
        item->column_texts = std::vector<std::string>{FILES[i][1], FILES[i][2]};
        item->icon = file_icon;
        item->checkable = true;
    }
    gui::Widget* list_page = new gui::Widget();
    gui::BoxLayout* list_box = new gui::BoxLayout(list_page);
    list_box->margins = gui::Margins(0, 12, 0, 0);
    list_box->spacing = 8;
    gui::ListView* file_list = new gui::ListView(list_page);
    file_list->add_column("名前", 200);
    file_list->add_column("大きさ", 90)->alignment = gui::Align::END;
    file_list->add_column("更新日", 110);
    file_list->sorting_enabled = true;  // 見出しを押すと並べ替える
    file_list->selection_mode = gui::SelectionMode::MULTI;
    file_list->model = files;
    gui::Label* list_status = new gui::Label(list_page, "Ctrl・Shift+クリックで複数選べる。ダブルクリック・Enter で開く");
    list_status->color = gui::ColorRole::TEXT_SECONDARY;
    file_list->on_selection_change = [=](gui::Object*)
    { list_status->text = std::to_string(file_list->selection.get().size()) + " 個を選んでいる"; };
    file_list->on_activate = [=](gui::Object*)
    { list_status->text = "開く: " + files->text(file_list->current, 0); };
    list_box->add(file_list, 1);
    list_box->add(list_status);
    tabs->add_tab(list_page, "一覧");

    gui::TreeModel* tree = new gui::TreeModel(&window);
    gui::TreeItem* documents = tree->add_item("文書");
    documents->icon = folder_icon;
    gui::TreeItem* work = documents->add_child("仕事");
    work->icon = folder_icon;
    work->add_child("報告書.txt")->icon = file_icon;
    work->add_child("見積もり.xlsx")->icon = file_icon;
    documents->add_child("README.md")->icon = file_icon;
    gui::TreeItem* pictures = tree->add_item("画像");
    pictures->icon = folder_icon;
    for (int i = 1; i <= 30; ++i)
    {
        pictures->add_child("写真 " + std::to_string(i) + ".png")->icon = file_icon;
    }
    gui::TreeItem* empty = tree->add_item("空のフォルダー");
    empty->icon = folder_icon;
    gui::TreeView* tree_view = new gui::TreeView();
    tree_view->model = tree;
    tree_view->expand(tree->index_of(documents));
    tabs->add_tab(tree_view, "木");

    tabs->add_tab(new gui::Widget(), "無効なタブ")->enabled = false;
    tabs->minimum_size = gui::Size(0, 240);
    column->add(tabs, 1);
    column->add(disabled, 0, gui::Align::START);  // 横に広げない

    // 画面の状態は変数に持ち、ウィジェットと結ぶ(docs/adr/0016)。
    gui::StringVariable* name = new gui::StringVariable(&window);
    gui::IntVariable* honorific = new gui::IntVariable(&window, 1);
    gui::BoolVariable* large = new gui::BoolVariable(&window, false);
    gui::bind(edit->text, name->value);
    gui::bind(san->group_value, honorific->value);
    gui::bind(sama->group_value, honorific->value);
    gui::bind(kun->group_value, honorific->value);
    gui::bind(big->checked, large->value);

    // どの変数が変わっても挨拶を作り直す(1 つのハンドラを 3 つの変数で共有する)。
    const gui::NotifyHandler update_greeting = [=](gui::Object*)
    {
        static const char* const SUFFIX[] = {"", "さん", "様", "くん"};
        const std::string n = name->value;
        greeting->text = n.empty() ? std::string() : "こんにちは、" + n + " " + SUFFIX[honorific->value.get()];
        greeting->font = gui::Font(large->value ? 20.0f : 14.0f);
        // 「黒」はテーマの文字の色にする(ダークでは明るい色になる)。他は固定の色。
        const int c = color->current_index;
        static const std::uint32_t RGB[] = {0, 0x2563eb, 0x059669, 0xdc2626};
        greeting->color = c <= 0 ? gui::ThemeColor(gui::ColorRole::TEXT) : gui::ThemeColor(gui::Color::rgb(RGB[c]));
    };
    name->on_change = update_greeting;
    honorific->on_change = update_greeting;
    large->on_change = update_greeting;
    color->on_change = update_greeting;

    int clicks = 0;
    button->on_click = [&](gui::Object*)
    {
        ++clicks;
        count->text = "クリック回数: " + std::to_string(clicks);
    };
    edit->on_submit = [=](gui::Object*) { edit->select_all(); };
    button->tooltip = "押した回数を数える";
    disabled->tooltip = "無効なウィジェットにもツールチップは出る";

    // メモの右クリックのメニュー(docs/adr/0022)。メニューはウィンドウが所有する。
    gui::Menu* memo_menu = new gui::Menu(&window);
    memo_menu->add_item("切り取り", [=](gui::Object*) { memo->cut(); })->shortcut_text = "Ctrl+X";
    memo_menu->add_item("コピー", [=](gui::Object*) { memo->copy(); })->shortcut_text = "Ctrl+C";
    memo_menu->add_item("貼り付け", [=](gui::Object*) { memo->paste(); })->shortcut_text = "Ctrl+V";
    memo_menu->add_separator();
    memo_menu->add_item("すべて選択", [=](gui::Object*) { memo->select_all(); })->shortcut_text = "Ctrl+A";
    gui::Menu* insert_menu = memo_menu->add_submenu("挿入");
    insert_menu->add_item("挨拶", [=](gui::Object*) { memo->insert(greeting->text.get()); });
    insert_menu->add_item("クリック回数", [=](gui::Object*) { memo->insert(count->text.get()); });
    memo->on_mouse_down = [=](gui::Object*, gui::MouseEvent& e)
    {
        if (e.button == gui::MouseButton::RIGHT)
        {
            memo_menu->popup(memo, e.pos);
        }
    };

    edit->focused = true;
    window.visible = true;

    for (int i = 1; i < argc; ++i)
    {
        if (std::string(argv[i]) == "--dark")
        {
            app.color_scheme = gui::ColorScheme::DARK;
            scheme->current_index = 2;
        }
        else if (std::string(argv[i]) == "--tab" && i + 1 < argc)
        {
            tabs->current_index = std::atoi(argv[++i]);
        }
        else if (std::string(argv[i]) == "--screenshot" && i + 1 < argc)
        {
            name->value = "Ünïcödé と日本語 AVA ffi";  // 変数への代入が入力欄と挨拶に伝わる
            honorific->value = 2;
            large->value = true;
            edit->select_all();
            color->current_index = 1;
            return window.save_png(argv[i + 1]) ? 0 : 1;
        }
    }
    return app.exec();
}
