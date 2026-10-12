// Tokotachi のサンプル: Label・Button・LineEdit・CheckBox・RadioButton・TabWidget など、変数による結びつけ、レイアウト。
//   hello                     ウィンドウを開く
//   hello --screenshot a.png  1 回描いて PNG に保存して終える(--dark を先に付けるとダークのテーマで描く。
//                             --tab N を先に付けると N 番目のタブを選んで描く)
#include <tokotachi/tokotachi.hpp>

#include <chrono>
#include <cstdint>
#include <cstdlib>
#include <fstream>
#include <memory>
#include <sstream>
#include <string>
#include <thread>
#include <vector>

// 16 × 16 の簡単なアイコン(フォルダー・ファイル)を画素から作る(普通は tokotachi::Image::load で画像ファイルを読む)。
tokotachi::Image make_icon(bool folder)
{
    std::vector<tokotachi::Color> pixels(16 * 16, tokotachi::Color(0, 0, 0, 0));
    const tokotachi::Color fill = folder ? tokotachi::Color::rgb(0xf6c344) : tokotachi::Color::rgb(0xffffff);
    const tokotachi::Color edge = folder ? tokotachi::Color::rgb(0xc98f0a) : tokotachi::Color::rgb(0x6b7280);
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
    return tokotachi::Image::from_pixels(16, 16, pixels);
}

int main(int argc, char** argv)
{
    tokotachi::Application app(argc, argv);
    tokotachi::Window window("hello - Tokotachi", 560, 620);
    // 下端のステータスバー(docs/adr/0030)。操作の結果は show_message で一時的に出す。
    tokotachi::StatusBar* status_bar = new tokotachi::StatusBar();
    status_bar->text = "準備完了";

    // 配置はレイアウトに任せる(docs/adr/0017)。ウィンドウは縦に並べ、横に並べる行は入れ物のウィジェットを介して入れ子にする。
    // 行の中のウィジェットは行を親にして作る(別の親で作っても、add が行の子に付け替える。docs/adr/0018)。
    // フォームはスクロール領域に入れる(ウィンドウを小さくすると縦にスクロールする。docs/adr/0021)。
    tokotachi::BoxLayout* window_box = new tokotachi::BoxLayout(&window);
    window_box->margins = tokotachi::Margins(0);
    tokotachi::ScrollArea* scroll_area_form = new tokotachi::ScrollArea(&window);
    window_box->add(scroll_area_form, 1);
    tokotachi::Widget* form = new tokotachi::Widget();
    scroll_area_form->content = form;
    tokotachi::BoxLayout* column = new tokotachi::BoxLayout(form);
    column->margins = tokotachi::Margins(24);
    column->spacing = 12;

    tokotachi::Label* label_title = new tokotachi::Label(&window, "Tokotachi のサンプル");
    label_title->font = tokotachi::Font(22);
    tokotachi::Label* label_description = new tokotachi::Label(
        &window, "このラベルは word_wrap で、ウィンドウの幅に合わせて折り返す。日本語の禁則(行の始めに「、」や「。」を置かない)にも従う。"
                 " English text wraps at spaces.");
    label_description->word_wrap = true;
    label_description->text_color = tokotachi::ColorRole::TEXT_SECONDARY;  // テーマの「控えめな文字」(ダークでも読める)

    tokotachi::Widget* click_row = new tokotachi::Widget(&window);
    tokotachi::BoxLayout* click_box = new tokotachi::BoxLayout(click_row, tokotachi::Orientation::HORIZONTAL);
    click_box->spacing = 16;
    tokotachi::Button* button_click = new tokotachi::Button(click_row, "クリック");
    button_click->minimum_size = tokotachi::Size(140, 36);
    tokotachi::Label* label_count = new tokotachi::Label(click_row, "クリック回数: 0");
    click_box->add(button_click, 0, tokotachi::Align::CENTER);
    click_box->add(label_count, 1);

    tokotachi::Label* label_caption = new tokotachi::Label(&window, "名前(IME で日本語も入力できる。Tab でフォーカスを移す)");
    label_caption->font = tokotachi::Font(12);
    label_caption->text_color = tokotachi::ColorRole::TEXT_SECONDARY;
    tokotachi::LineEdit* line_edit_name = new tokotachi::LineEdit(&window);
    line_edit_name->placeholder_text = "ここに入力";

    // 敬称はラジオボタンのグループ。各ボタンの group_value を同じ変数に結ぶ。
    tokotachi::Widget* option_row = new tokotachi::Widget(&window);
    tokotachi::BoxLayout* option_box = new tokotachi::BoxLayout(option_row, tokotachi::Orientation::HORIZONTAL);
    option_box->spacing = 16;
    tokotachi::RadioButton* radio_button_san = new tokotachi::RadioButton(option_row, "さん", 1);
    tokotachi::RadioButton* radio_button_sama = new tokotachi::RadioButton(option_row, "様", 2);
    tokotachi::RadioButton* radio_button_kun = new tokotachi::RadioButton(option_row, "くん", 3);
    tokotachi::CheckBox* check_box_big = new tokotachi::CheckBox(option_row, "大きい文字");
    option_box->add(radio_button_san);
    option_box->add(radio_button_sama);
    option_box->add(radio_button_kun);
    option_box->add_spacing(16);
    option_box->add(check_box_big);
    option_box->add_stretch();

    tokotachi::Label* label_greeting = new tokotachi::Label(&window);

    tokotachi::Button* button_disabled = new tokotachi::Button(&window, "無効なボタン");
    button_disabled->minimum_size = tokotachi::Size(140, 36);
    button_disabled->enabled = false;

    column->add(label_title);
    column->add(label_description);
    column->add(click_row);
    column->add(label_caption);
    column->add(line_edit_name);
    column->add(option_row);
    tokotachi::Widget* color_row = new tokotachi::Widget(&window);
    tokotachi::BoxLayout* color_box = new tokotachi::BoxLayout(color_row, tokotachi::Orientation::HORIZONTAL);
    color_box->add(new tokotachi::Label(color_row, "挨拶の色"), 0, tokotachi::Align::CENTER);
    tokotachi::ComboBox* combo_box_color = new tokotachi::ComboBox(color_row);
    std::vector<std::string> colors;
    colors.push_back("黒");
    colors.push_back("青");
    colors.push_back("緑");
    colors.push_back("赤");
    combo_box_color->items = colors;
    combo_box_color->current_index = 0;
    color_box->add(combo_box_color);
    color_box->add_spacing(24);
    // テーマ(docs/adr/0023)。既定は OS の設定に従う。
    color_box->add(new tokotachi::Label(color_row, "テーマ"), 0, tokotachi::Align::CENTER);
    tokotachi::ComboBox* combo_box_scheme = new tokotachi::ComboBox(color_row);
    std::vector<std::string> schemes;
    schemes.push_back("OS に従う");
    schemes.push_back("ライト");
    schemes.push_back("ダーク");
    combo_box_scheme->items = schemes;
    combo_box_scheme->current_index = 0;
    combo_box_scheme->on_change = [=](tokotachi::Object*)
    {
        static const tokotachi::ColorScheme SCHEMES[] = {tokotachi::ColorScheme::SYSTEM, tokotachi::ColorScheme::LIGHT,
                                                   tokotachi::ColorScheme::DARK};
        const int i = combo_box_scheme->current_index;
        tokotachi::Application::instance()->color_scheme = SCHEMES[i >= 0 ? i : 0];
    };
    color_box->add(combo_box_scheme);
    color_box->add_stretch();
    column->add(color_row);
    column->add(label_greeting);

    // タブ(docs/adr/0025)。ページごとにレイアウトを持たせ、TabWidget に加える。Ctrl+Tab で切り替えられる。
    tokotachi::TabWidget* tab_widget_main = new tokotachi::TabWidget(&window);
    tokotachi::Widget* memo_page = new tokotachi::Widget();
    tokotachi::BoxLayout* memo_box = new tokotachi::BoxLayout(memo_page);
    memo_box->margins = tokotachi::Margins(0, 12, 0, 0);
    memo_box->spacing = 8;
    memo_box->add(new tokotachi::Label(memo_page, "メモ(複数行。Enter で改行。右クリックでメニュー)"));
    tokotachi::TextEdit* text_edit_memo =
        new tokotachi::TextEdit(memo_page, "TextEdit は複数行の入力欄。折り返し・縦のスクロール・IME に対応する。\n2 行目");
    memo_box->add(text_edit_memo, 1);
    tab_widget_main->add_tab(memo_page, "メモ");

    // 値の部品(docs/adr/0024)。Slider・SpinBox・ProgressBar を同じ変数に結び、どれを動かしても揃う。
    tokotachi::Widget* level_page = new tokotachi::Widget();
    tokotachi::BoxLayout* level_box = new tokotachi::BoxLayout(level_page, tokotachi::Orientation::HORIZONTAL);
    level_box->margins = tokotachi::Margins(0, 12, 0, 0);
    level_box->spacing = 12;
    tokotachi::Slider* slider_level = new tokotachi::Slider(level_page);
    tokotachi::SpinBox* spin_box_level = new tokotachi::SpinBox(level_page);
    tokotachi::ProgressBar* progress_bar_level = new tokotachi::ProgressBar(level_page);
    progress_bar_level->text_visible = true;
    level_box->add(new tokotachi::Label(level_page, "進み具合"), 0, tokotachi::Align::START);
    level_box->add(slider_level, 1, tokotachi::Align::START);
    level_box->add(spin_box_level, 0, tokotachi::Align::START);
    level_box->add(progress_bar_level, 1, tokotachi::Align::START);
    tokotachi::DoubleVariable* level = new tokotachi::DoubleVariable(&window, 40);
    tokotachi::bind(slider_level->value, level->value);
    tokotachi::bind(spin_box_level->value, level->value);
    level->on_change = [=](tokotachi::Object*) { progress_bar_level->value = level->value.get(); };
    progress_bar_level->value = level->value.get();
    slider_level->tooltip = "ドラッグ・矢印キーで動かす";
    // 別のスレッドで数える(Task。docs/adr/0029)。仕事の関数は別のスレッドで動くので、ウィジェット・変数には触らず、
    // 進み具合を report_progress で知らせる。on_progress・on_finish はメインスレッドで呼ばれる。
    // (常駐するスレッドなら std::thread で作り、tokotachi::post と tokotachi::WeakRef で結果を返す。)
    tokotachi::Button* button_count = new tokotachi::Button(level_page, "裏で数える");
    button_count->tooltip = "Task で 0 から 100 まで数える。数えている間に押すと取り消す";
    level_box->add(button_count, 0, tokotachi::Align::START);
    tokotachi::Task<int>* counter = new tokotachi::Task<int>(&window);
    counter->on_progress = [=](tokotachi::Object*) { level->value = counter->progress.get(); };
    counter->on_finish = [=](tokotachi::Object*)
    {
        button_count->text = "裏で数える";
        status_bar->show_message(counter->cancelled ? "数えるのを取り消した"
                                                : "数え終えた: " + std::to_string(counter->result.get()),
                             4000);
    };
    button_count->on_click = [=](tokotachi::Object*)
    {
        if (counter->running)
        {
            counter->cancel();
            return;
        }
        button_count->text = "取り消す";
        counter->start([](tokotachi::TaskContext& ctx) -> int
        {
            int i = 0;
            for (; i <= 100 && !ctx.cancel_requested(); ++i)
            {
                std::this_thread::sleep_for(std::chrono::milliseconds(20));  // 重い処理の代わり
                ctx.report_progress(i);
            }
            return i - 1;
        });
    };
    tab_widget_main->add_tab(level_page, "進み具合");

    // 一覧と木(docs/adr/0026)。項目はモデル(ListModel・TreeModel)に持たせ、ビューに表示させる。
    const tokotachi::Image folder_icon = make_icon(true);
    const tokotachi::Image file_icon = make_icon(false);
    tokotachi::ListModel* files = new tokotachi::ListModel(&window);
    static const char* const FILES[][3] = {{"報告書.txt", "12 KB", "2026-10-01"},   {"写真.png", "2.4 MB", "2026-09-12"},
                                           {"見積もり.xlsx", "48 KB", "2026-09-30"}, {"README.md", "3 KB", "2026-08-05"},
                                           {"議事録.docx", "31 KB", "2026-10-03"},   {"build.log", "880 KB", "2026-10-04"},
                                           {"予定表.ics", "6 KB", "2026-07-21"},     {"archive.zip", "15 MB", "2026-06-30"}};
    for (std::size_t i = 0; i < sizeof(FILES) / sizeof(FILES[0]); ++i)
    {
        tokotachi::Item* item = files->add_item(FILES[i][0]);
        item->column_texts = std::vector<std::string>{FILES[i][1], FILES[i][2]};
        item->icon = file_icon;
        item->checkable = true;
    }
    tokotachi::Widget* list_page = new tokotachi::Widget();
    tokotachi::BoxLayout* list_box = new tokotachi::BoxLayout(list_page);
    list_box->margins = tokotachi::Margins(0, 12, 0, 0);
    list_box->spacing = 8;
    tokotachi::ListView* list_view_files = new tokotachi::ListView(list_page);
    list_view_files->add_column("名前", 200);
    list_view_files->add_column("大きさ", 90)->alignment = tokotachi::Align::END;
    list_view_files->add_column("更新日", 110);
    list_view_files->sorting_enabled = true;  // 見出しを押すと並べ替える
    list_view_files->selection_mode = tokotachi::SelectionMode::MULTI;
    list_view_files->model = files;
    tokotachi::Label* label_list_status = new tokotachi::Label(list_page, "Ctrl・Shift+クリックで複数選べる。ダブルクリック・Enter で開く");
    label_list_status->text_color = tokotachi::ColorRole::TEXT_SECONDARY;
    list_view_files->on_selection_change = [=](tokotachi::Object*)
    { label_list_status->text = std::to_string(list_view_files->selection.get().size()) + " 個を選んでいる"; };
    list_view_files->on_activate = [=](tokotachi::Object*)
    { label_list_status->text = "開く: " + files->text(list_view_files->current, 0); };
    list_box->add(list_view_files, 1);
    list_box->add(label_list_status);
    tab_widget_main->add_tab(list_page, "一覧");

    tokotachi::TreeModel* tree = new tokotachi::TreeModel(&window);
    tokotachi::TreeItem* documents = tree->add_item("文書");
    documents->icon = folder_icon;
    tokotachi::TreeItem* work = documents->add_child("仕事");
    work->icon = folder_icon;
    work->add_child("報告書.txt")->icon = file_icon;
    work->add_child("見積もり.xlsx")->icon = file_icon;
    documents->add_child("README.md")->icon = file_icon;
    tokotachi::TreeItem* pictures = tree->add_item("画像");
    pictures->icon = folder_icon;
    for (int i = 1; i <= 30; ++i)
    {
        pictures->add_child("写真 " + std::to_string(i) + ".png")->icon = file_icon;
    }
    tokotachi::TreeItem* empty = tree->add_item("空のフォルダー");
    empty->icon = folder_icon;
    tokotachi::TreeView* tree_view_folders = new tokotachi::TreeView();
    tree_view_folders->model = tree;
    tree_view_folders->expand(tree->index_of(documents));

    // 分割ウィンドウ(docs/adr/0031)。左に木、右に選んだ項目の中身。境目をドラッグして幅を変え、端まで寄せるとたためる。
    // 左は stretch 0(ウィンドウの幅が変わっても幅を保つ)、右は 1(増えた・減った幅を受け持つ)。
    tokotachi::ListModel* contents = new tokotachi::ListModel(&window);
    tokotachi::ListView* list_view_contents = new tokotachi::ListView();
    list_view_contents->add_column("名前", 160);
    list_view_contents->add_column("種類", 80);
    list_view_contents->model = contents;
    tree_view_folders->on_selection_change = [=](tokotachi::Object*)
    {
        contents->clear();
        const tokotachi::TreeItem* chosen = tree->item_at(tree_view_folders->current);
        if (!chosen)
        {
            return;
        }
        const std::vector<tokotachi::TreeItem*> children = chosen->children;
        for (std::size_t i = 0; i < children.size(); ++i)
        {
            const bool folder = !children[i]->children.get().empty();
            tokotachi::Item* item = contents->add_item(children[i]->text);
            item->icon = children[i]->icon;
            item->column_texts = std::vector<std::string>(1, folder ? "フォルダー" : "ファイル");
        }
    };
    tokotachi::Splitter* splitter_files = new tokotachi::Splitter();
    splitter_files->add_widget(tree_view_folders, 0);
    splitter_files->add_widget(list_view_contents, 1);
    splitter_files->sizes = std::vector<float>{180, 300};
    tab_widget_main->add_tab(splitter_files, "木");

    // 自由な描画(docs/adr/0032)。on_paint の中で Canvas に描く(Canvas は描画の呼び出しの間だけ有効)。
    // PaintBox(docs/adr/0033)は VCL の TPaintBox に当たる。cached にすると描いた結果を覚え、大きさ・テーマが変わるか
    // refresh を呼ぶまで on_paint を呼ばない(重い描画向け)。
    tokotachi::PaintBox* paint_box_drawing = new tokotachi::PaintBox();
    paint_box_drawing->cached = true;
    // Bitmap は書き換えられる画像。画素を直接書いて作り、Image にして貼る(ここでは色相の帯)。
    tokotachi::Bitmap hues(120, 24);
    for (int x = 0; x < hues.width(); ++x)
    {
        const float h = x * 6.0f / hues.width();
        const int i = static_cast<int>(h);
        const float f = h - i;
        const std::uint8_t up = static_cast<std::uint8_t>(255 * f), down = static_cast<std::uint8_t>(255 * (1 - f));
        const tokotachi::Color colors[] = {tokotachi::Color(255, up, 0), tokotachi::Color(down, 255, 0), tokotachi::Color(0, 255, up),
                                     tokotachi::Color(0, down, 255), tokotachi::Color(up, 0, 255), tokotachi::Color(255, 0, down)};
        for (int y = 0; y < hues.height(); ++y)
        {
            hues.set_pixel(x, y, colors[i % 6].with_alpha(static_cast<std::uint8_t>(255 - y * 8)));
        }
    }
    const tokotachi::Image hue_image = hues.to_image();
    // 重い描画は別のスレッドで Bitmap に描き、Image にして返す(Task。docs/adr/0034)。文字も別のスレッドで描ける。
    // 描き終えたら覚えている絵を捨てて(refresh)、貼り直す。
    std::shared_ptr<tokotachi::Image> fractal = std::make_shared<tokotachi::Image>();
    tokotachi::Task<tokotachi::Image>* fractal_task = new tokotachi::Task<tokotachi::Image>(&window);
    fractal_task->on_finish = [=](tokotachi::Object*)
    {
        *fractal = fractal_task->result;
        paint_box_drawing->refresh();
    };
    fractal_task->start([](tokotachi::TaskContext& ctx) -> tokotachi::Image
    {
        tokotachi::Bitmap b(240, 100);
        for (int y = 0; y < b.height() && !ctx.cancel_requested(); ++y)
        {
            for (int x = 0; x < b.width(); ++x)
            {
                const double cr = -2.2 + x * 3.0 / b.width(), ci = -1.1 + y * 2.2 / b.height();
                double zr = 0, zi = 0;
                int n = 0;
                for (; n < 200 && zr * zr + zi * zi < 4; ++n)
                {
                    const double t = zr * zr - zi * zi + cr;
                    zi = 2 * zr * zi + ci;
                    zr = t;
                }
                const std::uint8_t v = static_cast<std::uint8_t>(n >= 200 ? 0 : 255 - n * 255 / 200);
                b.set_pixel(x, y, tokotachi::Color(static_cast<std::uint8_t>(v / 3), static_cast<std::uint8_t>(v / 2), v));
            }
        }
        b.canvas().draw_text("別のスレッドで描いた", tokotachi::Point(6, 6), tokotachi::Color::rgb(0xffffff));
        return b.to_image();
    });
    paint_box_drawing->on_paint = [hue_image, fractal](tokotachi::Object* sender, tokotachi::Canvas& c)
    {
        const tokotachi::Rect r = static_cast<tokotachi::Widget*>(sender)->local_rect;
        const tokotachi::Color ink = tokotachi::Theme::light() == tokotachi::Application::instance()->theme.get()
                                   ? tokotachi::Color::rgb(0x111827)
                                   : tokotachi::Color::rgb(0xf3f4f6);
        // 円グラフ(円弧のパス)。
        static const float SHARES[] = {0.42f, 0.27f, 0.18f, 0.13f};
        static const std::uint32_t COLORS[] = {0x2563eb, 0x059669, 0xf59e0b, 0xdc2626};
        const tokotachi::Point center(55, 60);
        float angle = -1.5707963f;
        for (int i = 0; i < 4; ++i)
        {
            const float sweep = SHARES[i] * 6.2831853f;
            tokotachi::Path slice;
            slice.move_to(center.x, center.y);
            slice.add_arc(center, 45, 45, angle, sweep, false);
            slice.close();
            c.fill_path(slice, tokotachi::Color::rgb(COLORS[i]));
            angle += sweep;
        }
        // 破線と線の端・角の形。
        tokotachi::StrokeStyle dashed(2, tokotachi::LineCap::ROUND, tokotachi::LineJoin::ROUND);
        dashed.dashes = std::vector<float>{8, 5};
        std::vector<tokotachi::Point> zigzag;
        for (int i = 0; i < 6; ++i)
        {
            zigzag.push_back(tokotachi::Point(120 + i * 22.0f, i % 2 ? 40.0f : 15.0f));
        }
        tokotachi::Path line;
        line.add_polyline(zigzag);
        c.stroke_path(line, dashed, ink);
        c.draw_line(tokotachi::Point(120, 60), tokotachi::Point(230, 60), 6, tokotachi::Color::rgb(0x2563eb), tokotachi::LineCap::ROUND);
        c.stroke_ellipse(tokotachi::Rect(120, 75, 110, 40), tokotachi::StrokeStyle(1.5f), ink);

        // 回転と拡大(図形・グラデーション・文字のすべてに効く)。
        c.save();
        c.translate(305, 50);
        c.rotate(-0.35f);
        std::vector<tokotachi::GradientStop> stops;
        stops.push_back(tokotachi::GradientStop(0, tokotachi::Color::rgb(0x2563eb)));
        stops.push_back(tokotachi::GradientStop(1, tokotachi::Color::rgb(0x059669)));
        c.fill_round_rect(tokotachi::Rect(-60, -25, 120, 50), 10, tokotachi::Paint::linear(tokotachi::Point(-60, 0), tokotachi::Point(60, 0), stops));
        c.draw_text("回転した文字", tokotachi::Point(-48, -10), tokotachi::Color::rgb(0xffffff));
        c.restore();
        c.save();
        c.translate(250, 100);
        c.scale_by(1.8f, 1.8f);
        c.draw_text("拡大 Ag", tokotachi::Point(0, 0), ink);
        c.restore();

        // パスでクリップ(円の中だけに縞を描く)。
        c.save();
        tokotachi::Path circle;
        circle.add_ellipse(tokotachi::Point(425, 60), 42, 42);
        c.clip_path(circle);
        for (int i = 0; i < 12; ++i)
        {
            c.fill_rect(tokotachi::Rect(380 + i * 8.0f, 15, 4, 90), tokotachi::Color::rgb(COLORS[i % 4]));
        }
        c.draw_text("クリップ", tokotachi::Point(398, 52), ink);
        c.restore();

        // 透過: 図形ごとの半透明(重なりが濃い)と、グループの透過(begin_layer。重なりも同じ濃さ)。
        for (int k = 0; k < 2; ++k)
        {
            const float x0 = 20 + k * 150.0f;
            if (k == 1)
            {
                c.begin_layer(0.5f);
            }
            const std::uint8_t alpha = k == 0 ? 128 : 255;
            c.fill_ellipse(tokotachi::Rect(x0, 130, 60, 60), tokotachi::Color::rgb(0x2563eb, alpha));
            c.fill_ellipse(tokotachi::Rect(x0 + 35, 130, 60, 60), tokotachi::Color::rgb(0xdc2626, alpha));
            c.fill_ellipse(tokotachi::Rect(x0 + 17, 160, 60, 60), tokotachi::Color::rgb(0x059669, alpha));
            if (k == 1)
            {
                c.end_layer();
            }
            c.draw_text(k == 0 ? "図形ごとの半透明" : "グループの透過", tokotachi::Point(x0, 225), ink);
        }
        c.draw_image(hue_image, tokotachi::Rect(330, 150, 120, 24));
        c.draw_text("Bitmap の画素", tokotachi::Point(330, 180), ink);
        if (!fractal->is_null())
        {
            c.draw_image(*fractal, tokotachi::Rect(20, 255, 240, 100));
        }
        else
        {
            c.draw_text("裏で描いています...", tokotachi::Point(20, 255), ink);
        }
        (void)r;
    };
    tab_widget_main->add_tab(paint_box_drawing, "描画");

    tab_widget_main->add_tab(new tokotachi::Widget(), "無効なタブ")->enabled = false;
    tab_widget_main->minimum_size = tokotachi::Size(0, 400);
    column->add(tab_widget_main, 1);
    column->add(button_disabled, 0, tokotachi::Align::START);  // 横に広げない

    // 画面の状態は変数に持ち、ウィジェットと結ぶ(docs/adr/0016)。
    tokotachi::StringVariable* name = new tokotachi::StringVariable(&window);
    tokotachi::IntVariable* honorific = new tokotachi::IntVariable(&window, 1);
    tokotachi::BoolVariable* large = new tokotachi::BoolVariable(&window, false);
    tokotachi::bind(line_edit_name->text, name->value);
    tokotachi::bind(radio_button_san->group_value, honorific->value);
    tokotachi::bind(radio_button_sama->group_value, honorific->value);
    tokotachi::bind(radio_button_kun->group_value, honorific->value);
    tokotachi::bind(check_box_big->checked, large->value);

    // どの変数が変わっても挨拶を作り直す(1 つのハンドラを 3 つの変数で共有する)。
    const tokotachi::NotifyHandler update_greeting = [=](tokotachi::Object*)
    {
        static const char* const SUFFIX[] = {"", "さん", "様", "くん"};
        const std::string n = name->value;
        label_greeting->text = n.empty() ? std::string() : "こんにちは、" + n + " " + SUFFIX[honorific->value.get()];
        label_greeting->font = tokotachi::Font(large->value ? 20.0f : 14.0f);
        // 「黒」はテーマの文字の色にする(ダークでは明るい色になる)。他は固定の色。
        const int c = combo_box_color->current_index;
        static const std::uint32_t RGB[] = {0, 0x2563eb, 0x059669, 0xdc2626};
        label_greeting->text_color = c <= 0 ? tokotachi::ThemeColor(tokotachi::ColorRole::TEXT) : tokotachi::ThemeColor(tokotachi::Color::rgb(RGB[c]));
    };
    name->on_change = update_greeting;
    honorific->on_change = update_greeting;
    large->on_change = update_greeting;
    combo_box_color->on_change = update_greeting;

    int clicks = 0;
    button_click->on_click = [&](tokotachi::Object*)
    {
        ++clicks;
        label_count->text = "クリック回数: " + std::to_string(clicks);
    };
    line_edit_name->on_submit = [=](tokotachi::Object*) { line_edit_name->select_all(); };
    button_click->tooltip = "押した回数を数える";
    button_disabled->tooltip = "無効なウィジェットにもツールチップは出る";

    // メモの右クリックのメニュー(docs/adr/0022)。メニューはウィンドウが所有する。
    // 右クリックのメニューのショートカットは表示だけ(キーは TextEdit 自身が扱う)。
    tokotachi::Menu* menu_memo = new tokotachi::Menu(&window);
    menu_memo->add_item("切り取り(&T)", [=](tokotachi::Object*) { text_edit_memo->cut(); })->shortcut = "Ctrl+X";
    menu_memo->add_item("コピー(&C)", [=](tokotachi::Object*) { text_edit_memo->copy(); })->shortcut = "Ctrl+C";
    menu_memo->add_item("貼り付け(&P)", [=](tokotachi::Object*) { text_edit_memo->paste(); })->shortcut = "Ctrl+V";
    menu_memo->add_separator();
    menu_memo->add_item("すべて選択(&A)", [=](tokotachi::Object*) { text_edit_memo->select_all(); })->shortcut = "Ctrl+A";
    tokotachi::Menu* menu_insert = menu_memo->add_submenu("挿入(&I)");
    menu_insert->add_item("挨拶", [=](tokotachi::Object*) { text_edit_memo->insert(label_greeting->text.get()); });
    menu_insert->add_item("クリック回数", [=](tokotachi::Object*) { text_edit_memo->insert(label_count->text.get()); });
    text_edit_memo->on_mouse_down = [=](tokotachi::Object*, tokotachi::MouseEvent& e)
    {
        if (e.button == tokotachi::MouseButton::RIGHT)
        {
            menu_memo->popup(text_edit_memo, e.pos);
        }
    };

    // メニューバーとショートカット(docs/adr/0027)。メニューバーのメニューの Action のキーは、ウィンドウ全体で効く。
    tokotachi::MenuBar* menu_bar = new tokotachi::MenuBar();
    tokotachi::Menu* menu_file = menu_bar->add_menu("ファイル(&F)");
    tokotachi::Action* action_new = menu_file->add_item("新規(&N)", [=](tokotachi::Object*) { text_edit_memo->text = ""; });
    action_new->shortcut = "Ctrl+N";

    // OS のファイル選択(docs/adr/0028)。結果は on_finish で受け取る。メモの内容を読み書きする。
    std::vector<tokotachi::FileFilter> text_filters;
    text_filters.push_back(tokotachi::FileFilter("テキスト", "txt;md"));
    text_filters.push_back(tokotachi::FileFilter("すべてのファイル", "*"));
    tokotachi::FileDialog* file_dialog_open = new tokotachi::FileDialog(&window, tokotachi::FileDialogMode::OPEN);
    file_dialog_open->filters = text_filters;
    // 失敗したとき(Linux で xdg-desktop-portal も zenity も無いときなど)は理由を見せる。
    const auto report_failure = [&window](tokotachi::FileDialog* d)
    {
        tokotachi::MessageDialog::show(&window, "ファイルのダイアログ",
                                 "ファイルのダイアログを開けなかった: " + d->error.get(),
                                 std::vector<tokotachi::DialogButton>(), tokotachi::NotifyHandler(), tokotachi::MessageType::WARNING);
    };
    file_dialog_open->on_finish = [=](tokotachi::Object*)
    {
        const std::vector<std::string> paths = file_dialog_open->paths;
        if (!file_dialog_open->error.get().empty())
        {
            report_failure(file_dialog_open);
            return;
        }
        if (paths.empty())
        {
            return;  // 取り消した
        }
        std::ifstream in(paths[0].c_str(), std::ios::binary);
        std::ostringstream content;
        content << in.rdbuf();
        text_edit_memo->text = content.str();
        tab_widget_main->current_index = 0;
        status_bar->show_message("開いた: " + paths[0], 4000);
    };
    tokotachi::Action* action_open = menu_file->add_item("開く(&O)...", [=](tokotachi::Object*) { file_dialog_open->open(); });
    action_open->shortcut = "Ctrl+O";
    tokotachi::FileDialog* file_dialog_save = new tokotachi::FileDialog(&window, tokotachi::FileDialogMode::SAVE);
    file_dialog_save->filters = text_filters;
    file_dialog_save->location = "メモ.txt";
    file_dialog_save->on_finish = [=](tokotachi::Object*)
    {
        const std::vector<std::string> paths = file_dialog_save->paths;
        if (!file_dialog_save->error.get().empty())
        {
            report_failure(file_dialog_save);
            return;
        }
        if (paths.empty())
        {
            return;
        }
        std::ofstream out(paths[0].c_str(), std::ios::binary);
        out << text_edit_memo->text.get();
        status_bar->show_message(out ? "保存した: " + paths[0] : "保存できなかった: " + paths[0], 4000);
    };
    tokotachi::Action* action_save = menu_file->add_item("名前を付けて保存(&S)...", [=](tokotachi::Object*) { file_dialog_save->open(); });
    action_save->shortcut = "Ctrl+S";
    menu_file->add_separator();

    // 自分で中身を作るダイアログ(docs/adr/0028)。ボタンは OS の慣習の順に並び、Enter で OK、Esc でキャンセル。
    menu_file->add_item("名前を入力(&R)...", [&window, name](tokotachi::Object*)
    {
        tokotachi::Dialog* dialog_name = new tokotachi::Dialog(&window, "名前の入力");
        tokotachi::BoxLayout* box = new tokotachi::BoxLayout(dialog_name);
        box->margins = tokotachi::Margins(20, 20, 20, 12);
        box->spacing = 8;
        box->add(new tokotachi::Label(dialog_name, "挨拶に使う名前"));
        tokotachi::LineEdit* line_edit_field = new tokotachi::LineEdit(dialog_name, name->value);
        box->add(line_edit_field);
        dialog_name->add_button(tokotachi::DialogButton::OK);
        dialog_name->add_button(tokotachi::DialogButton::CANCEL);
        dialog_name->on_finish = [=](tokotachi::Object*)
        {
            if (dialog_name->result == tokotachi::DialogButton::OK)
            {
                name->value = line_edit_field->text.get();
            }
            dialog_name->delete_later();  // 自分のハンドラの中なので、後で破棄する
        };
        line_edit_field->select_all();
        line_edit_field->focused = true;
        dialog_name->open();
    })->shortcut = "Ctrl+R";
    menu_file->add_separator();

    // 閉じる前に確かめる(メッセージのダイアログ)。ウィンドウの閉じるボタンと Ctrl+Q の両方で。
    const auto confirm_quit = [&window]()
    {
        tokotachi::MessageDialog::show(&window, "終了", "サンプルを終了しますか？",
                                 std::vector<tokotachi::DialogButton>{tokotachi::DialogButton::YES, tokotachi::DialogButton::NO},
                                 [&window](tokotachi::Object* sender)
                                 {
                                     if (static_cast<tokotachi::Dialog*>(sender)->result == tokotachi::DialogButton::YES)
                                     {
                                         window.close();
                                     }
                                 },
                                 tokotachi::MessageType::QUESTION);
    };
    window.on_close = [=](tokotachi::Object*, bool& can_close)
    {
        can_close = false;
        confirm_quit();
    };
    menu_file->add_item("終了(&X)", [=](tokotachi::Object*) { confirm_quit(); })->shortcut = "Ctrl+Q";
    tokotachi::Menu* menu_view = menu_bar->add_menu("表示(&V)");
    tokotachi::Action* action_large = menu_view->add_item("大きい文字(&L)");
    action_large->checkable = true;
    action_large->shortcut = "Ctrl+L";
    tokotachi::bind(action_large->checked, large->value);  // チェックボックスと同じ変数に結ぶ
    tokotachi::Menu* menu_theme = menu_view->add_submenu("テーマ(&T)");
    menu_theme->add_item("OS に従う(&S)", [=](tokotachi::Object*) { combo_box_scheme->current_index = 0; });
    menu_theme->add_item("ライト(&L)", [=](tokotachi::Object*) { combo_box_scheme->current_index = 1; });
    menu_theme->add_item("ダーク(&D)", [=](tokotachi::Object*) { combo_box_scheme->current_index = 2; });
    menu_view->add_separator();
    menu_view->add_item("次のタブ", [=](tokotachi::Object*)
                        { tab_widget_main->current_index = (tab_widget_main->current_index + 1) % 5; })->shortcut = "F6";
    tokotachi::Menu* menu_help = menu_bar->add_menu("ヘルプ(&H)");
    menu_help->add_item("バージョン情報(&A)", [&window](tokotachi::Object*)
    {
        tokotachi::MessageDialog::show(&window, "バージョン情報",
                                 "Tokotachi のサンプル。SDL3 と plutovg の上に、C++11 で書いている。");
    });
    menu_help->add_item("警告の例(&W)", [&window](tokotachi::Object*)
    {
        tokotachi::MessageDialog::show(&window, "警告", "これは警告のダイアログの例。長い文はダイアログの幅で折り返す。"
                                 "ボタンが 1 つなら Enter でも Esc でも閉じられる。",
                                 std::vector<tokotachi::DialogButton>{tokotachi::DialogButton::CLOSE}, tokotachi::NotifyHandler(),
                                 tokotachi::MessageType::WARNING);
    });
    window.menu_bar = menu_bar;

    // ツールバー(docs/adr/0030)。メニューと同じ Action を並べる(無効にすれば両方が無効になる)。
    // 画像のある Action は画像だけを出し、文字はツールチップに出す。画像が無ければ文字を出す。
    tokotachi::ToolBar* tool_bar = new tokotachi::ToolBar();
    action_new->icon = file_icon;
    action_open->icon = folder_icon;
    tool_bar->add_action(action_new);
    tool_bar->add_action(action_open);
    tool_bar->add_action(action_save);
    tool_bar->add_separator();
    tool_bar->add_action(action_large);  // チェックできる Action は押し込んだ見た目になる
    tool_bar->add_item("挨拶を青に", [=](tokotachi::Object*) { combo_box_color->current_index = 1; })->tooltip = "挨拶の色を青にする";
    tool_bar->add_separator();
    tokotachi::ComboBox* combo_box_style = new tokotachi::ComboBox();
    std::vector<std::string> styles;
    styles.push_back("画像だけ");
    styles.push_back("画像の右に文字");
    styles.push_back("画像の下に文字");
    combo_box_style->items = styles;
    combo_box_style->current_index = 0;
    combo_box_style->tooltip = "ツールバーのボタンの見た目";
    combo_box_style->on_change = [=](tokotachi::Object*)
    {
        static const tokotachi::ToolButtonStyle STYLES[] = {tokotachi::ToolButtonStyle::ICON_ONLY,
                                                      tokotachi::ToolButtonStyle::TEXT_BESIDE_ICON,
                                                      tokotachi::ToolButtonStyle::TEXT_UNDER_ICON};
        tool_bar->button_style = STYLES[combo_box_style->current_index >= 0 ? combo_box_style->current_index.get() : 0];
    };
    tool_bar->add_widget(combo_box_style);  // 任意の部品も置ける(幅が足りなければ隠れる)
    window.tool_bar = tool_bar;

    // ステータスバーの右には部品を並べられる。裏で数える進み具合を出す。
    tokotachi::ProgressBar* progress_bar_status = new tokotachi::ProgressBar();
    progress_bar_status->minimum_size = tokotachi::Size(120, 0);
    status_bar->add_widget(progress_bar_status);
    level->on_change = [=](tokotachi::Object*)
    {
        progress_bar_level->value = level->value.get();
        progress_bar_status->value = level->value.get();
    };
    progress_bar_status->value = level->value.get();
    window.status_bar = status_bar;

    // ウィジェットに加えた Action のキーは、フォーカスがその中にあるときだけ効く。一覧の Delete で選んだ項目を除く。
    tokotachi::Action* action_remove_files = new tokotachi::Action(&window, "削除(&D)");
    action_remove_files->shortcut = "Delete";
    action_remove_files->on_trigger = [=](tokotachi::Object*)
    {
        const std::vector<tokotachi::ModelIndex> chosen = list_view_files->selection;
        for (std::size_t i = chosen.size(); i-- > 0;)
        {
            files->remove_item(files->item_at(chosen[i]));
        }
        label_list_status->text = std::to_string(chosen.size()) + " 個を削除した";
    };
    list_view_files->add_action(action_remove_files);
    tokotachi::Menu* menu_list = new tokotachi::Menu(&window);
    menu_list->add_action(action_remove_files);
    list_view_files->on_mouse_down = [=](tokotachi::Object*, tokotachi::MouseEvent& e)
    {
        if (e.button == tokotachi::MouseButton::RIGHT)
        {
            menu_list->popup(list_view_files, e.pos);
        }
    };

    line_edit_name->focused = true;
    window.visible = true;

    for (int i = 1; i < argc; ++i)
    {
        if (std::string(argv[i]) == "--dark")
        {
            app.color_scheme = tokotachi::ColorScheme::DARK;
            combo_box_scheme->current_index = 2;
        }
        else if (std::string(argv[i]) == "--tab" && i + 1 < argc)
        {
            tab_widget_main->current_index = std::atoi(argv[++i]);
        }
        else if (std::string(argv[i]) == "--screenshot" && i + 1 < argc)
        {
            name->value = "Ünïcödé と日本語 AVA ffi";  // 変数への代入が入力欄と挨拶に伝わる
            honorific->value = 2;
            large->value = true;
            line_edit_name->select_all();
            combo_box_color->current_index = 1;
            return window.save_png(argv[i + 1]) ? 0 : 1;
        }
    }
    return app.exec();
}
