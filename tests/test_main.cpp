// gui ライブラリのテスト。SDL の offscreen のビデオドライバで、画面なしで動かす(CMake が環境変数を設定する)。
#include <gui/gui.hpp>

#include "../src/core/binding_registry.hpp"
#include "../src/core/application_impl.hpp"
#include "../src/core/window_impl.hpp"

#include <cstdio>
#include <functional>
#include <string>
#include <vector>

namespace
{

int g_failures = 0;
int g_checks = 0;

#define CHECK(cond)                                                                                                    \
    do                                                                                                                 \
    {                                                                                                                  \
        ++g_checks;                                                                                                    \
        if (!(cond))                                                                                                   \
        {                                                                                                              \
            ++g_failures;                                                                                              \
            std::printf("  FAILED %s:%d: %s\n", __FILE__, __LINE__, #cond);                                            \
        }                                                                                                              \
    } while (0)

struct TestCase
{
    const char* name;
    std::function<void()> fn;
};

std::vector<TestCase>& registry()
{
    static std::vector<TestCase> r;
    return r;
}

struct Register
{
    Register(const char* name, std::function<void()> fn)
    {
        const TestCase t = {name, fn};
        registry().push_back(t);
    }
};

#define TEST(name)                                                                                                     \
    void name();                                                                                                       \
    Register register_##name(#name, &name);                                                                            \
    void name()

bool near(float a, float b) { return a - b < 0.01f && b - a < 0.01f; }

// 保護された仮想関数を呼ぶための派生クラス。
struct TestEdit : gui::LineEdit
{
    explicit TestEdit(gui::Widget* parent_widget, const std::string& s = std::string()) : gui::LineEdit(parent_widget, s)
    {
    }

    void key(gui::Key k, gui::Modifiers m = gui::MODIFIER_NONE)
    {
        gui::KeyEvent e(k, m, false, 0);
        key_down(e);
    }

    void type(const std::string& s)
    {
        gui::TextInputEvent e(s);
        text_input(e);
    }

    void compose(const std::string& s, int start = -1, int length = -1)
    {
        gui::TextEditingEvent e(s, start, length);
        text_editing(e);
    }
};

struct TestButton : gui::Button
{
    explicit TestButton(gui::Widget* parent_widget) : gui::Button(parent_widget, "b") {}

    bool key(gui::Key k)
    {
        gui::KeyEvent e(k, gui::MODIFIER_NONE, false, 0);
        key_down(e);
        return e.accepted;
    }

    bool press(gui::MouseButton b)
    {
        gui::MouseEvent e(gui::Point(1, 1), b, gui::MODIFIER_NONE, 1);
        mouse_down(e);
        return e.accepted;
    }
};

struct Counted : gui::Widget
{
    Counted(gui::Widget* parent_widget, int* counter) : gui::Widget(parent_widget), counter_(counter) {}
    ~Counted() { ++*counter_; }

private:
    int* counter_;
};

TEST(text_layout_caret_stops)
{
    // e + 結合アキュート(U+0301)は 1 つのクラスタ。
    const std::string s = "ae\xCC\x81\xE6\x97\xA5";  // a, e+U+0301, 日
    gui::TextLayout l(s, gui::Font(14), 1.0f);
    CHECK(l.width() > 0);
    CHECK(l.height() > 0);
    CHECK(l.next_offset(0) == 1);
    CHECK(l.next_offset(1) == 4);  // 結合文字の途中には止まらない
    CHECK(l.next_offset(4) == 7);
    CHECK(l.previous_offset(7) == 4);
    CHECK(l.previous_offset(4) == 1);
    CHECK(l.caret_position(0).x == 0);
    CHECK(l.caret_position(1).x < l.caret_position(4).x);
    CHECK(l.caret_position(4).x < l.caret_position(7).x);
    CHECK(l.offset_at(gui::Point(-10, 0)) == 0);
    CHECK(l.offset_at(gui::Point(1000, 0)) == s.size());
    CHECK(l.line_count() == 1);
}

TEST(text_layout_hard_line_breaks)
{
    const std::string s = "ab\ncd\r\nef";  // \n と \r\n
    gui::TextLayout l(s, gui::Font(14), 1.0f);
    CHECK(l.line_count() == 3);
    CHECK(l.line_start(1) == 3 && l.line_end(1) == 5);
    CHECK(l.line_start(2) == 7 && l.line_end(2) == 9);
    CHECK(near(l.height(), 2 * l.line_height() + l.ascent() + l.descent()));
    CHECK(l.line_of(2) == 0);  // 改行文字の直前はその行
    CHECK(l.line_of(3) == 1);
    CHECK(l.caret_position(3).x == 0);
    CHECK(near(l.caret_position(3).y, l.line_height()));
    CHECK(l.next_offset(2) == 3);      // 行の終わりから次の行の始めへ(改行文字を飛ばす)
    CHECK(l.previous_offset(7) == 5);  // \r\n も飛ばす
    CHECK(l.offset_at(gui::Point(0, l.line_height() * 2 + 1)) == 7);

    gui::TextLayout trailing("a\n", gui::Font(14), 1.0f);  // 改行文字で終わると、最後に空の行がある
    CHECK(trailing.line_count() == 2);
    CHECK(trailing.line_start(1) == 2);
}

TEST(text_layout_wraps_at_break_opportunities)
{
    const std::string s = "aaa bbb ccc";
    const float w = gui::TextLayout("aaa bbb", gui::Font(14), 1.0f).width();
    gui::TextLayout l(s, gui::Font(14), 1.0f, w + 1);
    CHECK(l.line_count() == 2);
    CHECK(l.line_start(1) == 8);  // 空白の後ろで分ける(行末の空白は幅に含めない)
    CHECK(l.line_of(8) == 1);     // 折り返した境目は次の行
    CHECK(l.width() <= w + 1);

    // 1 つの単語が幅に収まらなければ、文字の途中で分ける。
    gui::TextLayout narrow("abcdefgh", gui::Font(14), 1.0f, gui::TextLayout("abc", gui::Font(14), 1.0f).width() + 1);
    CHECK(narrow.line_count() >= 2);
    CHECK(narrow.line_start(1) == 3);
}

TEST(text_layout_japanese_line_breaking)
{
    // 「、」は行の始めに置かない(禁則)。「あいうえお」の幅で折り返すと、「お」ごと次の行へ送る。
    const std::string s = "あいうえお、かきくけこ";
    const float w = gui::TextLayout("あいうえお", gui::Font(14), 1.0f).width();
    gui::TextLayout l(s, gui::Font(14), 1.0f, w + 0.5f);
    CHECK(l.line_count() >= 2);
    const std::string second = s.substr(l.line_start(1), 3);
    CHECK(second != "、");
    CHECK(second == "お");
}

TEST(text_layout_alignment)
{
    gui::TextLayout l("a\nwide line", gui::Font(14), 1.0f, 0, gui::Align::END);
    CHECK(near(l.line_rect(0).right(), l.width()));  // 行の右端をそろえる
    CHECK(near(l.line_rect(1).x, 0));
}

TEST(text_layout_words)
{
    const std::string s = "hello world  foo";
    gui::TextLayout l(s, gui::Font(14), 1.0f);
    CHECK(l.next_word_offset(0) == 6);
    CHECK(l.next_word_offset(6) == 13);  // 空白が 2 つでも飛ばす
    CHECK(l.next_word_offset(13) == s.size());
    CHECK(l.previous_word_offset(13) == 6);
    CHECK(l.previous_word_offset(8) == 6);
    CHECK(l.previous_word_offset(6) == 0);
    std::size_t a = 0, b = 0;
    l.word_at(8, a, b);
    CHECK(a == 6 && b == 11);

    // カタカナの連続は 1 つの単語(UAX #29)
    const std::string j = "日本のテキスト";
    gui::TextLayout lj(j, gui::Font(14), 1.0f);
    lj.word_at(10, a, b);  // 「テ」の位置
    CHECK(j.substr(a, b - a) == "テキスト");
}

TEST(text_layout_rtl_paragraph)
{
    // ヘブライ語だけの段落は右から左。論理の先頭(位置 0)のキャレットは右端、末尾は左端。
    const std::string s = "שלום";  // 4 文字 x 2 バイト
    gui::TextLayout l(s, gui::Font(14), 1.0f);
    CHECK(l.line_is_rtl(0));
    CHECK(l.caret_position(0).x > l.caret_position(s.size()).x);
    CHECK(near(l.caret_position(0).x, l.width()));
    CHECK(near(l.caret_position(s.size()).x, 0));
    CHECK(l.caret_position(2).x < l.caret_position(0).x);  // 論理の順に進むと左へ動く
    CHECK(l.offset_at(gui::Point(-5, 0)) == s.size());     // 左端は論理の末尾
    CHECK(l.offset_at(gui::Point(l.width() + 5, 0)) == 0);
    CHECK(l.next_offset(0) == 2);                          // 矢印キーでの移動は論理の順
}

TEST(text_layout_mixed_direction)
{
    // 左から右の段落に右から左の単語が入る: 見た目は "abc " + ヘブライ語(右から左) + " def"。
    const std::string s = "abc שלום def";  // ヘブライ語は 4..12
    gui::TextLayout l(s, gui::Font(14), 1.0f);
    CHECK(!l.line_is_rtl(0));
    const float run_left = l.caret_position(4).x;  // ここは "c " の直後ではなく、ヘブライ語の最初の文字の右端
    CHECK(l.caret_position(4).x > l.caret_position(6).x);   // ヘブライ語の中では、論理の順に進むと左へ
    CHECK(l.caret_position(13).x > run_left);                // " def" はヘブライ語の右
    // "c " とヘブライ語の最初の 1 文字を選ぶと、見た目では離れた 2 つの矩形になる。
    const std::vector<gui::Rect> rects = l.selection_rects(2, 6);
    CHECK(rects.size() == 2);
    // 左から右の部分だけなら 1 つ
    CHECK(l.selection_rects(0, 4).size() == 1);
}

TEST(text_layout_rtl_alignment)
{
    // 右から左の段落では、START は右にそろえる。
    gui::TextLayout l("שלום\nשלום עולם", gui::Font(14), 1.0f);
    CHECK(l.line_count() == 2);
    CHECK(near(l.line_rect(0).right(), l.width()));
    CHECK(l.line_rect(0).x > 0);
    CHECK(near(l.line_rect(1).x, 0));
}

TEST(text_layout_arabic_shaping)
{
    // アラビア語は文字がつながる(連結した形は、単独の形を並べたより幅が狭くなるのが普通)。
    const std::string word = "مرحبا";
    gui::TextLayout joined(word, gui::Font(14), 1.0f);
    float separate = 0;
    for (std::size_t i = 0; i < word.size(); i += 2)
    {
        separate += gui::TextLayout(word.substr(i, 2), gui::Font(14), 1.0f).width();
    }
    CHECK(joined.line_is_rtl(0));
    CHECK(joined.width() < separate);
}

TEST(text_layout_scales_with_display_scale)
{
    gui::TextLayout a("Hello 日本語", gui::Font(14), 1.0f);
    gui::TextLayout b("Hello 日本語", gui::Font(14), 2.0f);
    // dp で返すので、倍率が変わってもほぼ同じ幅(ヒンティングの差の分だけずれる)。
    CHECK(b.width() > a.width() * 0.9f && b.width() < a.width() * 1.1f);
}

TEST(property_read_write)
{
    gui::Window window("t", 300, 200);
    gui::Label* label = new gui::Label(&window, "abc");
    gui::Label* other = new gui::Label(&window);
    CHECK(label->text == "abc");
    label->text = "x";
    CHECK(label->text.get() == "x");
    label->text += "yz";  // 複合代入
    CHECK(label->text == "xyz");
    other->text = label->text;  // プロパティ同士の代入は値のコピー
    CHECK(other->text == "xyz");
    const std::string joined = "<" + label->text + ">";  // std::string と直接つなげる
    CHECK(joined == "<xyz>");

    label->bounds = gui::Rect(10, 20, 100, 30);
    CHECK(label->x == 10.0f);
    label->x += 5;
    label->width = 50;
    const gui::Rect b = label->bounds;
    CHECK(b == gui::Rect(15, 20, 50, 30));
    CHECK(label->local_rect.get() == gui::Rect(0, 0, 50, 30));
    CHECK(label->parent == static_cast<gui::Widget*>(&window));
    CHECK(label->window == &window);
}

TEST(handler_property)
{
    gui::Window window("t", 300, 200);
    TestButton* a = new TestButton(&window);
    TestButton* b = new TestButton(&window);
    gui::Object* last = nullptr;
    int count = 0;
    const gui::NotifyHandler shared = [&](gui::Object* sender)
    {
        last = sender;
        ++count;
    };
    a->on_click = shared;
    b->on_click = a->on_click;  // 1 つのハンドラを共有し、sender で区別する
    a->key(gui::Key::SPACE);
    CHECK(last == a);
    b->key(gui::Key::SPACE);
    CHECK(last == b);
    b->on_click = nullptr;  // 解除
    b->key(gui::Key::SPACE);
    CHECK(count == 2);
}

TEST(widget_handler_and_propagation)
{
    gui::Window window("t", 300, 200);
    TestButton* button = new TestButton(&window);
    int downs = 0;
    button->on_mouse_down = [&](gui::Object*, gui::MouseEvent&) { ++downs; };
    CHECK(button->press(gui::MouseButton::LEFT));    // ボタンが受け取る
    CHECK(downs == 1);                               // 上書きした仮想関数からもハンドラが呼ばれる
    button->on_mouse_down = nullptr;
    CHECK(!button->press(gui::MouseButton::RIGHT));  // 受け取らない(親へ送られる)
    CHECK(!button->key(gui::Key::A));
}

TEST(widget_ownership)
{
    gui::Window window("t", 200, 100);
    int destroyed = 0;
    Counted* parent_widget = new Counted(&window, &destroyed);
    new Counted(parent_widget, &destroyed);
    Counted* child2 = new Counted(parent_widget, &destroyed);
    CHECK(parent_widget->children.get().size() == 2);
    delete child2;  // 子を先に破棄すると親の一覧から外れる
    CHECK(destroyed == 1);
    CHECK(parent_widget->children.get().size() == 1);
    delete parent_widget;  // 親を破棄すると残りの子も破棄される
    CHECK(destroyed == 3);
    CHECK(window.children.get().empty());
}

TEST(window_destroys_children)
{
    int destroyed = 0;
    {
        gui::Window window("t", 200, 100);
        new Counted(new Counted(&window, &destroyed), &destroyed);
    }
    CHECK(destroyed == 2);
}

TEST(child_at_and_mapping)
{
    gui::Window window("t", 300, 200);
    gui::Widget* a = new gui::Widget(&window);
    a->bounds = gui::Rect(10, 20, 100, 50);
    gui::Widget* b = new gui::Widget(a);
    b->bounds = gui::Rect(5, 5, 10, 10);
    CHECK(window.child_at(gui::Point(16, 26)) == b);
    CHECK(window.child_at(gui::Point(50, 40)) == a);
    CHECK(window.child_at(gui::Point(200, 150)) == nullptr);
    CHECK(b->map_to_window(gui::Point(1, 1)) == gui::Point(16, 26));
    b->visible = false;
    CHECK(window.child_at(gui::Point(16, 26)) == a);
    CHECK(!b->showing);
}

TEST(line_edit_editing)
{
    gui::Window window("t", 300, 100);
    TestEdit* edit = new TestEdit(&window);
    edit->bounds = gui::Rect(0, 0, 200, 30);
    int changes = 0;
    edit->on_change = [&](gui::Object*) { ++changes; };
    edit->focused = true;
    CHECK(edit->focused);

    edit->type("abc");
    CHECK(edit->text == "abc");
    CHECK(edit->cursor_position == 3u);
    edit->key(gui::Key::LEFT);
    edit->key(gui::Key::BACKSPACE);
    CHECK(edit->text == "ac");
    CHECK(edit->cursor_position == 1u);
    edit->key(gui::Key::DEL);
    CHECK(edit->text == "a");

    edit->type("日本");
    CHECK(edit->text == "a日本");
    edit->key(gui::Key::BACKSPACE);  // マルチバイトの文字を 1 文字消す
    CHECK(edit->text == "a日");

    edit->key(gui::Key::HOME);
    edit->key(gui::Key::END, gui::MODIFIER_SHIFT);
    CHECK(edit->selected_text == "a日");
    CHECK(edit->has_selection);
    edit->type("x");  // 選択を置き換える
    CHECK(edit->text == "x");
    CHECK(changes == 6);
}

TEST(line_edit_preedit)
{
    gui::Window window("t", 300, 100);
    TestEdit* edit = new TestEdit(&window);
    edit->bounds = gui::Rect(0, 0, 200, 30);
    edit->focused = true;
    edit->type("ab");
    edit->key(gui::Key::LEFT);

    edit->compose("にほん", 0, 3);
    CHECK(edit->text == "ab");       // 変換中の文字列はテキストに入らない
    edit->key(gui::Key::BACKSPACE);  // 変換中のキーは IME のもの
    CHECK(edit->text == "ab");
    edit->compose("");
    edit->type("日本");  // 確定
    CHECK(edit->text == "a日本b");
    CHECK(edit->cursor_position == 7u);
}

TEST(line_edit_strips_control_characters)
{
    gui::Window window("t", 300, 100);
    gui::LineEdit* edit = new gui::LineEdit(&window, "a\nb\tc");
    CHECK(edit->text == "abc");
    edit->insert("x\r\ny");
    CHECK(edit->text == "abcxy");
    edit->text = "1\n2";
    CHECK(edit->text == "12");
}

TEST(button_click_by_key)
{
    gui::Window window("t", 300, 100);
    TestButton* button = new TestButton(&window);
    int clicks = 0;
    button->on_click = [&](gui::Object*) { ++clicks; };
    button->key(gui::Key::SPACE);
    button->key(gui::Key::RETURN);
    button->key(gui::Key::A);
    CHECK(clicks == 2);
}

TEST(focus_cleared_when_hidden)
{
    gui::Window window("t", 300, 100);
    gui::Widget* panel = new gui::Widget(&window);
    gui::LineEdit* edit = new gui::LineEdit(panel);
    edit->focused = true;
    CHECK(window.focused_widget == static_cast<gui::Widget*>(edit));
    panel->visible = false;
    CHECK(window.focused_widget == nullptr);
    panel->visible = true;
    edit->focused = true;
    delete panel;  // フォーカスのあるウィジェットを破棄してもぶら下がらない
    CHECK(window.focused_widget == nullptr);
}

TEST(timer_repeats_on_main_thread)
{
    gui::Application* app = gui::Application::instance();
    gui::Window window("t", 100, 100);
    window.visible = true;
    gui::Timer timer;
    timer.interval = 10;
    int fired = 0;
    timer.on_timer = [&](gui::Object* sender)
    {
        CHECK(sender == &timer);
        if (++fired == 3)
        {
            app->quit(42);
        }
    };
    timer.enabled = true;
    CHECK(timer.enabled);
    CHECK(app->exec() == 42);
    CHECK(fired == 3);
    CHECK(timer.enabled);  // 繰り返すタイマーは動いたまま
}

TEST(timer_single_shot)
{
    gui::Application* app = gui::Application::instance();
    gui::Window window("t", 100, 100);
    window.visible = true;
    gui::Timer once;
    once.interval = 5;
    once.repeat = false;
    int fired = 0;
    once.on_timer = [&](gui::Object*) { ++fired; };
    once.enabled = true;

    gui::Timer stopper;  // once が 2 回目を呼ばないことを確かめるため、少し待ってから終える
    stopper.interval = 60;
    stopper.on_timer = [&](gui::Object*) { app->quit(); };
    stopper.enabled = true;
    app->exec();
    CHECK(fired == 1);
    CHECK(!once.enabled);  // 1 回だけのタイマーは呼んだ後に止まる
}

TEST(timer_destroyed_while_running)
{
    gui::Application* app = gui::Application::instance();
    gui::Window window("t", 100, 100);
    window.visible = true;
    int fired = 0;
    {
        gui::Timer t;
        t.interval = 1;
        t.on_timer = [&](gui::Object*) { ++fired; };
        t.enabled = true;
    }  // 破棄すると止まる(すでに積まれたイベントが届いても呼ばない)
    gui::Timer stopper;
    stopper.interval = 30;
    stopper.on_timer = [&](gui::Object*) { app->quit(); };
    stopper.enabled = true;
    app->exec();
    CHECK(fired == 0);
}

TEST(handler_shared_between_widget_and_timer)
{
    gui::Window window("t", 100, 100);
    TestButton* button = new TestButton(&window);
    gui::Timer timer;
    int from_button = 0, from_timer = 0;
    // sender は Object*。種類は dynamic_cast で見分ける。
    const gui::NotifyHandler refresh = [&](gui::Object* sender)
    {
        if (dynamic_cast<gui::Button*>(sender))
        {
            ++from_button;
        }
        else if (dynamic_cast<gui::Timer*>(sender))
        {
            ++from_timer;
        }
    };
    button->on_click = refresh;
    timer.on_timer = button->on_click;
    button->key(gui::Key::SPACE);
    timer.on_timer.get()(&timer);
    CHECK(from_button == 1);
    CHECK(from_timer == 1);
}

struct CountedObject : gui::Object
{
    CountedObject(gui::Object* owner_object, int* counter) : gui::Object(owner_object), counter_(counter) {}
    ~CountedObject() { ++*counter_; }

private:
    int* counter_;
};

TEST(object_ownership)
{
    int destroyed = 0;
    {
        gui::Window window("t", 100, 100);
        gui::Widget* panel = new gui::Widget(&window);
        new CountedObject(&window, &destroyed);
        new CountedObject(panel, &destroyed);  // ウィジェットもウィジェット以外のオブジェクトを所有できる
        gui::Timer* timer = new gui::Timer(&window);
        CHECK(timer->owner == static_cast<gui::Object*>(&window));
        CHECK(panel->owner == static_cast<gui::Object*>(&window));
        CountedObject* early = new CountedObject(&window, &destroyed);
        delete early;  // 先に破棄すると所有者の一覧から外れる
        CHECK(destroyed == 1);
    }
    CHECK(destroyed == 3);
}

TEST(binding_variable_and_widgets)
{
    gui::Window window("t", 300, 100);
    TestEdit* edit = new TestEdit(&window);
    edit->bounds = gui::Rect(0, 0, 200, 30);
    gui::Label* label = new gui::Label(&window);
    gui::StringVariable* name = new gui::StringVariable(&window, "初期値");
    int changes = 0, sets = 0;
    name->on_change = [&](gui::Object* sender)
    {
        CHECK(sender == name);
        ++changes;
    };
    name->on_set = [&](gui::Object*) { ++sets; };

    gui::bind(edit->text, name->value);  // 結んだ時点で edit を name の値にする
    gui::bind(label->text, name->value);
    CHECK(edit->text == "初期値");
    CHECK(label->text == "初期値");
    CHECK(changes == 0);

    // 利用者の操作 → 変数 → もう一方のウィジェット
    edit->focused = true;
    edit->key(gui::Key::END);
    edit->type("!");
    CHECK(name->value == "初期値!");
    CHECK(label->text == "初期値!");
    CHECK(changes == 1);

    // 変数への代入 → 両方のウィジェット
    name->value = "abc";
    CHECK(edit->text == "abc");
    CHECK(label->text == "abc");
    CHECK(changes == 2);

    // 同じ値の代入では on_change は呼ばず、on_set だけ呼ぶ(docs/adr/0016)
    const int sets_before = sets;
    name->value = "abc";
    CHECK(changes == 2);
    CHECK(sets == sets_before + 1);

    // ウィジェットのプロパティへの代入 → 変数 → もう一方のウィジェット
    label->text = "xyz";
    CHECK(name->value == "xyz");
    CHECK(edit->text == "xyz");
}

TEST(binding_one_way_from_readonly)
{
    gui::Window window("t", 300, 100);
    TestEdit* edit = new TestEdit(&window, "hello");
    gui::Label* label = new gui::Label(&window);
    gui::bind(label->text, edit->selected_text);
    CHECK(label->text == "");
    edit->select_all();
    CHECK(label->text == "hello");
    edit->deselect();
    CHECK(label->text == "");
    label->text = "x";  // 読み取り専用の側へは伝えない
    CHECK(edit->text == "hello");
}

TEST(binding_dependent_properties)
{
    gui::Window window("t", 300, 100);
    gui::Widget* w = new gui::Widget(&window);
    gui::Variable<float>* width = new gui::Variable<float>(&window, 0);
    int changes = 0;
    width->on_change = [&](gui::Object*) { ++changes; };
    gui::bind(width->value, w->width);  // width->value を w の幅にする
    w->bounds = gui::Rect(0, 0, 40, 10);  // bounds を変えると、一緒に変わった width も知らせる
    CHECK(width->value == 40.0f);
    width->value = 25;
    CHECK(w->width == 25.0f);
    const gui::Rect b = w->bounds;
    CHECK(b.width == 25.0f);
    w->x = 5;  // 幅は変わらないので知らせない
    CHECK(changes == 2);  // bounds の代入・value の代入(結んだときは同じ値なので知らせない)
}

TEST(binding_focus)
{
    gui::Window window("t", 300, 100);
    gui::LineEdit* a = new gui::LineEdit(&window);
    gui::LineEdit* b = new gui::LineEdit(&window);
    gui::BoolVariable* a_focused = new gui::BoolVariable(&window, false);
    gui::bind(a_focused->value, a->focused);
    a->focused = true;
    CHECK(a_focused->value);
    b->focused = true;  // フォーカスが移ると、移る前のウィジェットの focused も知らせる
    CHECK(!a_focused->value);
    a_focused->value = true;  // 変数からフォーカスを動かす
    CHECK(a->focused);
}

TEST(binding_cycle_terminates)
{
    gui::Window window("t", 100, 100);
    gui::IntVariable* a = new gui::IntVariable(&window);
    gui::IntVariable* b = new gui::IntVariable(&window);
    gui::IntVariable* c = new gui::IntVariable(&window);
    gui::bind(a->value, b->value);
    gui::bind(b->value, c->value);
    gui::bind(c->value, a->value);
    int changes = 0;
    a->on_change = [&](gui::Object*) { ++changes; };
    a->value = 7;  // 循環していても止まる。値が変わったときだけ伝えるので、通知は重複しない
    CHECK(b->value == 7);
    CHECK(c->value == 7);
    CHECK(changes == 1);
}

TEST(binding_removed_with_object)
{
    gui::Window window("t", 100, 100);
    const std::size_t before = gui::detail::binding_count();
    gui::StringVariable* v = new gui::StringVariable(&window, "a");
    gui::Label* label = new gui::Label(&window);
    gui::LineEdit* edit = new gui::LineEdit(&window);
    gui::bind(label->text, v->value);
    gui::bind(edit->text, v->value);
    gui::bind(edit->text, v->value);  // 同じ組は 2 度結ばない
    CHECK(gui::detail::binding_count() == before + 2);
    delete label;  // 破棄すると、その結びつきは外れる
    CHECK(gui::detail::binding_count() == before + 1);
    v->value = "b";
    CHECK(edit->text == "b");
    gui::unbind(edit->text, v->value);
    CHECK(gui::detail::binding_count() == before);
    v->value = "c";
    CHECK(edit->text == "b");
}

struct TestCheck : gui::CheckBox
{
    explicit TestCheck(gui::Widget* parent_widget) : gui::CheckBox(parent_widget, "c") {}
    void press_space()
    {
        gui::KeyEvent e(gui::Key::SPACE, gui::MODIFIER_NONE, false, 0);
        key_down(e);
    }
};

struct TestRadio : gui::RadioButton
{
    TestRadio(gui::Widget* parent_widget, int v) : gui::RadioButton(parent_widget, "r", v) {}
    void press_space()
    {
        gui::KeyEvent e(gui::Key::SPACE, gui::MODIFIER_NONE, false, 0);
        key_down(e);
    }
};

TEST(check_box_toggles_and_binds)
{
    gui::Window window("t", 200, 100);
    TestCheck* check = new TestCheck(&window);
    gui::BoolVariable* flag = new gui::BoolVariable(&window, true);
    int changes = 0, clicks = 0;
    check->on_change = [&](gui::Object*) { ++changes; };
    check->on_click = [&](gui::Object*) { ++clicks; };
    gui::bind(check->checked, flag->value);
    CHECK(check->checked);
    CHECK(changes == 1);  // 結んだときに false から true に変わった
    check->press_space();
    CHECK(!check->checked);
    CHECK(!flag->value);
    CHECK(changes == 2);
    CHECK(clicks == 1);
    flag->value = true;
    CHECK(check->checked);
    CHECK(clicks == 1);  // 代入では on_click は呼ばない
}

TEST(radio_group_via_variable)
{
    gui::Window window("t", 200, 100);
    TestRadio* r1 = new TestRadio(&window, 1);
    TestRadio* r2 = new TestRadio(&window, 2);
    TestRadio* r3 = new TestRadio(&window, 3);
    gui::IntVariable* choice = new gui::IntVariable(&window, 1);
    int choice_changes = 0, r1_changes = 0;
    choice->on_change = [&](gui::Object*) { ++choice_changes; };
    r1->on_change = [&](gui::Object*) { ++r1_changes; };
    gui::bind(r1->group_value, choice->value);
    gui::bind(r2->group_value, choice->value);
    gui::bind(r3->group_value, choice->value);
    CHECK(r1->checked);
    CHECK(!r2->checked);
    CHECK(!r3->checked);

    r2->press_space();  // 押したボタンだけが選ばれ、変数も変わる
    CHECK(choice->value == 2);
    CHECK(!r1->checked);
    CHECK(r2->checked);
    CHECK(!r3->checked);
    CHECK(choice_changes == 1);
    CHECK(r1_changes == 2);  // 結んだとき選ばれ、r2 を押して外れた

    r2->press_space();  // 選ばれているボタンを押しても変わらない
    CHECK(choice_changes == 1);

    choice->value = 3;  // 変数から選ぶ
    CHECK(r3->checked);
    CHECK(!r2->checked);
}

// 望む大きさ・最小の大きさを指定できるウィジェット。
struct Fixed : gui::Widget
{
    Fixed(gui::Widget* parent_widget, float w, float h, float min_w = 0, float min_h = 0)
        : gui::Widget(parent_widget), preferred_(w, h), minimum_(min_w, min_h)
    {
    }

    void set_preferred(float w, float h)
    {
        preferred_ = gui::Size(w, h);
        request_layout();
    }

protected:
    gui::Size compute_preferred_size() const override { return preferred_; }
    gui::Size compute_minimum_size() const override { return minimum_; }

private:
    gui::Size preferred_;
    gui::Size minimum_;
};

bool rect_is(const gui::Widget* w, float x, float y, float width, float height)
{
    const gui::Rect r = w->bounds;
    const bool ok = near(r.x, x) && near(r.y, y) && near(r.width, width) && near(r.height, height);
    if (!ok)
    {
        std::printf("    bounds = (%g, %g, %g, %g), expected (%g, %g, %g, %g)\n", r.x, r.y, r.width, r.height, x, y, width,
                    height);
    }
    return ok;
}

TEST(box_layout_stretch_and_spacing)
{
    gui::Window window("t", 400, 200);
    gui::Widget* panel = new gui::Widget(&window);
    panel->bounds = gui::Rect(0, 0, 300, 40);
    gui::BoxLayout* box = new gui::BoxLayout(panel, gui::Orientation::HORIZONTAL);
    CHECK(panel->layout == static_cast<gui::Layout*>(box));
    box->spacing = 10;
    Fixed* a = new Fixed(panel, 50, 20);
    Fixed* b = new Fixed(panel, 20, 20);
    Fixed* c = new Fixed(panel, 30, 20);
    box->add(a);
    box->add(b, 1);  // 余った幅を受け取る
    box->add(c, 0, gui::Align::CENTER);
    panel->apply_layout();
    CHECK(rect_is(a, 0, 0, 50, 40));    // 直交する向きは既定で FILL
    CHECK(rect_is(b, 60, 0, 200, 40));  // 300 - 50 - 30 - 10 * 2
    CHECK(rect_is(c, 270, 10, 30, 20)); // CENTER
    // 望む大きさ: 並べる向きは合計 + 間隔、直交する向きは最大
    const gui::Size pref = panel->preferred_size;
    CHECK(near(pref.width, 120) && near(pref.height, 20));

    b->visible = false;  // 隠れた項目は詰める
    panel->apply_layout();
    CHECK(rect_is(c, 60, 10, 30, 20));
}

TEST(box_layout_shrinks_to_minimum)
{
    gui::Window window("t", 400, 200);
    gui::Widget* panel = new gui::Widget(&window);
    panel->bounds = gui::Rect(0, 0, 40, 100);
    gui::BoxLayout* box = new gui::BoxLayout(panel);  // 縦
    box->spacing = 0;
    Fixed* a = new Fixed(panel, 10, 60, 0, 40);  // 縮められるのは 20
    Fixed* b = new Fixed(panel, 10, 60, 0, 20);  // 縮められるのは 40
    box->add(a);
    box->add(b);
    panel->apply_layout();  // 120 → 100: 差 20 を 1:2 で縮める(53.3 と 46.7。辺はピクセルに揃える)
    CHECK(rect_is(a, 0, 0, 40, 53));
    CHECK(rect_is(b, 0, 53, 40, 47));
    const gui::Size min = panel->minimum_hint;
    CHECK(near(min.height, 60));
}

TEST(box_layout_spacers_and_margins)
{
    gui::Window window("t", 400, 200);
    gui::Widget* panel = new gui::Widget(&window);
    panel->bounds = gui::Rect(0, 0, 200, 50);
    gui::BoxLayout* box = new gui::BoxLayout(panel, gui::Orientation::HORIZONTAL);
    box->spacing = 0;
    box->margins = gui::Margins(5);
    Fixed* ok = new Fixed(panel, 40, 20);
    Fixed* cancel = new Fixed(panel, 40, 20);
    box->add_stretch();  // 右に寄せる
    box->add(ok);
    box->add_spacing(10);
    box->add(cancel);
    panel->apply_layout();
    CHECK(rect_is(ok, 105, 5, 40, 40));
    CHECK(rect_is(cancel, 155, 5, 40, 40));
}

TEST(grid_layout_cells_spans_and_stretch)
{
    gui::Window window("t", 400, 200);
    gui::Widget* panel = new gui::Widget(&window);
    panel->bounds = gui::Rect(0, 0, 300, 100);
    gui::GridLayout* grid = new gui::GridLayout(panel);
    grid->row_spacing = 10;
    grid->column_spacing = 10;
    Fixed* label = new Fixed(panel, 60, 20);
    Fixed* edit = new Fixed(panel, 100, 30);
    Fixed* wide = new Fixed(panel, 50, 20);
    grid->add(label, 0, 0);
    grid->add(edit, 0, 1);
    grid->add(wide, 1, 0, 1, 2);  // 2 列にまたがる
    grid->set_column_stretch(1, 1);
    panel->apply_layout();
    // 列 0 は 60、列 1 は残り(300 - 60 - 10 = 230)。行 0 は 30(最大)、行 1 は 20。
    CHECK(rect_is(label, 0, 5, 60, 20));  // 縦は既定で CENTER
    CHECK(rect_is(edit, 70, 0, 230, 30));
    CHECK(rect_is(wide, 0, 40, 300, 20));
    const gui::Size pref = panel->preferred_size;
    CHECK(near(pref.width, 170) && near(pref.height, 60));
}

TEST(grid_layout_span_widens_columns)
{
    gui::Window window("t", 400, 200);
    gui::Widget* panel = new gui::Widget(&window);
    gui::GridLayout* grid = new gui::GridLayout(panel);
    grid->column_spacing = 0;
    new Fixed(panel, 10, 10);
    Fixed* a = new Fixed(panel, 20, 10);
    Fixed* b = new Fixed(panel, 20, 10);
    Fixed* span = new Fixed(panel, 100, 10);
    grid->add(a, 0, 0);
    grid->add(b, 0, 1);
    grid->add(span, 1, 0, 1, 2);  // 40 では足りないので、2 列に 30 ずつ足す
    const gui::Size pref = panel->preferred_size;
    CHECK(near(pref.width, 100));
}

TEST(place_layout_relative_and_anchor)
{
    gui::Window window("t", 400, 200);
    gui::Widget* panel = new gui::Widget(&window);
    panel->bounds = gui::Rect(0, 0, 200, 100);
    gui::PlaceLayout* place = new gui::PlaceLayout(panel);
    Fixed* center = new Fixed(panel, 40, 20);
    Fixed* bar = new Fixed(panel, 10, 10);
    gui::Placement p;
    p.relative_x = 0.5f;
    p.relative_y = 0.5f;
    p.anchor = gui::Anchor::CENTER;
    place->add(center, p);
    gui::Placement q;
    q.relative_y = 1;
    q.relative_width = 1;
    q.height = 8;
    q.anchor = gui::Anchor::BOTTOM_LEFT;
    place->add(bar, q);
    panel->apply_layout();
    CHECK(rect_is(center, 80, 40, 40, 20));
    CHECK(rect_is(bar, 0, 92, 200, 8));
    panel->bounds = gui::Rect(0, 0, 100, 50);  // 親の大きさが変わると置き直す
    panel->apply_layout();
    CHECK(rect_is(center, 30, 15, 40, 20));
}

TEST(layout_nesting_through_container_widgets)
{
    gui::Window window("t", 400, 200);
    gui::BoxLayout* column = new gui::BoxLayout(&window);  // ウィンドウ自身にもレイアウトを付けられる
    column->spacing = 0;
    Fixed* top = new Fixed(&window, 50, 30);
    gui::Widget* row = new gui::Widget(&window);  // 入れ子は入れ物のウィジェットを介す
    gui::BoxLayout* row_box = new gui::BoxLayout(row, gui::Orientation::HORIZONTAL);
    row_box->spacing = 0;
    Fixed* left = new Fixed(row, 70, 25);
    Fixed* right = new Fixed(row, 30, 40);
    row_box->add(left, 1);
    row_box->add(right);
    column->add(top);
    column->add(row);
    window.apply_layout();
    CHECK(rect_is(row, 0, 30, 400, 40));  // 入れ物の望む高さは子の最大
    CHECK(rect_is(left, 0, 0, 370, 40));
    CHECK(rect_is(right, 370, 0, 30, 40));

    left->set_preferred(70, 60);  // 子の望む大きさが変わると、祖先まで配置し直す
    window.apply_layout();
    CHECK(rect_is(row, 0, 30, 400, 60));

    window.visible = true;
    window.width = 300;  // ウィンドウの大きさが変わると配置し直す
    window.apply_layout();
    CHECK(near(window.width, 300));
    CHECK(rect_is(left, 0, 0, 270, 60));
}

TEST(layout_follows_text_changes)
{
    gui::Window window("t", 400, 200);
    gui::Widget* panel = new gui::Widget(&window);
    panel->bounds = gui::Rect(0, 0, 400, 40);
    gui::BoxLayout* box = new gui::BoxLayout(panel, gui::Orientation::HORIZONTAL);
    gui::Label* label = new gui::Label(panel, "a");
    Fixed* after = new Fixed(panel, 10, 10);
    box->add(label);
    box->add(after);
    panel->apply_layout();
    const float x1 = after->x;
    label->text = "ずっと長いテキスト";  // ラベルの望む幅が変わる
    panel->apply_layout();
    CHECK(after->x > x1);
}

TEST(layout_replace_remove_and_destroy)
{
    gui::Window window("t", 400, 200);
    gui::Widget* panel = new gui::Widget(&window);
    panel->bounds = gui::Rect(0, 0, 100, 100);
    int destroyed = 0;
    struct CountedLayout : gui::BoxLayout
    {
        CountedLayout(gui::Widget* t, int* c) : gui::BoxLayout(t), counter(c) {}
        ~CountedLayout() { ++*counter; }
        int* counter;
    };
    new CountedLayout(panel, &destroyed);
    gui::GridLayout* grid = new gui::GridLayout(panel);  // 2 つ目を付けると古いレイアウトは破棄される
    CHECK(destroyed == 1);
    CHECK(panel->layout == static_cast<gui::Layout*>(grid));

    Fixed* a = new Fixed(panel, 10, 10);
    Fixed* b = new Fixed(panel, 10, 10);
    grid->add(a, 0, 0);
    grid->add(b, 0, 1);
    delete a;  // 管理している子を破棄すると、レイアウトから外れる
    panel->apply_layout();
    CHECK(rect_is(b, 0, 0, 10, 10));  // 列 0 は空(幅 0、間隔も入れない)、列 1 に b

    Fixed* other = new Fixed(&window, 10, 10);
    grid->add(other, 1, 1);  // 子でないウィジェットは子にする(親を付け替える)
    CHECK(other->parent == panel);
    panel->apply_layout();
    CHECK(rect_is(other, 0, 18, 10, 10));
    grid->add(&window, 2, 0);  // ウィンドウ(祖先)は入れられない
    CHECK(window.parent == nullptr);

    delete grid;  // 外すと自由配置に戻る
    CHECK(panel->layout == nullptr);
    b->bounds = gui::Rect(1, 2, 3, 4);
    panel->apply_layout();
    CHECK(rect_is(b, 1, 2, 3, 4));
}

TEST(layout_notifies_bindings)
{
    gui::Window window("t", 400, 200);
    gui::Widget* panel = new gui::Widget(&window);
    panel->bounds = gui::Rect(0, 0, 200, 40);
    gui::BoxLayout* box = new gui::BoxLayout(panel, gui::Orientation::HORIZONTAL);
    Fixed* a = new Fixed(panel, 10, 10);
    box->add(a, 1);
    gui::Variable<float>* width = new gui::Variable<float>(&window, 0);
    gui::bind(width->value, a->width);
    panel->apply_layout();
    CHECK(near(width->value, 200));  // レイアウトが決めた幅が、結んだ変数に伝わる
    panel->width = 120;
    panel->apply_layout();
    CHECK(near(width->value, 120));
}

TEST(text_input_enabled_when_shown_after_focus)
{
    // 表示の前にフォーカスを移しても、表示したときに文字入力(IME)が有効になる(サンプルで見つかった不具合)。
    gui::Window window("t", 200, 100);
    gui::LineEdit* edit = new gui::LineEdit(&window);
    edit->focused = true;
    CHECK(!window.impl()->text_input_active);  // 表示されていないうちは有効にしない
    window.visible = true;
    CHECK(window.impl()->text_input_active);
    CHECK(SDL_TextInputActive(window.impl()->window));

    gui::Widget* panel = new gui::Widget(&window);
    gui::LineEdit* inner = new gui::LineEdit(panel);
    panel->visible = false;
    inner->focused = true;  // 隠れているのでフォーカスは移らない
    CHECK(window.focused_widget == static_cast<gui::Widget*>(edit));
}

TEST(reparent_moves_ownership_and_layout)
{
    gui::Window window("t", 300, 200);
    gui::Widget* a = new gui::Widget(&window);
    gui::Widget* b = new gui::Widget(&window);
    gui::BoxLayout* box_a = new gui::BoxLayout(a);
    int destroyed = 0;
    Counted* child = new Counted(a, &destroyed);
    box_a->add(child);
    CHECK(a->children.get().size() == 1);

    child->parent = b;  // 付け替えると、表示のツリー・所有者・レイアウトから移る
    CHECK(child->parent == b);
    CHECK(child->owner == static_cast<gui::Object*>(b));
    CHECK(a->children.get().empty());
    CHECK(b->children.get().size() == 1);
    delete a;  // 元の親を破棄しても、移ったウィジェットは残る
    CHECK(destroyed == 0);
    delete b;  // 新しい親と一緒に破棄される
    CHECK(destroyed == 1);
}

TEST(reparent_refuses_cycles_and_windows)
{
    gui::Window window("t", 300, 200);
    gui::Widget* a = new gui::Widget(&window);
    gui::Widget* b = new gui::Widget(a);
    a->parent = b;  // 子孫を親にはできない
    CHECK(a->parent == static_cast<gui::Widget*>(&window));
    gui::Window other("u", 100, 100);
    other.parent = a;  // ウィンドウは付け替えられない
    CHECK(other.parent == nullptr);
}

TEST(reparent_to_other_window_clears_focus)
{
    gui::Window w1("t", 300, 200);
    gui::Window w2("u", 300, 200);
    gui::LineEdit* edit = new gui::LineEdit(&w1);
    edit->focused = true;
    CHECK(w1.focused_widget == static_cast<gui::Widget*>(edit));
    edit->parent = &w2;  // 別のウィンドウへ移ると、元のウィンドウのフォーカスは外れる
    CHECK(w1.focused_widget == nullptr);
    CHECK(edit->window == &w2);
    edit->focused = true;
    CHECK(w2.focused_widget == static_cast<gui::Widget*>(edit));
}

TEST(label_alignment)
{
    gui::Window window("t", 300, 200);
    gui::Label* label = new gui::Label(&window, "abc");
    label->alignment = gui::Align::END;
    CHECK(label->alignment == gui::Align::END);
}

TEST(line_edit_word_navigation)
{
    gui::Window window("t", 300, 100);
    TestEdit* edit = new TestEdit(&window, "hello world foo");
    edit->bounds = gui::Rect(0, 0, 200, 30);
    edit->focused = true;
    edit->key(gui::Key::HOME);
    edit->key(gui::Key::RIGHT, gui::MODIFIER_CONTROL);
    CHECK(edit->cursor_position == 6u);
    edit->key(gui::Key::RIGHT, gui::MODIFIER_CONTROL | gui::MODIFIER_SHIFT);
    CHECK(edit->selected_text == "world ");
    edit->key(gui::Key::END);
    edit->key(gui::Key::BACKSPACE, gui::MODIFIER_CONTROL);  // 単語の始めまで消す
    CHECK(edit->text == "hello world ");
    edit->key(gui::Key::HOME);
    edit->key(gui::Key::DEL, gui::MODIFIER_CONTROL);  // 次の単語の始めまで消す
    CHECK(edit->text == "world ");
}

TEST(line_edit_rtl_text)
{
    gui::Window window("t", 300, 100);
    TestEdit* edit = new TestEdit(&window, "abc ");
    edit->bounds = gui::Rect(0, 0, 200, 30);
    edit->focused = true;
    edit->type("שלום");  // 右から左の文字も入力できる
    CHECK(edit->text == "abc שלום");
    edit->key(gui::Key::LEFT);  // 移動は論理の順
    CHECK(edit->cursor_position == 10u);
    edit->key(gui::Key::BACKSPACE);  // 論理の直前の文字(ו)を消す
    CHECK(edit->text == "abc שלם");
}

TEST(label_word_wrap)
{
    gui::Window window("t", 400, 300);
    gui::BoxLayout* column = new gui::BoxLayout(&window);
    column->margins = gui::Margins(0);
    gui::Label* label = new gui::Label(&window, "折り返しのあるラベルは、幅に応じて高さが変わる。Wrapping labels change their height.");
    label->word_wrap = true;
    column->add(label);
    column->add_stretch();
    window.apply_layout();
    const float wide_height = label->height;
    window.width = 120;  // 幅を狭めると行が増え、高さが増える
    window.visible = true;
    window.apply_layout();
    CHECK(label->height > wide_height);
    const gui::Size min = label->minimum_hint;
    CHECK(min.width < 120);  // 最小の幅は最も長い単語の幅
}

// 保護された仮想関数・状態を確かめるための TextEdit。
struct TestMemo : gui::TextEdit
{
    explicit TestMemo(gui::Widget* parent_widget, const std::string& s = std::string()) : gui::TextEdit(parent_widget, s) {}

    void key(gui::Key k, gui::Modifiers m = gui::MODIFIER_NONE)
    {
        gui::KeyEvent e(k, m, false, 0);
        key_down(e);
    }

    void type(const std::string& s)
    {
        gui::TextInputEvent e(s);
        text_input(e);
    }

    bool wheel(float dy)
    {
        gui::WheelEvent e(gui::Point(10, 10), gui::Point(0, dy), gui::MODIFIER_NONE);
        mouse_wheel(e);
        return e.accepted;
    }

    gui::Point scroll() const { return scroll_offset(); }
    std::size_t lines() const { return text_layout().line_count(); }
    gui::Rect area() const { return text_area(); }
};

TEST(text_edit_newlines_and_navigation)
{
    gui::Window window("t", 300, 200);
    TestMemo* memo = new TestMemo(&window);
    memo->bounds = gui::Rect(0, 0, 240, 120);
    memo->focused = true;
    memo->type("ab");
    memo->key(gui::Key::RETURN);  // 改行を入れる
    memo->type("cd");
    CHECK(memo->text == "ab\ncd");
    memo->key(gui::Key::UP);  // 上の行へ(横の位置を保つ)
    CHECK(memo->cursor_position.get() <= 2u);
    memo->key(gui::Key::END);  // 行の終わり
    CHECK(memo->cursor_position == 2u);
    memo->key(gui::Key::DOWN);
    memo->key(gui::Key::HOME);  // 行の始め
    CHECK(memo->cursor_position == 3u);
    memo->key(gui::Key::END, gui::MODIFIER_CONTROL);  // 全体の終わり
    CHECK(memo->cursor_position == 5u);
    memo->key(gui::Key::HOME, gui::MODIFIER_CONTROL | gui::MODIFIER_SHIFT);  // 全体を選ぶ
    CHECK(memo->selected_text == "ab\ncd");
}

TEST(text_edit_normalizes_text)
{
    gui::Window window("t", 300, 200);
    gui::TextEdit* memo = new gui::TextEdit(&window);
    memo->insert("a\r\nb\rc\td");  // 改行は \n にそろえ、タブは空白に
    CHECK(memo->text == "a\nb\nc d");
}

TEST(text_edit_scrolls_to_caret)
{
    gui::Window window("t", 300, 300);
    TestMemo* memo = new TestMemo(&window);
    memo->bounds = gui::Rect(0, 0, 200, 80);
    memo->focused = true;
    for (int i = 0; i < 20; ++i)
    {
        memo->type("line\n");
    }
    window.apply_layout();
    CHECK(memo->scroll().y > 0);  // キャレット(最後の行)が見えるようにスクロールする
    memo->key(gui::Key::HOME, gui::MODIFIER_CONTROL);
    CHECK(memo->scroll().y == 0);
    CHECK(memo->wheel(-1));  // ホイールを手前に回すと下へ
    CHECK(memo->scroll().y > 0);
    memo->key(gui::Key::END, gui::MODIFIER_CONTROL);
    const float bottom = memo->scroll().y;
    memo->wheel(-100);  // 範囲の外へは動かない
    CHECK(memo->scroll().y == bottom);
}

TEST(text_edit_word_wrap)
{
    gui::Window window("t", 300, 300);
    TestMemo* memo = new TestMemo(&window, "a long line of text that does not fit in a narrow text edit");
    memo->bounds = gui::Rect(0, 0, 120, 200);
    CHECK(memo->lines() > 1);
    memo->word_wrap = false;
    CHECK(memo->lines() == 1u);
    CHECK(!memo->wheel(-1));  // 縦にスクロールできなければ受け取らない(親へ送る)
}

TEST(scroll_bar_range)
{
    gui::Window window("t", 300, 300);
    gui::ScrollBar* bar = new gui::ScrollBar(&window);
    int changes = 0;
    bar->on_change = [&](gui::Object*) { ++changes; };
    bar->maximum = 100;
    bar->page_step = 50;
    bar->value = 150;  // 範囲に丸める
    CHECK(bar->value == 100.0f);
    bar->maximum = 40;  // 小さくすると value も丸める
    CHECK(bar->value == 40.0f);
    bar->value = -5;
    CHECK(bar->value == 0.0f);
    CHECK(changes == 3);
}

struct TestScroll : gui::ScrollArea
{
    explicit TestScroll(gui::Widget* parent_widget) : gui::ScrollArea(parent_widget) {}
    bool wheel(float dy)
    {
        gui::WheelEvent e(gui::Point(10, 10), gui::Point(0, dy), gui::MODIFIER_NONE);
        mouse_wheel(e);
        return e.accepted;
    }
};

TEST(scroll_area_scrolls_content)
{
    gui::Window window("t", 300, 300);
    TestScroll* area = new TestScroll(&window);
    area->bounds = gui::Rect(0, 0, 200, 100);
    Fixed* content = new Fixed(nullptr, 150, 500);
    area->content = content;  // 中身はスクロール領域の中に移る
    CHECK(content->parent != nullptr);
    window.apply_layout();
    // 縦にだけスクロールし、中身の幅は見えている幅(スクロールバーの分だけ狭い)に合わせる。
    CHECK(near(content->width, 200 - gui::ScrollBar::THICKNESS));
    CHECK(near(content->height, 500));
    area->scroll_position = gui::Point(0, 50);
    CHECK(near(content->y, -50));
    CHECK(area->wheel(-1));
    CHECK(content->y < -50.0f);
    area->scroll_to_visible(gui::Rect(0, 490, 10, 10));  // 最後まで
    CHECK(near(area->scroll_position.get().y, 400));
    area->scroll_position = gui::Point(0, 10000);  // 範囲に丸める
    CHECK(near(area->scroll_position.get().y, 400));

    content->set_preferred(150, 50);  // 中身が収まると、スクロールバーを消して位置を 0 にする
    window.apply_layout();
    CHECK(near(area->scroll_position.get().y, 0));
    CHECK(near(content->width, 200));
    CHECK(!area->wheel(-1));  // スクロールできなければ受け取らない
}

TEST(wheel_goes_to_widget_under_pointer_then_parents)
{
    gui::Window window("t", 300, 300);
    window.visible = true;
    gui::ScrollArea* area = new gui::ScrollArea(&window);
    area->bounds = gui::Rect(0, 0, 200, 100);
    gui::Widget* content = new gui::Widget();
    content->minimum_size = gui::Size(100, 400);
    gui::Label* label = new gui::Label(content, "ラベル");
    label->bounds = gui::Rect(0, 0, 100, 20);
    area->content = content;
    window.apply_layout();
    // ラベルの上でホイールを回す: ラベルは受け取らないので、親の ScrollArea がスクロールする。
    SDL_Event e;
    SDL_zero(e);
    e.type = SDL_EVENT_MOUSE_WHEEL;
    e.wheel.windowID = SDL_GetWindowID(window.impl()->window);
    e.wheel.x = 0;
    e.wheel.y = -1;
    e.wheel.direction = SDL_MOUSEWHEEL_NORMAL;
    const float to_window = window.impl()->scale / window.impl()->density;
    e.wheel.mouse_x = 10 * to_window;
    e.wheel.mouse_y = 10 * to_window;
    window.impl()->handle(e);
    CHECK(area->scroll_position.get().y > 0);
}

// SDL のイベントを、Application のイベントの振り分け(ポップアップの扱いを含む)に直接渡す。
void dispatch_key(gui::Window& window, SDL_Keycode key, bool down = true)
{
    SDL_Event e;
    SDL_zero(e);
    e.type = down ? SDL_EVENT_KEY_DOWN : SDL_EVENT_KEY_UP;
    e.key.windowID = SDL_GetWindowID(window.impl()->window);
    e.key.key = key;
    e.key.down = down;
    gui::Application::instance()->impl()->dispatch(e);
}

void dispatch_click(gui::Window& window, const gui::Point& p)
{
    const float k = window.impl()->scale / window.impl()->density;
    for (int i = 0; i < 2; ++i)
    {
        SDL_Event e;
        SDL_zero(e);
        e.type = i == 0 ? SDL_EVENT_MOUSE_BUTTON_DOWN : SDL_EVENT_MOUSE_BUTTON_UP;
        e.button.windowID = SDL_GetWindowID(window.impl()->window);
        e.button.button = SDL_BUTTON_LEFT;
        e.button.down = i == 0;
        e.button.clicks = 1;
        e.button.x = p.x * k;
        e.button.y = p.y * k;
        gui::Application::instance()->impl()->dispatch(e);
    }
}

TEST(popup_opens_relative_to_owner)
{
    gui::Window window("t", 300, 200);
    window.visible = true;
    gui::Button* button = new gui::Button(&window, "b");
    button->bounds = gui::Rect(20, 30, 80, 24);
    gui::Popup* popup = new gui::Popup(button);
    int closed = 0;
    popup->on_closed = [&](gui::Object*) { ++closed; };
    popup->open(button->local_rect, gui::Size(120, 60));
    CHECK(popup->visible);
    CHECK(popup->impl()->window != nullptr);
    CHECK(near(popup->width, 120) && near(popup->height, 60));
    CHECK(gui::Application::instance()->impl()->open_menus.size() == 1);
    int x = 0, y = 0;
    SDL_GetWindowPosition(popup->impl()->window, &x, &y);
    int px = 0, py = 0;
    SDL_GetWindowPosition(window.impl()->window, &px, &py);
    // ポップアップの位置は、親のウィンドウの button の左下(SDL3 はポップアップの位置を親からの相対で返す)。
    // ポップアップに対応しないドライバ(offscreen)の代わりの通常のウィンドウは、絶対の位置なので確かめない。
    if (!popup->impl()->popup_parent)
    {
        const float k = window.impl()->scale / window.impl()->density;
        CHECK(x == static_cast<int>(20 * k + 0.5f) && y == static_cast<int>(54 * k + 0.5f));
    }
    (void)px;
    (void)py;

    dispatch_click(window, gui::Point(250, 150));  // 外側を押すと閉じる(押したことは捨てる)
    CHECK(!popup->visible);
    CHECK(closed == 1);
    CHECK(gui::Application::instance()->impl()->open_menus.empty());
    delete button;  // 持ち主と一緒に破棄される(ぶら下がらない)
}

TEST(combo_box_selects_with_keys_and_popup)
{
    gui::Window window("t", 300, 300);
    window.visible = true;
    gui::ComboBox* combo = new gui::ComboBox(&window);
    combo->bounds = gui::Rect(10, 10, 150, 30);
    std::vector<std::string> items;
    items.push_back("小");
    items.push_back("中");
    items.push_back("大");
    combo->items = items;
    CHECK(combo->current_index == -1);
    int changes = 0;
    combo->on_change = [&](gui::Object*) { ++changes; };
    combo->focused = true;

    dispatch_key(window, SDLK_DOWN);  // 閉じているときは ↓ で選択を変える
    CHECK(combo->current_index == 0);
    CHECK(combo->current_text == "小");

    dispatch_key(window, SDLK_SPACE);  // 一覧を開く
    CHECK(combo->is_popup_open());
    dispatch_key(window, SDLK_DOWN);  // 開いている間のキーは一覧へ送る
    dispatch_key(window, SDLK_DOWN);
    dispatch_key(window, SDLK_RETURN);  // 決める
    CHECK(!combo->is_popup_open());
    CHECK(combo->current_index == 2);
    CHECK(changes == 2);

    dispatch_key(window, SDLK_SPACE);
    dispatch_key(window, SDLK_UP);
    dispatch_key(window, SDLK_ESCAPE);  // Esc は選ばずに閉じる
    CHECK(!combo->is_popup_open());
    CHECK(combo->current_index == 2);

    items.pop_back();
    combo->items = items;  // 範囲の外になったら -1
    CHECK(combo->current_index == -1);
}

TEST(menu_keyboard_and_submenu)
{
    gui::Window window("t", 400, 300);
    window.visible = true;
    gui::Widget* area = new gui::Widget(&window);
    area->bounds = gui::Rect(0, 0, 400, 300);
    gui::Menu* menu = new gui::Menu(&window);
    std::string chosen;
    menu->add_item("切り取り", [&](gui::Object*) { chosen = "cut"; });
    gui::MenuItem* copy = menu->add_item("コピー", [&](gui::Object* sender) {
        chosen = static_cast<gui::MenuItem*>(sender)->text;
    });
    copy->shortcut_text = "Ctrl+C";
    menu->add_separator();
    gui::MenuItem* disabled = menu->add_item("無効", [&](gui::Object*) { chosen = "disabled"; });
    disabled->enabled = false;
    gui::Menu* sort = menu->add_submenu("並べ替え");
    sort->add_item("名前順", [&](gui::Object*) { chosen = "name"; });
    sort->add_item("日付順", [&](gui::Object*) { chosen = "date"; });

    menu->popup(area, gui::Point(50, 40));
    CHECK(menu->is_open());
    dispatch_key(window, SDLK_DOWN);
    dispatch_key(window, SDLK_DOWN);
    dispatch_key(window, SDLK_RETURN);  // 2 つ目(コピー)を選ぶ
    CHECK(chosen == "コピー");
    CHECK(!menu->is_open());  // 選ぶとすべて閉じる

    menu->popup(area, gui::Point(50, 40));
    dispatch_key(window, SDLK_UP);    // 何も強調していなければ、↑ は最後の項目(並べ替え)
    dispatch_key(window, SDLK_UP);    // 無効な項目・区切りを飛ばしてコピー
    dispatch_key(window, SDLK_DOWN);  // 区切り・無効な項目を飛ばして並べ替えへ戻る
    dispatch_key(window, SDLK_RIGHT);  // サブメニューを開き、最初の項目を強調する
    CHECK(sort->is_open());
    dispatch_key(window, SDLK_DOWN);
    dispatch_key(window, SDLK_RETURN);
    CHECK(chosen == "date");
    CHECK(!sort->is_open() && !menu->is_open());

    menu->popup(area, gui::Point(50, 40));
    dispatch_key(window, SDLK_UP);
    dispatch_key(window, SDLK_RIGHT);
    dispatch_key(window, SDLK_LEFT);  // サブメニューだけ閉じる
    CHECK(!sort->is_open() && menu->is_open());
    dispatch_key(window, SDLK_ESCAPE);
    CHECK(!menu->is_open());

    menu->popup(area, gui::Point(50, 40));
    dispatch_click(window, gui::Point(350, 250));  // 外側を押すと閉じる
    CHECK(!menu->is_open());
    CHECK(gui::Application::instance()->impl()->open_menus.empty());
}

TEST(tooltip_property)
{
    gui::Window window("t", 300, 200);
    gui::Button* button = new gui::Button(&window, "b");
    button->tooltip = "説明";
    CHECK(button->tooltip == "説明");
}

TEST(window_close_hides)
{
    gui::Window window("t", 100, 100);
    window.visible = true;
    window.close();  // close() は on_close を呼ばずに閉じる
    CHECK(!window.visible);
}

TEST(save_png)
{
    gui::Window window("t", 120, 60);
    new gui::Label(&window, "テスト");
    window.visible = true;
    CHECK(window.save_png("gui_test_save.png"));
    std::FILE* f = std::fopen("gui_test_save.png", "rb");
    CHECK(f != nullptr);
    if (f)
    {
        std::fseek(f, 0, SEEK_END);
        CHECK(std::ftell(f) > 100);
        std::fclose(f);
        std::remove("gui_test_save.png");
    }
}

// 描画の結果のピクセル(ARGB8888)と比べるための値。
std::uint32_t packed(const gui::Color& c)
{
    return (std::uint32_t(c.a) << 24) | (std::uint32_t(c.r) << 16) | (std::uint32_t(c.g) << 8) | c.b;
}

TEST(theme_color_resolves_role_or_fixed)
{
    gui::Application* app = gui::Application::instance();
    app->color_scheme = gui::ColorScheme::LIGHT;
    const gui::ThemeColor role(gui::ColorRole::ACCENT);
    const gui::ThemeColor fixed(gui::Color::rgb(0x123456));
    CHECK(role.is_role() && role.role() == gui::ColorRole::ACCENT);
    CHECK(!fixed.is_role() && fixed.color() == gui::Color::rgb(0x123456));
    CHECK(role.resolve() == gui::Theme::light()[gui::ColorRole::ACCENT]);
    CHECK(role != fixed);
    CHECK(gui::ThemeColor(gui::ColorRole::TEXT) == gui::ThemeColor());
    app->color_scheme = gui::ColorScheme::DARK;
    CHECK(role.resolve() == gui::Theme::dark()[gui::ColorRole::ACCENT]);
    CHECK(fixed.resolve() == gui::Color::rgb(0x123456));  // 固定の色はテーマで変わらない
    app->color_scheme = gui::ColorScheme::SYSTEM;
}

TEST(theme_switch_notifies_and_repaints)
{
    gui::Application* app = gui::Application::instance();
    app->color_scheme = gui::ColorScheme::LIGHT;
    int changes = 0;
    int dark_changes = 0;
    app->on_theme_change = [&](gui::Object* sender)
    {
        CHECK(sender == app);
        ++changes;
    };
    gui::BoolVariable* dark = new gui::BoolVariable(nullptr, false);
    gui::bind(dark->value, app->dark);
    dark->on_change = [&](gui::Object*) { ++dark_changes; };

    gui::Window window("t", 120, 60);
    gui::Label* label = new gui::Label(&window, "テスト");
    window.visible = true;
    window.impl()->render(false);
    CHECK(window.background.get() == gui::ThemeColor(gui::ColorRole::WINDOW));
    CHECK(window.impl()->buffer[0] == packed(gui::Theme::light()[gui::ColorRole::WINDOW]));

    app->color_scheme = gui::ColorScheme::DARK;
    CHECK(app->dark);
    CHECK(app->theme.get() == gui::Theme::dark());
    CHECK(changes == 1);
    CHECK(dark_changes == 1 && dark->value);
    CHECK(!window.impl()->dirty.empty());  // 描き直しを依頼済み
    window.impl()->render(false);
    CHECK(window.impl()->buffer[0] == packed(gui::Theme::dark()[gui::ColorRole::WINDOW]));
    CHECK(label->color.get().resolve() == gui::Theme::dark()[gui::ColorRole::TEXT]);

    app->color_scheme = gui::ColorScheme::DARK;  // 同じ値: 知らせない
    CHECK(changes == 1);

    // 使っていない方のテーマを変えても、使うテーマは変わらない。使っている方を変えると変わる。
    gui::Theme custom = gui::Theme::light();
    custom[gui::ColorRole::ACCENT] = gui::Color::rgb(0x059669);
    app->light_theme = custom;
    CHECK(changes == 1);
    app->color_scheme = gui::ColorScheme::LIGHT;
    CHECK(changes == 2 && !app->dark);
    CHECK(gui::ThemeColor(gui::ColorRole::ACCENT).resolve() == gui::Color::rgb(0x059669));

    app->on_theme_change = gui::NotifyHandler();
    delete dark;
    app->light_theme = gui::Theme::light();
    app->color_scheme = gui::ColorScheme::SYSTEM;
}

}  // namespace

int main(int argc, char** argv)
{
    gui::Application app(argc, argv);
    for (std::size_t i = 0; i < registry().size(); ++i)
    {
        const int before = g_failures;
        registry()[i].fn();
        std::printf("%s %s\n", g_failures == before ? "ok  " : "FAIL", registry()[i].name);
    }
    std::printf("%d checks, %d failures\n", g_checks, g_failures);
    return g_failures == 0 ? 0 : 1;
}
