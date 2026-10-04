// gui ライブラリのテスト。SDL の offscreen のビデオドライバで、画面なしで動かす(CMake が環境変数を設定する)。
#include <gui/gui.hpp>

#include "../src/core/binding_registry.hpp"
#include "../src/core/application_impl.hpp"
#include "../src/core/window_impl.hpp"

#include <algorithm>
#include <cmath>
#include <atomic>
#include <chrono>
#include <thread>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <functional>
#include <limits>
#include <stdexcept>
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
void dispatch_key(gui::Window& window, SDL_Keycode key, bool down = true, SDL_Keymod mod = SDL_KMOD_NONE)
{
    SDL_Event e;
    SDL_zero(e);
    e.type = down ? SDL_EVENT_KEY_DOWN : SDL_EVENT_KEY_UP;
    e.key.windowID = SDL_GetWindowID(window.impl()->window);
    e.key.key = key;
    e.key.mod = mod;
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
    gui::Action* copy = menu->add_item("コピー", [&](gui::Object* sender) {
        chosen = static_cast<gui::Action*>(sender)->text;
    });
    copy->shortcut = "Ctrl+C";
    menu->add_separator();
    gui::Action* disabled = menu->add_item("無効", [&](gui::Object*) { chosen = "disabled"; });
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

void dispatch_text(gui::Window& window, const char* text)
{
    SDL_Event e;
    SDL_zero(e);
    e.type = SDL_EVENT_TEXT_INPUT;
    e.text.windowID = SDL_GetWindowID(window.impl()->window);
    e.text.text = text;
    gui::Application::instance()->impl()->dispatch(e);
}

TEST(range_base_clamps_and_moves_bounds)
{
    gui::Window window("t", 300, 100);
    gui::Slider* slider = new gui::Slider(&window);
    int changes = 0;
    slider->on_change = [&](gui::Object*) { ++changes; };
    slider->value = 150;  // 範囲(既定 0〜100)に丸める
    CHECK(slider->value == 100.0);
    slider->value = -1;
    CHECK(slider->value == 0.0);
    CHECK(changes == 2);
    slider->value = 30;
    slider->minimum = 50;  // value も丸める
    CHECK(slider->value == 50.0);
    slider->maximum = 20;  // minimum より小さくすると minimum も動く
    CHECK(slider->minimum == 20.0 && slider->maximum == 20.0 && slider->value == 20.0);
    slider->minimum = 0;
    slider->maximum = 100;
    CHECK(slider->value == 20.0);
    slider->value = std::numeric_limits<double>::quiet_NaN();  // 数でない値は無視する
    CHECK(slider->value == 20.0);

    gui::DoubleVariable* level = new gui::DoubleVariable(&window, 0);
    gui::bind(slider->value, level->value);
    level->value = 75;
    CHECK(slider->value == 75.0);
}

TEST(slider_keys_and_mouse)
{
    gui::Window window("t", 300, 100);
    window.visible = true;
    gui::Slider* slider = new gui::Slider(&window);
    slider->bounds = gui::Rect(10, 10, 216, 24);  // つまみの中心が動ける長さは 200
    window.apply_layout();
    slider->value = 20;
    slider->focused = true;
    dispatch_key(window, SDLK_RIGHT);
    CHECK(slider->value == 21.0);
    dispatch_key(window, SDLK_PAGEUP);
    CHECK(slider->value == 31.0);
    dispatch_key(window, SDLK_DOWN);
    CHECK(slider->value == 30.0);
    dispatch_key(window, SDLK_END);
    CHECK(slider->value == 100.0);
    dispatch_key(window, SDLK_HOME);
    CHECK(slider->value == 0.0);

    // 溝を押すと、その位置の値になる(single_step に合わせる)。
    dispatch_click(window, gui::Point(10 + 8 + 100.4f, 22));
    CHECK(slider->value == 50.0);
    // 代入した値は合わせないが、操作した値は single_step に合わせる。
    slider->single_step = 10;
    slider->value = 33;
    CHECK(slider->value == 33.0);
    dispatch_key(window, SDLK_RIGHT);
    CHECK(slider->value == 40.0);

    // 縦は下が minimum。
    slider->orientation = gui::Orientation::VERTICAL;
    slider->bounds = gui::Rect(10, 10, 24, 216);
    window.apply_layout();
    slider->single_step = 1;
    dispatch_click(window, gui::Point(22, 10 + 8 + 50));  // 上から 1/4 の位置
    CHECK(slider->value == 75.0);
}

TEST(spin_box_commit_and_steps)
{
    gui::Window window("t", 300, 100);
    window.visible = true;
    gui::SpinBox* spin = new gui::SpinBox(&window);
    spin->bounds = gui::Rect(10, 10, 120, 32);
    spin->maximum = 10;
    spin->decimals = 1;
    spin->value = 2.5;
    window.apply_layout();
    CHECK(spin->text == "2.5");
    int changes = 0;
    spin->on_change = [&](gui::Object*) { ++changes; };

    // 入力欄を押し、打ち込んで Enter で決める(桁数に丸める)。
    dispatch_click(window, gui::Point(30, 26));
    dispatch_key(window, SDLK_END);
    for (int i = 0; i < 3; ++i)
    {
        dispatch_key(window, SDLK_BACKSPACE);
    }
    dispatch_text(window, "7.25");
    CHECK(spin->value == 2.5);  // 決めるまでは変わらない
    dispatch_key(window, SDLK_RETURN);
    CHECK(near(static_cast<float>(spin->value.get()), 7.3f));
    CHECK(spin->text == "7.3");
    CHECK(changes == 1);

    // 数値として読めなければ元に戻す。範囲の外は丸める。
    dispatch_key(window, SDLK_END);
    for (int i = 0; i < 3; ++i)
    {
        dispatch_key(window, SDLK_BACKSPACE);
    }
    dispatch_text(window, "abc");
    dispatch_key(window, SDLK_RETURN);
    CHECK(spin->text == "7.3");
    dispatch_key(window, SDLK_END);
    for (int i = 0; i < 3; ++i)
    {
        dispatch_key(window, SDLK_BACKSPACE);
    }
    dispatch_text(window, "99");
    dispatch_key(window, SDLK_RETURN);
    CHECK(spin->value == 10.0 && spin->text == "10.0");

    // ↑↓ は single_step ずつ(範囲の中で)。
    dispatch_key(window, SDLK_UP);
    CHECK(spin->value == 10.0);
    dispatch_key(window, SDLK_DOWN);
    CHECK(spin->value == 9.0 && spin->text == "9.0");
    // 上のボタンを押す。
    dispatch_click(window, gui::Point(10 + 120 - 10, 14));
    CHECK(spin->value == 10.0);
    dispatch_click(window, gui::Point(10 + 120 - 10, 38));
    CHECK(spin->value == 9.0);

    spin->decimals = 0;
    CHECK(spin->text == "9");
}

TEST(progress_bar_range)
{
    gui::Window window("t", 300, 100);
    gui::ProgressBar* progress = new gui::ProgressBar(&window);
    progress->value = 150;
    CHECK(progress->value == 100.0);
    progress->value = 40;
    const float plain = progress->preferred_size.get().width;
    progress->text_visible = true;
    CHECK(progress->preferred_size.get().width > plain);  // 割合の文字の分だけ広い
    progress->minimum = 200;
    CHECK(progress->maximum == 200.0 && progress->value == 200.0);
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

TEST(tab_widget_add_insert_remove)
{
    gui::Window window("t", 300, 200);
    gui::TabWidget* tabs = new gui::TabWidget(&window);
    CHECK(tabs->current_index == -1 && tabs->current_tab == nullptr);
    int changes = 0;
    tabs->on_change = [&](gui::Object* sender)
    {
        CHECK(sender == tabs);
        ++changes;
    };

    gui::Widget* a = new gui::Widget();
    gui::Widget* b = new gui::Widget();
    gui::Widget* c = new gui::Widget();
    gui::Tab* ta = tabs->add_tab(a, "A");
    CHECK(ta && ta->page == a && ta->title.get() == "A" && a->parent == tabs);
    CHECK(tabs->current_index == 0 && tabs->current_tab == ta && a->visible && changes == 1);
    gui::Tab* tb = tabs->add_tab(b, "B");
    CHECK(tabs->current_index == 0 && !b->visible && changes == 1);  // 2 つ目以降は選ばない
    CHECK(tabs->add_tab(b, "B2") == tb && tabs->tabs.get().size() == 2);  // 同じページは加えない
    gui::Tab* tc = tabs->insert_tab(0, c, "C");
    CHECK(tabs->tabs.get().size() == 3 && tabs->tabs.get()[0] == tc);
    CHECK(tabs->current_tab == ta && tabs->current_index == 1 && changes == 2);  // 選んでいるタブは同じで、番号がずれる

    tabs->current_index = 2;
    CHECK(tabs->current_tab == tb && b->visible && !a->visible && !c->visible && changes == 3);
    tabs->current_index = 5;  // 範囲の外は無視する
    tabs->current_index = -1;
    CHECK(tabs->current_index == 2 && changes == 3);

    // 外したページは親が無くなり、見える状態で返る。選んでいたタブなら隣を選ぶ。
    CHECK(tabs->remove_tab(tb) == b);
    CHECK(b->parent == nullptr && b->visible && tabs->tabs.get().size() == 2);
    CHECK(tabs->current_tab == ta && a->visible && changes == 4);
    delete b;

    // ページを破棄する・別の親へ移すと、そのタブも消える。
    delete a;
    CHECK(tabs->tabs.get().size() == 1 && tabs->current_tab == tc && c->visible && changes == 5);
    c->parent = &window;
    CHECK(tabs->tabs.get().empty() && tabs->current_index == -1 && tabs->current_tab == nullptr && changes == 6);
    CHECK(c->visible);

    // 別の TabWidget へ加えると、元のタブは消える。
    gui::TabWidget* other = new gui::TabWidget(&window);
    tabs->add_tab(c, "C");
    other->add_tab(c, "C");
    CHECK(tabs->tabs.get().empty() && other->tabs.get().size() == 1 && c->parent == other);
    CHECK(tabs->insert_tab(0, tabs, "self") == nullptr);
    tabs->on_change = gui::NotifyHandler();
}

TEST(tab_widget_keys_and_clicks)
{
    gui::Window window("t", 400, 200);
    window.visible = true;
    gui::TabWidget* tabs = new gui::TabWidget(&window);
    tabs->bounds = gui::Rect(0, 0, 400, 200);
    gui::LineEdit* edit = new gui::LineEdit();
    tabs->add_tab(edit, "1");
    gui::Tab* second = tabs->add_tab(new gui::Widget(), "2");
    tabs->add_tab(new gui::Widget(), "3");
    second->enabled = false;
    window.apply_layout();
    CHECK(edit->bounds.get().y >= 20 && edit->bounds.get().width == 400);  // 見出しの下に並べる

    // 見出しは最小の幅(48dp)で左から並ぶ。押すとフォーカスが移る。
    dispatch_click(window, gui::Point(120, 10));
    CHECK(tabs->current_index == 2 && tabs->focused);
    dispatch_click(window, gui::Point(72, 10));  // 無効なタブは選べない
    CHECK(tabs->current_index == 2);

    dispatch_key(window, SDLK_LEFT);  // 無効なタブを飛ばす
    CHECK(tabs->current_index == 0);
    dispatch_key(window, SDLK_LEFT);  // ←→ は巡回しない
    CHECK(tabs->current_index == 0);
    dispatch_key(window, SDLK_END);
    CHECK(tabs->current_index == 2);
    dispatch_key(window, SDLK_HOME);
    CHECK(tabs->current_index == 0);

    // Ctrl+Tab・Ctrl+Page Up/Down は巡回する。中の部品にフォーカスがあっても届く。
    edit->focused = true;
    dispatch_key(window, SDLK_TAB, true, SDL_KMOD_LCTRL);
    CHECK(tabs->current_index == 2);
    dispatch_key(window, SDLK_TAB, true, SDL_KMOD_LCTRL);
    CHECK(tabs->current_index == 0);
    dispatch_key(window, SDLK_TAB, true, SDL_Keymod(SDL_KMOD_LCTRL | SDL_KMOD_LSHIFT));
    CHECK(tabs->current_index == 2);
    dispatch_key(window, SDLK_PAGEUP, true, SDL_KMOD_LCTRL);
    CHECK(tabs->current_index == 0);
    dispatch_key(window, SDLK_PAGEDOWN, true, SDL_KMOD_LCTRL);
    CHECK(tabs->current_index == 2);
    // 隠したページの中にあったフォーカスは、TabWidget へ移る。
    CHECK(!edit->focused && tabs->focused);

    // 代入では無効なタブも選べる。
    tabs->current_index = 1;
    CHECK(tabs->current_tab == second);
}

TEST(tab_widget_scrolls_overflowing_titles)
{
    gui::Window window("t", 400, 200);
    window.visible = true;
    gui::TabWidget* tabs = new gui::TabWidget(&window);
    tabs->bounds = gui::Rect(0, 0, 150, 120);
    for (int i = 0; i < 6; ++i)
    {
        tabs->add_tab(new gui::Widget(), std::string(1, char('1' + i)));
    }
    window.apply_layout();
    CHECK(tabs->minimum_hint.get().width >= 48 + 24 * 2);  // 見出し 1 つと ◀▶

    // 見出しは 6 × 48 = 288dp、見せられる幅は 150 - 48 = 102dp。最後を選ぶと、右端が見えるまでスクロールする。
    tabs->current_index = 5;
    dispatch_click(window, gui::Point(50, 10));  // 50 + 186 = 236 → 5 つ目(192〜240)
    CHECK(tabs->current_index == 4);
    dispatch_click(window, gui::Point(114, 10));  // ◀ で 80dp 戻す
    dispatch_click(window, gui::Point(10, 10));   // 10 + 106 = 116 → 3 つ目(96〜144)
    CHECK(tabs->current_index == 2);
    dispatch_click(window, gui::Point(138, 10));  // ▶ では選ぶタブは変わらない
    CHECK(tabs->current_index == 2);
}

// マウスのボタン・移動のイベント(位置は dp)。修飾キーは SDL_SetModState で決める。
void dispatch_mouse(gui::Window& window, SDL_EventType type, const gui::Point& p, int clicks = 1)
{
    const float k = window.impl()->scale / window.impl()->density;
    SDL_Event e;
    SDL_zero(e);
    e.type = type;
    if (type == SDL_EVENT_MOUSE_MOTION)
    {
        e.motion.windowID = SDL_GetWindowID(window.impl()->window);
        e.motion.x = p.x * k;
        e.motion.y = p.y * k;
    }
    else
    {
        e.button.windowID = SDL_GetWindowID(window.impl()->window);
        e.button.button = SDL_BUTTON_LEFT;
        e.button.down = type == SDL_EVENT_MOUSE_BUTTON_DOWN;
        e.button.clicks = static_cast<Uint8>(clicks);
        e.button.x = p.x * k;
        e.button.y = p.y * k;
    }
    gui::Application::instance()->impl()->dispatch(e);
}

void click_with(gui::Window& window, const gui::Point& p, SDL_Keymod mod, int clicks = 1)
{
    SDL_SetModState(mod);
    dispatch_mouse(window, SDL_EVENT_MOUSE_BUTTON_DOWN, p, clicks);
    dispatch_mouse(window, SDL_EVENT_MOUSE_BUTTON_UP, p, clicks);
    SDL_SetModState(SDL_KMOD_NONE);
}

// ItemView の行の高さ(倍率 1)と、行 r の中ほど(枠の 1dp と見出しの高さを足す)。
float view_row_height() { return std::max(std::ceil(gui::TextLayout("Ag", gui::Font(), 1).height()), 16.0f) + 8; }

gui::Point row_point(int r, float x = 100, float header = 0)
{
    const float h = view_row_height();
    return gui::Point(x, 1 + header + r * h + h / 2);
}

TEST(image_from_pixels_and_draw)
{
    std::vector<gui::Color> pixels(4, gui::Color(255, 0, 0, 255));
    pixels[3] = gui::Color(0, 0, 255, 128);
    const gui::Image image = gui::Image::from_pixels(2, 2, pixels);
    CHECK(!image.is_null() && image.width() == 2 && image.height() == 2);
    CHECK(image.pixel(0, 0) == gui::Color(255, 0, 0, 255));
    CHECK(image.pixel(1, 1).a == 128 && image.pixel(1, 1).b >= 254);  // アルファ乗算を戻す
    CHECK(image.pixel(5, 5).a == 0);
    CHECK(gui::Image::from_pixels(3, 3, pixels).is_null());  // 数が合わない
    CHECK(gui::Image::load("no-such-file.png").is_null());

    // 描くと、伸縮して矩形を塗る。
    gui::Window window("t", 40, 40);
    gui::Widget* w = new gui::Widget(&window);
    w->bounds = gui::Rect(0, 0, 40, 40);
    w->on_paint = [=](gui::Object*, gui::Canvas& canvas) { canvas.draw_image(image, gui::Rect(0, 0, 20, 20)); };
    window.visible = true;
    window.impl()->render(false);
    const int stride = window.impl()->pixel_width;
    CHECK(window.impl()->buffer[2 * stride + 2] == 0xffff0000u);
}

TEST(list_view_selection_and_keys)
{
    gui::Window window("t", 300, 300);
    window.visible = true;
    gui::ListModel* list = new gui::ListModel(&window);
    std::vector<gui::Item*> items;
    for (int i = 0; i < 5; ++i)
    {
        items.push_back(list->add_item("項目 " + std::to_string(i)));
    }
    items[2]->enabled = false;
    gui::ListView* view = new gui::ListView(&window);
    view->bounds = gui::Rect(0, 0, 200, 250);
    view->model = list;
    window.apply_layout();
    int changes = 0;
    view->on_selection_change = [&](gui::Object* sender)
    {
        CHECK(sender == view);
        ++changes;
    };

    // SINGLE: 押した行を選び、current も移る。
    dispatch_click(window, row_point(1));
    CHECK(view->current == list->index_of(items[1]) && view->is_selected(list->index_of(items[1])));
    CHECK(view->selection.get().size() == 1 && changes == 1 && view->focused);
    CHECK(view->index_at(row_point(3)) == list->index_of(items[3]));
    dispatch_key(window, SDLK_DOWN);  // 無効な項目は飛ばす
    CHECK(view->current == list->index_of(items[3]) && changes == 2);
    dispatch_key(window, SDLK_HOME);
    CHECK(view->current == list->index_of(items[0]));
    dispatch_key(window, SDLK_END);
    CHECK(view->current == list->index_of(items[4]) && view->selection.get().size() == 1);
    dispatch_click(window, row_point(2));  // 無効な項目は選べない
    CHECK(view->current == list->index_of(items[4]));
    view->current = list->index_of(items[0]);  // SINGLE では代入でも選ぶ
    CHECK(view->is_selected(list->index_of(items[0])) && !view->is_selected(list->index_of(items[4])));

    // MULTI: Ctrl+クリックで加え、Shift+クリックで範囲(無効な項目は除く)、Ctrl+A ですべて。
    view->selection_mode = gui::SelectionMode::MULTI;
    click_with(window, row_point(3), SDL_KMOD_LCTRL);
    CHECK(view->selection.get().size() == 2);
    click_with(window, row_point(4), SDL_KMOD_LSHIFT);  // 起点は 3
    CHECK(view->selection.get().size() == 2 && view->is_selected(list->index_of(items[4])));
    click_with(window, row_point(0), SDL_KMOD_LSHIFT);
    const std::vector<gui::ModelIndex> range = view->selection;
    CHECK(range.size() == 3 && range[0] == list->index_of(items[0]) && range[2] == list->index_of(items[3]));  // 表示の順
    dispatch_key(window, SDLK_A, true, SDL_KMOD_LCTRL);
    CHECK(view->selection.get().size() == 4);
    dispatch_key(window, SDLK_DOWN, true, SDL_KMOD_LSHIFT);  // current は 0 から 1 へ。起点 3 から 1 まで
    CHECK(view->selection.get().size() == 2 && view->current == list->index_of(items[1]));
    click_with(window, row_point(4), SDL_KMOD_NONE);
    CHECK(view->selection.get().size() == 1);

    // 行の無いところを押すと選択を外す。
    dispatch_click(window, gui::Point(100, 240));
    CHECK(view->selection.get().empty());

    // ダブルクリック・Enter で on_activate。
    int activations = 0;
    view->on_activate = [&](gui::Object*) { ++activations; };
    click_with(window, row_point(1), SDL_KMOD_NONE, 2);
    dispatch_key(window, SDLK_RETURN);
    CHECK(activations == 2);
    view->on_selection_change = gui::NotifyHandler();
}

TEST(list_view_follows_model_changes)
{
    gui::Window window("t", 300, 300);
    window.visible = true;
    gui::ListModel* list = new gui::ListModel(&window);
    gui::Item* b = list->add_item("b");
    gui::Item* c = list->add_item("c");
    gui::Item* a = list->add_item("a");
    gui::ListView* view = new gui::ListView(&window);
    view->bounds = gui::Rect(0, 0, 200, 250);
    view->model = list;
    view->selection_mode = gui::SelectionMode::MULTI;
    view->select(list->index_of(c));
    view->current = list->index_of(c);
    int changes = 0;
    view->on_selection_change = [&](gui::Object*) { ++changes; };

    // 前に加えると番号がずれるが、同じ項目を選んだまま。
    list->insert_item(0, "z");
    CHECK(view->is_selected(list->index_of(c)) && view->current == list->index_of(c) && list->index_of(c).row() == 2);
    // 並べ替えても同じ項目。
    list->sort(0, gui::SortOrder::ASCENDING);
    CHECK(list->items.get()[0] == a && list->items.get()[3]->text.get() == "z");
    CHECK(view->is_selected(list->index_of(c)) && view->current == list->index_of(c) && changes == 0);
    // 選んでいる項目を除くと、選択から外れて知らせる。
    list->remove_item(c);
    CHECK(view->selection.get().empty() && !view->current.get().is_valid() && changes == 1);
    // 項目のチェックは、チェックボックスを押すか Space で変わる。
    b->checkable = true;
    window.apply_layout();
    const int row_b = list->index_of(b).row();
    dispatch_click(window, row_point(row_b, 1 + 8 + 8));
    CHECK(b->checked && view->current == list->index_of(b));
    dispatch_key(window, SDLK_SPACE);
    CHECK(!b->checked);
    // モデルを破棄すると、ビューは model を手放す。
    delete list;
    CHECK(view->model == nullptr && view->selection.get().empty());
    view->on_selection_change = gui::NotifyHandler();
}

TEST(list_view_columns_sort_and_resize)
{
    gui::Window window("t", 400, 300);
    window.visible = true;
    gui::ListModel* list = new gui::ListModel(&window);
    const char* const NAMES[] = {"b", "c", "a"};
    const char* const SIZES[] = {"2", "1", "3"};
    for (int i = 0; i < 3; ++i)
    {
        list->add_item(NAMES[i])->column_texts = std::vector<std::string>(1, SIZES[i]);
    }
    gui::ListView* view = new gui::ListView(&window);
    view->bounds = gui::Rect(0, 0, 300, 250);
    gui::Column* name = view->add_column("名前", 100);
    view->add_column("大きさ", 60);
    view->model = list;
    window.apply_layout();
    CHECK(list->text(list->index(0), 1) == "2");
    const float header = view_row_height();

    // 見出しを押すと並べ替える(sorting_enabled のときだけ)。もう一度押すと逆順。
    dispatch_click(window, gui::Point(50, 1 + header / 2));
    CHECK(view->sort_column == -1);
    view->sorting_enabled = true;
    dispatch_click(window, gui::Point(130, 1 + header / 2));
    CHECK(view->sort_column == 1 && view->sort_order == gui::SortOrder::ASCENDING);
    CHECK(list->items.get()[0]->text.get() == "c");
    dispatch_click(window, gui::Point(130, 1 + header / 2));
    CHECK(view->sort_order == gui::SortOrder::DESCENDING && list->items.get()[0]->text.get() == "a");
    view->sort_column = 0;  // 代入でも並べ替える
    CHECK(list->items.get()[0]->text.get() == "c");
    // 見出しの下の行を押す。
    dispatch_click(window, row_point(0, 100, header));
    CHECK(view->current == list->index(0));

    // 境目をドラッグして幅を変える(最小 24)。
    dispatch_mouse(window, SDL_EVENT_MOUSE_BUTTON_DOWN, gui::Point(1 + 100, 1 + header / 2));
    dispatch_mouse(window, SDL_EVENT_MOUSE_MOTION, gui::Point(1 + 150, 1 + header / 2));
    dispatch_mouse(window, SDL_EVENT_MOUSE_BUTTON_UP, gui::Point(1 + 150, 1 + header / 2));
    CHECK(near(name->width, 150));
    name->width = 5;
    CHECK(near(name->width, 24));
    view->remove_column(name);
    CHECK(view->columns.get().size() == 1 && view->sort_column == -1);
}

TEST(tree_view_expand_and_keys)
{
    gui::Window window("t", 300, 400);
    window.visible = true;
    gui::TreeModel* tree = new gui::TreeModel(&window);
    gui::TreeItem* docs = tree->add_item("文書");
    gui::TreeItem* work = docs->add_child("仕事");
    gui::TreeItem* report = work->add_child("報告書");
    gui::TreeItem* readme = docs->add_child("README");
    gui::TreeItem* pics = tree->add_item("画像");
    CHECK(report->parent_item == work && docs->children.get().size() == 2 && tree->items.get().size() == 2);
    CHECK(tree->parent_of(tree->index_of(report)) == tree->index_of(work));
    CHECK(tree->row_count(tree->index_of(docs)) == 2 && tree->index(1, tree->index_of(docs)) == tree->index_of(readme));

    gui::TreeView* view = new gui::TreeView(&window);
    view->bounds = gui::Rect(0, 0, 250, 350);
    view->model = tree;
    window.apply_layout();
    // 最初は一番上だけ。
    CHECK(view->index_at(row_point(1)) == tree->index_of(pics));
    CHECK(!view->index_at(row_point(2)).is_valid());

    // 印を押すと開く。
    dispatch_click(window, row_point(0, 1 + 4 + 10));
    CHECK(view->is_expanded(tree->index_of(docs)));
    CHECK(view->index_at(row_point(1)) == tree->index_of(work) && view->index_at(row_point(3)) == tree->index_of(pics));
    CHECK(!view->current.get().is_valid());  // 印を押しても選ばない

    // → で開く・子へ、← で閉じる・親へ。
    dispatch_click(window, row_point(1));
    CHECK(view->current == tree->index_of(work));
    dispatch_key(window, SDLK_RIGHT);
    CHECK(view->is_expanded(tree->index_of(work)) && view->current == tree->index_of(work));
    dispatch_key(window, SDLK_RIGHT);
    CHECK(view->current == tree->index_of(report));
    dispatch_key(window, SDLK_LEFT);
    CHECK(view->current == tree->index_of(work));
    dispatch_key(window, SDLK_LEFT);
    CHECK(!view->is_expanded(tree->index_of(work)));
    dispatch_key(window, SDLK_LEFT);
    CHECK(view->current == tree->index_of(docs));

    // 閉じた中に current があれば、閉じた項目へ移す。
    view->current = tree->index_of(readme);
    view->collapse(tree->index_of(docs));
    CHECK(view->current == tree->index_of(docs) && view->is_selected(tree->index_of(docs)));

    // scroll_to は祖先を開く。
    view->scroll_to(tree->index_of(report));
    CHECK(view->is_expanded(tree->index_of(docs)) && view->is_expanded(tree->index_of(work)));
    CHECK(view->index_at(row_point(2)) == tree->index_of(report));

    // 項目を除くと、その子孫の選択も外れる。
    view->current = tree->index_of(report);
    tree->remove_item(work);
    CHECK(view->selection.get().empty() && docs->children.get().size() == 1);
    CHECK(view->index_at(row_point(1)) == tree->index_of(readme));
    // 子を加えると表示に現れる(開いていれば)。
    gui::TreeItem* added = docs->insert_child(0, "新しい");
    CHECK(view->index_at(row_point(1)) == tree->index_of(added));

    view->expand_all();
    view->collapse_all();
    CHECK(view->index_at(row_point(1)) == tree->index_of(pics));
    tree->clear();
    CHECK(!view->index_at(row_point(0)).is_valid());
}


// 項目のオブジェクトを作らないモデル: 100 万行でも、描くときに問い合わせるのは見えている行だけ。
struct CountingModel : gui::ItemModel
{
    mutable int queries;
    CountingModel() : queries(0) {}
    int row_count(const gui::ModelIndex& parent) const override { return parent.is_valid() ? 0 : 1000000; }
    std::string text(const gui::ModelIndex& index, int) const override
    {
        ++queries;
        return std::to_string(index.row());
    }
};

TEST(item_view_queries_only_visible_rows)
{
    gui::Window window("t", 300, 300);
    window.visible = true;
    CountingModel* model = new CountingModel();
    gui::ListView* view = new gui::ListView(&window);
    view->bounds = gui::Rect(0, 0, 200, 250);
    view->model = model;
    window.impl()->render(false);
    CHECK(model->queries > 0 && model->queries < 20);
    dispatch_click(window, row_point(0));
    dispatch_key(window, SDLK_END);
    CHECK(view->current.get().row() == 999999);
    CHECK(view->index_at(row_point(0)).row() > 999980);  // 最後までスクロールした
    delete model;
    CHECK(view->model == nullptr);
}

TEST(shortcut_parse_and_match)
{
    const gui::Shortcut save("Ctrl+S");
    CHECK(save.key == gui::Key::S && save.modifiers == gui::MODIFIER_CONTROL);
    CHECK(gui::Shortcut("ctrl+shift+z").to_string() == "Ctrl+Shift+Z");
    CHECK(gui::Shortcut("F5").key == gui::Key::F5 && gui::Shortcut("Delete").key == gui::Key::DEL);
    CHECK(gui::Shortcut("Alt+PageDown").to_string() == "Alt+PageDown");
    CHECK(gui::Shortcut("Ctrl+").is_empty() && gui::Shortcut("Hyper+A").is_empty() && gui::Shortcut("F13").is_empty());
    CHECK(gui::Shortcut().to_string().empty());
    gui::KeyEvent e(gui::Key::S, gui::MODIFIER_CONTROL, false, 0);
    CHECK(save.matches(e));
    e.modifiers = gui::MODIFIER_CONTROL | gui::MODIFIER_SHIFT;
    CHECK(!save.matches(e));  // 修飾キーは過不足なく
}

TEST(action_trigger_and_menu_entries)
{
    gui::Window window("t", 300, 200);
    gui::Action* action = new gui::Action(&window, "太字(&B)");
    int triggers = 0;
    action->on_trigger = [&](gui::Object* sender)
    {
        CHECK(sender == action);
        ++triggers;
    };
    action->checkable = true;
    action->trigger();
    CHECK(triggers == 1 && action->checked);
    action->enabled = false;
    action->trigger();
    CHECK(triggers == 1 && action->checked);

    // 1 つの Action を複数のメニューに入れられる。Action を破棄すると項目も消える。
    gui::Menu* a = new gui::Menu(&window);
    gui::Menu* b = new gui::Menu(&window);
    a->add_action(action);
    b->add_action(action);
    gui::Action* owned = a->add_item("所有");
    CHECK(a->actions.get().size() == 2 && b->actions.get().size() == 1);
    delete action;
    CHECK(a->actions.get().size() == 1 && b->actions.get().empty());
    a->remove_action(owned);  // メニューが作った Action は破棄される
    CHECK(a->actions.get().empty());
}

TEST(menu_bar_layout_and_shortcuts)
{
    gui::Window window("t", 400, 300);
    gui::BoxLayout* box = new gui::BoxLayout(&window);
    box->margins = gui::Margins(0);
    gui::LineEdit* edit = new gui::LineEdit(&window);
    gui::ListView* list = new gui::ListView(&window);
    box->add(edit);
    box->add(list, 1);
    gui::MenuBar* bar = new gui::MenuBar();
    gui::Menu* file = bar->add_menu("ファイル(&F)");
    gui::Menu* edit_menu = bar->add_menu("編集(&E)");
    std::string done;
    file->add_item("保存(&S)", [&](gui::Object*) { done += "save "; })->shortcut = "Ctrl+S";
    gui::Menu* recent = file->add_submenu("最近(&R)");
    recent->add_item("一つ目(&1)", [&](gui::Object*) { done += "recent "; })->shortcut = "Ctrl+1";
    gui::Action* select_all = edit_menu->add_item("すべて選択(&A)", [&](gui::Object*) { done += "all "; });
    select_all->shortcut = "Ctrl+A";
    window.menu_bar = bar;
    window.visible = true;
    window.apply_layout();

    // メニューバーは上端に、レイアウトはその下に置く。
    CHECK(bar->parent == &window && bar->y == 0 && near(bar->width, 400));
    CHECK(near(edit->y, bar->height) && window.minimum_hint.get().height > bar->height);

    // メニューバーのメニューのショートカットは、ウィンドウ全体で効く(サブメニューも)。
    dispatch_key(window, SDLK_S, true, SDL_KMOD_LCTRL);
    dispatch_key(window, SDLK_1, true, SDL_KMOD_LCTRL);
    CHECK(done == "save recent ");
    // フォーカスのあるウィジェットが受け取ったキーは、ショートカットより優先する(入力欄の Ctrl+A)。
    edit->focused = true;
    dispatch_key(window, SDLK_A, true, SDL_KMOD_LCTRL);
    CHECK(done == "save recent ");
    // ウィジェットに加えた Action は、フォーカスがその中にあるときだけ効き、ウィンドウのものより優先する。
    gui::Action* local_save = new gui::Action(&window);
    local_save->shortcut = "Ctrl+S";
    local_save->on_trigger = [&](gui::Object*) { done += "local "; };
    list->add_action(local_save);
    dispatch_key(window, SDLK_S, true, SDL_KMOD_LCTRL);  // 入力欄にフォーカス: ウィンドウのもの
    list->focused = true;
    dispatch_key(window, SDLK_S, true, SDL_KMOD_LCTRL);
    CHECK(done == "save recent save local ");
    // 無効な Action は実行しない(次の候補へ進む)。
    local_save->enabled = false;
    dispatch_key(window, SDLK_S, true, SDL_KMOD_LCTRL);
    CHECK(done == "save recent save local save ");
    delete local_save;
    CHECK(list->actions.get().empty());

    // メニューバーを破棄すると、レイアウトの領域は上端からになる。
    delete bar;
    CHECK(window.menu_bar == nullptr);
    window.apply_layout();
    CHECK(near(edit->y, 0));
}

TEST(menu_bar_keyboard)
{
    gui::Window window("t", 400, 300);
    gui::MenuBar* bar = new gui::MenuBar();
    gui::Menu* file = bar->add_menu("ファイル(&F)");
    gui::Menu* edit = bar->add_menu("編集(&E)");
    std::string done;
    file->add_item("開く(&O)", [&](gui::Object*) { done += "open "; });
    file->add_item("保存(&S)", [&](gui::Object*) { done += "save "; });
    edit->add_item("元に戻す(&U)", [&](gui::Object*) { done += "undo "; });
    window.menu_bar = bar;
    window.visible = true;
    window.apply_layout();

    // Alt+ニーモニックで開き、項目のニーモニックで選ぶ。
    dispatch_key(window, SDLK_F, true, SDL_KMOD_LALT);
    CHECK(file->is_open());
    dispatch_key(window, SDLK_S);
    CHECK(done == "save " && !file->is_open());

    // F10 でメニューバーへ移り、→ で次へ、↓ で開く。
    dispatch_key(window, SDLK_F10);
    dispatch_key(window, SDLK_RIGHT);
    dispatch_key(window, SDLK_DOWN);
    CHECK(edit->is_open());
    // 開いたメニューの ← は隣のメニューへ。Enter で最初の項目を選ぶ。
    dispatch_key(window, SDLK_LEFT);
    CHECK(file->is_open() && !edit->is_open());
    dispatch_key(window, SDLK_RETURN);
    CHECK(done == "save open ");

    // Esc はメニューバーへ戻り、もう一度 Esc でやめる。
    dispatch_key(window, SDLK_E, true, SDL_KMOD_LALT);
    CHECK(edit->is_open());
    dispatch_key(window, SDLK_ESCAPE);
    CHECK(!edit->is_open());
    dispatch_key(window, SDLK_DOWN);  // まだメニューバーにいる
    CHECK(edit->is_open());
    dispatch_key(window, SDLK_ESCAPE);
    dispatch_key(window, SDLK_ESCAPE);
    dispatch_key(window, SDLK_DOWN);  // メニューバーから出た
    CHECK(!edit->is_open() && !file->is_open());

    // Alt を押して離すとメニューバーへ移る。
    dispatch_key(window, SDLK_LALT, true, SDL_KMOD_LALT);
    dispatch_key(window, SDLK_LALT, false);
    dispatch_key(window, SDLK_RETURN);
    CHECK(file->is_open());
    file->close();
    // Alt を押したまま他のキーを押したときは移らない。
    dispatch_key(window, SDLK_LALT, true, SDL_KMOD_LALT);
    dispatch_key(window, SDLK_X, true, SDL_KMOD_LALT);
    dispatch_key(window, SDLK_LALT, false);
    dispatch_key(window, SDLK_RETURN);
    CHECK(!file->is_open());
}

TEST(delete_later_and_post)
{
    gui::Window window("t", 100, 100);
    gui::Widget* w = new gui::Widget(&window);
    int calls = 0;
    w->on_resize = [&](gui::Object* sender)
    {
        ++calls;
        sender->delete_later();
        sender->delete_later();  // 2 度頼んでも 1 度だけ破棄する
    };
    w->bounds = gui::Rect(0, 0, 10, 10);
    CHECK(calls == 1 && window.children.get().size() == 1);  // まだ残っている
    // post した関数は、イベントの処理の中で呼ばれる。その処理の後に、頼まれたオブジェクトを破棄する。
    bool posted = false;
    gui::detail::ApplicationImpl::post([&]() { posted = true; });
    // SDL3 は 1 回りの終わりの印をキューに入れるので、前に残った印で止まらないよう 2 回り読む。
    SDL_Event e;
    for (int round = 0; round < 2 && !posted; ++round)
    {
        while (SDL_PollEvent(&e))
        {
            gui::Application::instance()->impl()->dispatch(e);
        }
    }
    CHECK(posted);
    CHECK(window.children.get().empty());
    // 頼んだ後に破棄されたら、何もしない。
    gui::Widget* x = new gui::Widget(&window);
    x->delete_later();
    delete x;
    gui::Application::instance()->impl()->flush_doomed();
    CHECK(gui::Application::instance()->impl()->doomed.empty());
}

bool accept_button_first()
{
#ifdef _WIN32
    return true;
#else
    const char* desktop = std::getenv("XDG_CURRENT_DESKTOP");
    return desktop && std::strstr(desktop, "KDE") != nullptr;
#endif
}

TEST(dialog_modal_buttons_and_keys)
{
    gui::Window window("t", 400, 300);
    gui::Button* behind = new gui::Button(&window, "後ろ");
    behind->bounds = gui::Rect(10, 10, 100, 30);
    int behind_clicks = 0;
    behind->on_click = [&](gui::Object*) { ++behind_clicks; };
    window.visible = true;

    gui::Dialog* dialog = new gui::Dialog(&window, "名前");
    gui::BoxLayout* box = new gui::BoxLayout(dialog);
    gui::LineEdit* field = new gui::LineEdit(dialog);
    box->add(field);
    gui::Button* ok = dialog->add_button(gui::DialogButton::OK);
    gui::Button* cancel = dialog->add_button(gui::DialogButton::CANCEL);
    CHECK(ok->style == gui::ButtonStyle::PRIMARY && cancel->style == gui::ButtonStyle::SECONDARY);
    std::vector<gui::DialogButton> results;
    dialog->on_finish = [&](gui::Object* sender)
    {
        CHECK(sender == dialog);
        results.push_back(dialog->result);
    };
    dialog->open();
    CHECK(dialog->visible && dialog->owner == &window);
    CHECK(dialog->width >= 320 && dialog->result == gui::DialogButton::NONE);
    // ボタンは下端に、OS の慣習の順に並ぶ。中身はその上。
    CHECK((ok->x < cancel->x) == accept_button_first());
    CHECK(near(ok->y, cancel->y) && field->y.get() + field->height.get() <= ok->y);

    // 開いている間は、親への入力を捨てる。
    dispatch_click(window, gui::Point(50, 25));
    dispatch_key(window, SDLK_SPACE);
    CHECK(behind_clicks == 0);

    // 入力欄で Enter: 入力欄は受け取らず(on_submit が無い)、決定のボタンで閉じる。
    field->focused = true;
    dispatch_key(*dialog, SDLK_RETURN);
    CHECK(results.size() == 1 && results[0] == gui::DialogButton::OK && !dialog->visible);
    CHECK(dialog->result == gui::DialogButton::OK);
    dispatch_click(window, gui::Point(50, 25));
    CHECK(behind_clicks == 1);  // 閉じたら親へ届く

    // Esc・close は取り消しのボタンの結果。
    dialog->open();
    dispatch_key(*dialog, SDLK_ESCAPE);
    dialog->open();
    dialog->close();
    CHECK(results.size() == 3 && results[1] == gui::DialogButton::CANCEL && results[2] == gui::DialogButton::CANCEL);
    // ボタンを押す。
    dialog->open();
    dispatch_click(*dialog, gui::Point(ok->x + 10, ok->y + 10));
    CHECK(results.size() == 4 && results[3] == gui::DialogButton::OK);
    dialog->on_finish = gui::NotifyHandler();
}

TEST(message_dialog_show_finishes_and_deletes)
{
    gui::Window window("t", 400, 300);
    window.visible = true;
    const std::size_t windows_before = gui::Application::instance()->impl()->windows.size();
    gui::DialogButton answer = gui::DialogButton::NONE;
    gui::MessageDialog* d = gui::MessageDialog::show(
        &window, "確認", "保存しますか？ 長い文はダイアログの幅で折り返す。長い文はダイアログの幅で折り返す。",
        std::vector<gui::DialogButton>{gui::DialogButton::SAVE, gui::DialogButton::DISCARD, gui::DialogButton::CANCEL},
        [&](gui::Object* sender) { answer = static_cast<gui::Dialog*>(sender)->result; }, gui::MessageType::QUESTION);
    CHECK(d->visible && d->text.get().find("保存") == 0 && d->type == gui::MessageType::QUESTION);
    CHECK(d->width <= 560 && gui::Application::instance()->impl()->windows.size() == windows_before + 1);
    if (const char* dir = std::getenv("GUI_TEST_SHOTS"))
    {
        d->save_png(std::string(dir) + "/message_dialog.png");
    }
    dispatch_key(*d, SDLK_ESCAPE);  // ダイアログは dispatch の終わりに破棄される
    CHECK(answer == gui::DialogButton::CANCEL);
    CHECK(gui::Application::instance()->impl()->windows.size() == windows_before);

    // OK だけのお知らせは、Esc でも閉じる。
    answer = gui::DialogButton::NONE;
    gui::MessageDialog* info = gui::MessageDialog::show(&window, "お知らせ", "終わった",
                                                        std::vector<gui::DialogButton>(),
                                                        [&](gui::Object* sender)
                                                        { answer = static_cast<gui::Dialog*>(sender)->result; });
    dispatch_key(*info, SDLK_ESCAPE);
    CHECK(answer == gui::DialogButton::OK);
}

// 溜まっているイベントをすべて処理する(SDL3 の 1 回りの終わりの印で止まらないよう 2 回り)。
void pump_events()
{
    SDL_Event e;
    for (int round = 0; round < 2; ++round)
    {
        while (SDL_PollEvent(&e))
        {
            gui::Application::instance()->impl()->dispatch(e);
        }
    }
}

TEST(post_from_worker_threads)
{
    gui::Application* app = gui::Application::instance();
    CHECK(gui::is_main_thread());
    gui::Window window("t", 100, 100);  // 表示しているウィンドウが無いと exec はすぐに終わる
    window.visible = true;
    const int THREADS = 4, PER_THREAD = 100;
    std::vector<int> last(THREADS, -1);
    int received = 0;
    bool in_order = true;
    bool on_main = true;
    std::atomic<int> worker_on_main(0);
    std::vector<std::thread> workers;
    for (int t = 0; t < THREADS; ++t)
    {
        workers.push_back(std::thread(
            [&, t]()
            {
                if (gui::is_main_thread())
                {
                    ++worker_on_main;
                }
                // メインスレッドが exec で眠ってから頼む(頼むと起きることを確かめる)。
                std::this_thread::sleep_for(std::chrono::milliseconds(30));
                for (int i = 0; i < PER_THREAD; ++i)
                {
                    gui::post(
                        [&, t, i]()
                        {
                            on_main = on_main && gui::is_main_thread();
                            in_order = in_order && last[t] == i - 1;  // 同じスレッドからは頼んだ順
                            last[t] = i;
                            if (++received == THREADS * PER_THREAD)
                            {
                                app->quit();
                            }
                        });
                }
            }));
    }
    gui::Timer guard;  // 届かなかったときに終えるため
    guard.interval = 5000;
    guard.on_timer = [&](gui::Object*) { app->quit(); };
    guard.enabled = true;
    app->exec();
    for (std::size_t i = 0; i < workers.size(); ++i)
    {
        workers[i].join();
    }
    CHECK(received == THREADS * PER_THREAD);
    CHECK(in_order && on_main && worker_on_main == 0);
    CHECK(!gui::post(std::function<void()>()));  // 空の関数は頼まない
}

TEST(post_with_weak_ref)
{
    gui::Window window("t", 100, 100);
    gui::Label* label = new gui::Label(&window, "前");
    gui::Label* doomed = new gui::Label(&window, "消える");
    const gui::WeakRef<gui::Label> live(label);
    const gui::WeakRef<gui::Label> dead(doomed);
    CHECK(live.get() == label && dead.get() == doomed && gui::WeakRef<gui::Label>().expired());
    int calls = 0;
    std::thread worker(
        [&]()
        {
            gui::post(live, [&](gui::Label* l) { l->text = "後"; ++calls; });
            gui::post(dead, [&](gui::Label*) { ++calls; });
        });
    worker.join();
    delete doomed;  // 頼んだ処理が呼ばれる前に破棄する
    CHECK(dead.expired() && !live.expired());
    pump_events();
    CHECK(calls == 1 && label->text.get() == "後");
    // 同じ Object から作った WeakRef は目印を共有する。
    const gui::WeakRef<gui::Label> again(label);
    delete label;
    CHECK(live.expired() && again.expired());
}

// done() が true になるまでイベントを処理する(最長 timeout_ms)。
template <typename F>
bool run_until(F done, int timeout_ms = 5000)
{
    const Uint64 end = SDL_GetTicks() + static_cast<Uint64>(timeout_ms);
    SDL_Event e;
    while (!done() && SDL_GetTicks() < end)
    {
        if (SDL_WaitEventTimeout(&e, 20))
        {
            gui::Application::instance()->impl()->dispatch(e);
        }
    }
    return done();
}

TEST(task_result_and_progress)
{
    gui::Window window("t", 100, 100);
    gui::Task<long>* task = new gui::Task<long>(&window);
    int progress_calls = 0, finishes = 0;
    double last_progress = -1;
    bool increasing = true;
    task->on_progress = [&](gui::Object* sender)
    {
        CHECK(sender == task && gui::is_main_thread());
        ++progress_calls;
        increasing = increasing && task->progress > last_progress;
        last_progress = task->progress;
    };
    task->on_finish = [&](gui::Object*)
    {
        ++finishes;
        CHECK(!task->running && !task->cancelled && task->error.get().empty());
    };
    const bool started = task->start(
        [](gui::TaskContext& ctx) -> long
        {
            long sum = 0;
            for (int i = 1; i <= 2000; ++i)
            {
                sum += i;
                ctx.report_progress(i);
            }
            return sum;
        });
    CHECK(started && task->running);
    CHECK(!task->start([](gui::TaskContext&) -> long { return 0; }));  // 動いている間は始めない
    CHECK(run_until([&]() { return finishes > 0; }));
    CHECK(finishes == 1 && task->result == 2001000L);
    CHECK(task->progress == 2000.0 && increasing);
    CHECK(progress_calls >= 0 && progress_calls < 2000);  // 間引いて届く

    // 終わった後は、もう一度始められる(進み具合などは戻る)。
    CHECK(task->start([](gui::TaskContext&) -> long { return 7; }) && task->progress == 0.0);
    CHECK(run_until([&]() { return finishes > 1; }));
    CHECK(task->result == 7L);
}

TEST(task_cancel_and_error)
{
    gui::Window window("t", 100, 100);
    gui::Task<void>* waiter = new gui::Task<void>(&window);
    bool finished = false;
    waiter->on_finish = [&](gui::Object*) { finished = true; };
    waiter->start(
        [](gui::TaskContext& ctx)
        {
            while (!ctx.cancel_requested())  // 取り消されるまで待つ
            {
                std::this_thread::sleep_for(std::chrono::milliseconds(1));
            }
        });
    waiter->cancel();
    CHECK(run_until([&]() { return finished; }));
    CHECK(waiter->cancelled && !waiter->running && waiter->error.get().empty());

    gui::Task<std::string>* failing = new gui::Task<std::string>(&window);
    bool failed = false;
    failing->on_finish = [&](gui::Object*) { failed = true; };
    failing->start([](gui::TaskContext&) -> std::string { throw std::runtime_error("読めなかった"); });
    CHECK(run_until([&]() { return failed; }));
    CHECK(failing->error.get() == "読めなかった" && failing->result.get().empty() && !failing->cancelled);
}

TEST(task_destroyed_while_running)
{
    gui::Window window("t", 100, 100);
    gui::Task<int>* task = new gui::Task<int>(&window);
    bool called = false;
    task->on_finish = [&](gui::Object*) { called = true; };
    task->on_progress = [&](gui::Object*) { called = true; };
    std::atomic<bool> gate(false), saw_cancel(false), returned(false);
    task->start(
        [&](gui::TaskContext& ctx) -> int
        {
            while (!gate)
            {
                std::this_thread::sleep_for(std::chrono::milliseconds(1));
            }
            saw_cancel = ctx.cancel_requested();
            ctx.report_progress(1);
            returned = true;
            return 1;
        });
    delete task;  // 動いている間に破棄する: 取り消しを頼み、結果は捨てる
    gate = true;
    CHECK(run_until([&]() { return returned.load(); }));
    std::this_thread::sleep_for(std::chrono::milliseconds(50));  // 戻った後の post が届くのを待つ
    run_until([]() { return false; }, 100);
    CHECK(saw_cancel && !called);
}

TEST(tool_bar_buttons_and_shortcuts)
{
    gui::Window window("t", 500, 300);
    gui::BoxLayout* box = new gui::BoxLayout(&window);
    box->margins = gui::Margins(0);
    gui::LineEdit* edit = new gui::LineEdit(&window);
    box->add(edit);
    gui::MenuBar* bar = new gui::MenuBar();
    bar->add_menu("ファイル(&F)")->add_item("開く");
    gui::ToolBar* tools = new gui::ToolBar();
    std::string done;
    gui::Action* save = new gui::Action(&window, "保存(&S)...");
    save->shortcut = "Ctrl+S";
    save->on_trigger = [&](gui::Object*) { done += "save "; };
    tools->add_action(save);
    gui::Action* bold = tools->add_item("太字");
    bold->checkable = true;
    tools->add_separator();
    gui::StatusBar* status = new gui::StatusBar();
    window.menu_bar = bar;
    window.tool_bar = tools;
    window.status_bar = status;
    window.visible = true;
    window.apply_layout();

    // 上からメニューバー・ツールバー・レイアウト、下端にステータスバー。
    CHECK(near(tools->y, bar->height) && near(edit->y, bar->height.get() + tools->height.get()));
    CHECK(near(status->y.get() + status->height.get(), window.height) && status->height > 0);
    // ボタンはツールバーの子。押すと Action を実行する。ツールチップは文字(メニュー向けの表記を省く)とショートカット。
    std::vector<gui::Widget*> buttons;
    const std::vector<gui::Widget*> children = tools->children;
    for (std::size_t i = 0; i < children.size(); ++i)
    {
        if (children[i]->visible)
        {
            buttons.push_back(children[i]);
        }
    }
    CHECK(buttons.size() == 2 && buttons[0]->tooltip.get() == "保存 (Ctrl+S)");
    const gui::Point at = buttons[0]->map_to_window(gui::Point(5, 5));
    dispatch_click(window, at);
    CHECK(done == "save ");
    dispatch_click(window, buttons[1]->map_to_window(gui::Point(5, 5)));
    CHECK(bold->checked);
    // ツールバーの Action のショートカットはウィンドウ全体で効く(入力欄にフォーカスがあっても)。
    edit->focused = true;
    dispatch_key(window, SDLK_S, true, SDL_KMOD_LCTRL);
    CHECK(done == "save save ");
    // Action を無効にするとボタンも無効。破棄するとボタンも消える。
    save->enabled = false;
    CHECK(!buttons[0]->enabled);
    dispatch_click(window, at);
    CHECK(done == "save save ");
    delete save;
    CHECK(tools->actions.get().size() == 1);
    window.apply_layout();
    CHECK(tools->preferred_size.get().width > 0);
}

TEST(tool_bar_overflow_menu)
{
    gui::Window window("t", 200, 200);
    gui::ToolBar* tools = new gui::ToolBar();
    std::string done;
    for (int i = 0; i < 8; ++i)
    {
        tools->add_item("項目" + std::to_string(i), [&, i](gui::Object*) { done = std::to_string(i); });
    }
    window.tool_bar = tools;
    window.visible = true;
    window.apply_layout();
    // 収まらない項目は隠れ、右端の » が出る。
    int shown = 0;
    gui::Widget* chevron = nullptr;
    const std::vector<gui::Widget*> children = tools->children;
    for (std::size_t i = 0; i < children.size(); ++i)
    {
        if (children[i]->visible)
        {
            ++shown;
            if (!chevron || children[i]->x > chevron->x)
            {
                chevron = children[i];
            }
        }
    }
    CHECK(shown > 1 && shown < 9 && chevron->tooltip.get() == "その他の項目");
    CHECK(chevron->x.get() + chevron->width.get() <= 200);
    // » のメニューから、隠れた項目を選べる。
    dispatch_click(window, chevron->map_to_window(gui::Point(5, 5)));
    dispatch_key(window, SDLK_UP);  // 最後の項目
    dispatch_key(window, SDLK_RETURN);
    CHECK(done == "7");
}

TEST(status_bar_message_and_widgets)
{
    gui::Window window("t", 400, 200);
    gui::StatusBar* status = new gui::StatusBar();
    status->text = "準備完了";
    gui::Label* position = new gui::Label(nullptr, "行 1");
    status->add_widget(position);
    window.status_bar = status;
    window.visible = true;
    window.apply_layout();
    CHECK(position->parent == status && near(position->x.get() + position->width.get(), 400 - 10));
    status->show_message("保存した", 50);
    CHECK(status->message.get() == "保存した");
    CHECK(run_until([&]() { return status->message.get().empty(); }));  // 時間が経つと消える
    status->show_message("ずっと");
    run_until([]() { return false; }, 100);
    CHECK(status->message.get() == "ずっと");
    status->clear_message();
    CHECK(status->message.get().empty() && status->text.get() == "準備完了");
    delete position;  // 部品を破棄しても大丈夫
    window.apply_layout();
    window.status_bar = nullptr;
    CHECK(window.status_bar == nullptr);
}

TEST(splitter_sizes_stretch_and_drag)
{
    gui::Window window("t", 600, 200);
    gui::Splitter* split = new gui::Splitter(&window);
    split->bounds = gui::Rect(0, 0, 400, 200);
    gui::Widget* left = new gui::Widget();
    gui::Widget* right = new gui::Widget();
    left->minimum_size = gui::Size(50, 0);
    right->minimum_size = gui::Size(50, 0);
    gui::SplitterPane* left_pane = split->add_widget(left);
    gui::SplitterPane* right_pane = split->add_widget(right);
    CHECK(left->parent == split && split->panes.get().size() == 2 && left_pane->widget == left);
    window.visible = true;
    window.apply_layout();
    // 望む大きさ(50)から始め、余りを stretch(1 と 1)で配る。境目は 6dp。
    std::vector<float> s = split->sizes;
    CHECK(near(s[0], 197) && near(s[1], 197) && near(right->x, 203));

    // 左の stretch を 0 にすると、大きくしても左は幅を保つ。
    left_pane->stretch = 0;
    split->bounds = gui::Rect(0, 0, 500, 200);
    window.apply_layout();
    s = split->sizes;
    CHECK(near(s[0], 197) && near(s[1], 297));

    // 境目をドラッグする。最小より小さくはならず、最小の半分より小さくするとたたむ。
    int changes = 0;
    split->on_change = [&](gui::Object*) { ++changes; };
    dispatch_mouse(window, SDL_EVENT_MOUSE_BUTTON_DOWN, gui::Point(200, 100));
    dispatch_mouse(window, SDL_EVENT_MOUSE_MOTION, gui::Point(103, 100));
    window.apply_layout();
    CHECK(near(split->sizes.get()[0], 100) && near(split->sizes.get()[1], 394) && changes == 1);
    dispatch_mouse(window, SDL_EVENT_MOUSE_MOTION, gui::Point(33, 100));
    CHECK(near(split->sizes.get()[0], 50));  // 最小(30 は最小の半分より大きい)
    dispatch_mouse(window, SDL_EVENT_MOUSE_MOTION, gui::Point(13, 100));
    window.apply_layout();
    CHECK(near(split->sizes.get()[0], 0) && left_pane->collapsed && !left->visible);
    dispatch_mouse(window, SDL_EVENT_MOUSE_BUTTON_UP, gui::Point(13, 100));
    // たたんだ区画は、境目(左端)をドラッグして戻す。
    dispatch_mouse(window, SDL_EVENT_MOUSE_BUTTON_DOWN, gui::Point(3, 100));
    dispatch_mouse(window, SDL_EVENT_MOUSE_MOTION, gui::Point(153, 100));
    dispatch_mouse(window, SDL_EVENT_MOUSE_BUTTON_UP, gui::Point(153, 100));
    window.apply_layout();
    CHECK(near(split->sizes.get()[0], 150) && !left_pane->collapsed && left->visible);
    // たためない区画はたたまない。
    right_pane->collapsible = false;
    dispatch_mouse(window, SDL_EVENT_MOUSE_BUTTON_DOWN, gui::Point(153, 100));
    dispatch_mouse(window, SDL_EVENT_MOUSE_MOTION, gui::Point(498, 100));
    dispatch_mouse(window, SDL_EVENT_MOUSE_BUTTON_UP, gui::Point(498, 100));
    CHECK(near(split->sizes.get()[1], 50));

    // sizes の代入(合計が合わない分は stretch で合わせる)。
    split->sizes = std::vector<float>{120, 100};
    window.apply_layout();
    CHECK(near(split->sizes.get()[0], 120) && near(split->sizes.get()[1], 374));
    // 区画の部品を破棄すると、その区画は消える。
    delete left;
    window.apply_layout();
    CHECK(split->panes.get().size() == 1 && near(right->x, 0) && near(right->width, 500));
    split->on_change = gui::NotifyHandler();
}

TEST(splitter_vertical_and_nested)
{
    gui::Window window("t", 300, 400);
    gui::Splitter* outer = new gui::Splitter(&window, gui::Orientation::VERTICAL);
    outer->bounds = gui::Rect(0, 0, 300, 400);
    gui::Widget* top = new gui::Widget();
    gui::Splitter* inner = new gui::Splitter();
    inner->add_widget(new gui::Widget());
    inner->add_widget(new gui::Widget());
    inner->add_widget(new gui::Widget());
    outer->add_widget(top);
    outer->add_widget(inner);
    outer->sizes = std::vector<float>{100, 294};
    window.visible = true;
    window.apply_layout();
    CHECK(near(top->height, 100) && near(inner->y, 106) && near(inner->height, 294) && near(inner->width, 300));
    // 3 つの区画は同じ stretch なので、同じ幅(境目 2 つ)。
    const std::vector<float> s = inner->sizes;
    CHECK(s.size() == 3 && near(s[0], 96) && near(s[1], 96) && near(s[2], 96));
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
