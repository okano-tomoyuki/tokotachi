// Tokotachi のテスト。SDL の offscreen のビデオドライバで、画面なしで動かす(CMake が環境変数を設定する)。
#include <tokotachi/tokotachi.hpp>

#include "../src/core/binding_registry.hpp"
#include "../src/core/application_impl.hpp"
#include "../src/windows/window_impl.hpp"

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
struct TestEdit : tokotachi::LineEdit
{
    explicit TestEdit(tokotachi::Widget* parent_widget, const std::string& s = std::string()) : tokotachi::LineEdit(parent_widget, s)
    {
    }

    void key(tokotachi::Key k, tokotachi::Modifiers m = tokotachi::MODIFIER_NONE)
    {
        tokotachi::KeyEvent e(k, m, false, 0);
        key_down(e);
    }

    void type(const std::string& s)
    {
        tokotachi::TextInputEvent e(s);
        text_input(e);
    }

    void compose(const std::string& s, int start = -1, int length = -1)
    {
        tokotachi::TextEditingEvent e(s, start, length);
        text_editing(e);
    }
};

struct TestButton : tokotachi::Button
{
    explicit TestButton(tokotachi::Widget* parent_widget) : tokotachi::Button(parent_widget, "b") {}

    bool key(tokotachi::Key k)
    {
        tokotachi::KeyEvent e(k, tokotachi::MODIFIER_NONE, false, 0);
        key_down(e);
        return e.accepted;
    }

    bool press(tokotachi::MouseButton b)
    {
        tokotachi::MouseEvent e(tokotachi::Point(1, 1), b, tokotachi::MODIFIER_NONE, 1);
        mouse_down(e);
        return e.accepted;
    }
};

struct Counted : tokotachi::Widget
{
    Counted(tokotachi::Widget* parent_widget, int* counter) : tokotachi::Widget(parent_widget), counter_(counter) {}
    ~Counted() { ++*counter_; }

private:
    int* counter_;
};

TEST(text_layout_caret_stops)
{
    // e + 結合アキュート(U+0301)は 1 つのクラスタ。
    const std::string s = "ae\xCC\x81\xE6\x97\xA5";  // a, e+U+0301, 日
    tokotachi::TextLayout l(s, tokotachi::Font(14), 1.0f);
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
    CHECK(l.offset_at(tokotachi::Point(-10, 0)) == 0);
    CHECK(l.offset_at(tokotachi::Point(1000, 0)) == s.size());
    CHECK(l.line_count() == 1);
}

TEST(text_layout_bold_and_italic)
{
    // 太字は字の送りが広くなり(本物の太字の書体か、合成した太字)、斜体は同じ文字の並びを描ける(docs/adr/0045)
    const std::string s = "Tokotachi å¸¸ç«";  // Tokotachi 常立
    const tokotachi::TextLayout regular(s, tokotachi::Font(14), 1.0f);
    const tokotachi::TextLayout bold(s, tokotachi::Font(14, true), 1.0f);
    const tokotachi::TextLayout italic(s, tokotachi::Font(14, false, true), 1.0f);
    const tokotachi::TextLayout both(s, tokotachi::Font(14, true, true), 1.0f);
    CHECK(bold.width() > regular.width());
    CHECK(italic.width() > 0 && both.width() > 0);
    CHECK(bold.font().bold && !bold.font().italic);
    CHECK(tokotachi::Font(14, true) != tokotachi::Font(14));
    CHECK(tokotachi::Font(14, false, true) != tokotachi::Font(14));
}

TEST(text_layout_hard_line_breaks)
{
    const std::string s = "ab\ncd\r\nef";  // \n と \r\n
    tokotachi::TextLayout l(s, tokotachi::Font(14), 1.0f);
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
    CHECK(l.offset_at(tokotachi::Point(0, l.line_height() * 2 + 1)) == 7);

    tokotachi::TextLayout trailing("a\n", tokotachi::Font(14), 1.0f);  // 改行文字で終わると、最後に空の行がある
    CHECK(trailing.line_count() == 2);
    CHECK(trailing.line_start(1) == 2);
}

TEST(text_layout_wraps_at_break_opportunities)
{
    const std::string s = "aaa bbb ccc";
    const float w = tokotachi::TextLayout("aaa bbb", tokotachi::Font(14), 1.0f).width();
    tokotachi::TextLayout l(s, tokotachi::Font(14), 1.0f, w + 1);
    CHECK(l.line_count() == 2);
    CHECK(l.line_start(1) == 8);  // 空白の後ろで分ける(行末の空白は幅に含めない)
    CHECK(l.line_of(8) == 1);     // 折り返した境目は次の行
    CHECK(l.width() <= w + 1);

    // 1 つの単語が幅に収まらなければ、文字の途中で分ける。
    tokotachi::TextLayout narrow("abcdefgh", tokotachi::Font(14), 1.0f, tokotachi::TextLayout("abc", tokotachi::Font(14), 1.0f).width() + 1);
    CHECK(narrow.line_count() >= 2);
    CHECK(narrow.line_start(1) == 3);
}

TEST(text_layout_japanese_line_breaking)
{
    // 「、」は行の始めに置かない(禁則)。「あいうえお」の幅で折り返すと、「お」ごと次の行へ送る。
    const std::string s = "あいうえお、かきくけこ";
    const float w = tokotachi::TextLayout("あいうえお", tokotachi::Font(14), 1.0f).width();
    tokotachi::TextLayout l(s, tokotachi::Font(14), 1.0f, w + 0.5f);
    CHECK(l.line_count() >= 2);
    const std::string second = s.substr(l.line_start(1), 3);
    CHECK(second != "、");
    CHECK(second == "お");
}

TEST(text_layout_alignment)
{
    tokotachi::TextLayout l("a\nwide line", tokotachi::Font(14), 1.0f, 0, tokotachi::Align::END);
    CHECK(near(l.line_rect(0).right(), l.width()));  // 行の右端をそろえる
    CHECK(near(l.line_rect(1).x, 0));
}

TEST(text_layout_words)
{
    const std::string s = "hello world  foo";
    tokotachi::TextLayout l(s, tokotachi::Font(14), 1.0f);
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
    tokotachi::TextLayout lj(j, tokotachi::Font(14), 1.0f);
    lj.word_at(10, a, b);  // 「テ」の位置
    CHECK(j.substr(a, b - a) == "テキスト");
}

TEST(text_layout_rtl_paragraph)
{
    // ヘブライ語だけの段落は右から左。論理の先頭(位置 0)のキャレットは右端、末尾は左端。
    const std::string s = "שלום";  // 4 文字 x 2 バイト
    tokotachi::TextLayout l(s, tokotachi::Font(14), 1.0f);
    CHECK(l.line_is_rtl(0));
    CHECK(l.caret_position(0).x > l.caret_position(s.size()).x);
    CHECK(near(l.caret_position(0).x, l.width()));
    CHECK(near(l.caret_position(s.size()).x, 0));
    CHECK(l.caret_position(2).x < l.caret_position(0).x);  // 論理の順に進むと左へ動く
    CHECK(l.offset_at(tokotachi::Point(-5, 0)) == s.size());     // 左端は論理の末尾
    CHECK(l.offset_at(tokotachi::Point(l.width() + 5, 0)) == 0);
    CHECK(l.next_offset(0) == 2);                          // 矢印キーでの移動は論理の順
}

TEST(text_layout_mixed_direction)
{
    // 左から右の段落に右から左の単語が入る: 見た目は "abc " + ヘブライ語(右から左) + " def"。
    const std::string s = "abc שלום def";  // ヘブライ語は 4..12
    tokotachi::TextLayout l(s, tokotachi::Font(14), 1.0f);
    CHECK(!l.line_is_rtl(0));
    const float run_left = l.caret_position(4).x;  // ここは "c " の直後ではなく、ヘブライ語の最初の文字の右端
    CHECK(l.caret_position(4).x > l.caret_position(6).x);   // ヘブライ語の中では、論理の順に進むと左へ
    CHECK(l.caret_position(13).x > run_left);                // " def" はヘブライ語の右
    // "c " とヘブライ語の最初の 1 文字を選ぶと、見た目では離れた 2 つの矩形になる。
    const std::vector<tokotachi::Rect> rects = l.selection_rects(2, 6);
    CHECK(rects.size() == 2);
    // 左から右の部分だけなら 1 つ
    CHECK(l.selection_rects(0, 4).size() == 1);
}

TEST(text_layout_rtl_alignment)
{
    // 右から左の段落では、START は右にそろえる。
    tokotachi::TextLayout l("שלום\nשלום עולם", tokotachi::Font(14), 1.0f);
    CHECK(l.line_count() == 2);
    CHECK(near(l.line_rect(0).right(), l.width()));
    CHECK(l.line_rect(0).x > 0);
    CHECK(near(l.line_rect(1).x, 0));
}

TEST(text_layout_arabic_shaping)
{
    // アラビア語は文字がつながる(連結した形は、単独の形を並べたより幅が狭くなるのが普通)。
    const std::string word = "مرحبا";
    tokotachi::TextLayout joined(word, tokotachi::Font(14), 1.0f);
    float separate = 0;
    for (std::size_t i = 0; i < word.size(); i += 2)
    {
        separate += tokotachi::TextLayout(word.substr(i, 2), tokotachi::Font(14), 1.0f).width();
    }
    CHECK(joined.line_is_rtl(0));
    CHECK(joined.width() < separate);
}

TEST(text_layout_scales_with_display_scale)
{
    tokotachi::TextLayout a("Hello 日本語", tokotachi::Font(14), 1.0f);
    tokotachi::TextLayout b("Hello 日本語", tokotachi::Font(14), 2.0f);
    // dp で返すので、倍率が変わってもほぼ同じ幅(ヒンティングの差の分だけずれる)。
    CHECK(b.width() > a.width() * 0.9f && b.width() < a.width() * 1.1f);
}

TEST(property_read_write)
{
    tokotachi::Window window("t", 300, 200);
    tokotachi::Label* label = new tokotachi::Label(&window, "abc");
    tokotachi::Label* other = new tokotachi::Label(&window);
    CHECK(label->text == "abc");
    label->text = "x";
    CHECK(label->text.get() == "x");
    label->text += "yz";  // 複合代入
    CHECK(label->text == "xyz");
    other->text = label->text;  // プロパティ同士の代入は値のコピー
    CHECK(other->text == "xyz");
    const std::string joined = "<" + label->text + ">";  // std::string と直接つなげる
    CHECK(joined == "<xyz>");

    label->bounds = tokotachi::Rect(10, 20, 100, 30);
    CHECK(label->x == 10.0f);
    label->x += 5;
    label->width = 50;
    const tokotachi::Rect b = label->bounds;
    CHECK(b == tokotachi::Rect(15, 20, 50, 30));
    CHECK(label->local_rect.get() == tokotachi::Rect(0, 0, 50, 30));
    CHECK(label->parent == static_cast<tokotachi::Widget*>(&window));
    CHECK(label->window == &window);
}

TEST(handler_property)
{
    tokotachi::Window window("t", 300, 200);
    TestButton* a = new TestButton(&window);
    TestButton* b = new TestButton(&window);
    tokotachi::Object* last = nullptr;
    int count = 0;
    const tokotachi::NotifyHandler shared = [&](tokotachi::Object* sender)
    {
        last = sender;
        ++count;
    };
    a->on_click = shared;
    b->on_click = a->on_click;  // 1 つのハンドラを共有し、sender で区別する
    a->key(tokotachi::Key::SPACE);
    CHECK(last == a);
    b->key(tokotachi::Key::SPACE);
    CHECK(last == b);
    b->on_click = nullptr;  // 解除
    b->key(tokotachi::Key::SPACE);
    CHECK(count == 2);
}

TEST(widget_handler_and_propagation)
{
    tokotachi::Window window("t", 300, 200);
    TestButton* button = new TestButton(&window);
    int downs = 0;
    button->on_mouse_down = [&](tokotachi::Object*, tokotachi::MouseEvent&) { ++downs; };
    CHECK(button->press(tokotachi::MouseButton::LEFT));    // ボタンが受け取る
    CHECK(downs == 1);                               // 上書きした仮想関数からもハンドラが呼ばれる
    button->on_mouse_down = nullptr;
    CHECK(!button->press(tokotachi::MouseButton::RIGHT));  // 受け取らない(親へ送られる)
    CHECK(!button->key(tokotachi::Key::A));
}

TEST(widget_ownership)
{
    tokotachi::Window window("t", 200, 100);
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
        tokotachi::Window window("t", 200, 100);
        new Counted(new Counted(&window, &destroyed), &destroyed);
    }
    CHECK(destroyed == 2);
}

TEST(child_at_and_mapping)
{
    tokotachi::Window window("t", 300, 200);
    tokotachi::Widget* a = new tokotachi::Widget(&window);
    a->bounds = tokotachi::Rect(10, 20, 100, 50);
    tokotachi::Widget* b = new tokotachi::Widget(a);
    b->bounds = tokotachi::Rect(5, 5, 10, 10);
    CHECK(window.child_at(tokotachi::Point(16, 26)) == b);
    CHECK(window.child_at(tokotachi::Point(50, 40)) == a);
    CHECK(window.child_at(tokotachi::Point(200, 150)) == nullptr);
    CHECK(b->map_to_window(tokotachi::Point(1, 1)) == tokotachi::Point(16, 26));
    b->visible = false;
    CHECK(window.child_at(tokotachi::Point(16, 26)) == a);
    CHECK(!b->showing);
}

TEST(line_edit_editing)
{
    tokotachi::Window window("t", 300, 100);
    TestEdit* edit = new TestEdit(&window);
    edit->bounds = tokotachi::Rect(0, 0, 200, 30);
    int changes = 0;
    edit->on_change = [&](tokotachi::Object*) { ++changes; };
    edit->focused = true;
    CHECK(edit->focused);

    edit->type("abc");
    CHECK(edit->text == "abc");
    CHECK(edit->cursor_position == 3u);
    edit->key(tokotachi::Key::LEFT);
    edit->key(tokotachi::Key::BACKSPACE);
    CHECK(edit->text == "ac");
    CHECK(edit->cursor_position == 1u);
    edit->key(tokotachi::Key::DEL);
    CHECK(edit->text == "a");

    edit->type("日本");
    CHECK(edit->text == "a日本");
    edit->key(tokotachi::Key::BACKSPACE);  // マルチバイトの文字を 1 文字消す
    CHECK(edit->text == "a日");

    edit->key(tokotachi::Key::HOME);
    edit->key(tokotachi::Key::END, tokotachi::MODIFIER_SHIFT);
    CHECK(edit->selected_text == "a日");
    CHECK(edit->has_selection);
    edit->type("x");  // 選択を置き換える
    CHECK(edit->text == "x");
    CHECK(changes == 6);
}

TEST(line_edit_preedit)
{
    tokotachi::Window window("t", 300, 100);
    TestEdit* edit = new TestEdit(&window);
    edit->bounds = tokotachi::Rect(0, 0, 200, 30);
    edit->focused = true;
    edit->type("ab");
    edit->key(tokotachi::Key::LEFT);

    edit->compose("にほん", 0, 3);
    CHECK(edit->text == "ab");       // 変換中の文字列はテキストに入らない
    edit->key(tokotachi::Key::BACKSPACE);  // 変換中のキーは IME のもの
    CHECK(edit->text == "ab");
    edit->compose("");
    edit->type("日本");  // 確定
    CHECK(edit->text == "a日本b");
    CHECK(edit->cursor_position == 7u);
}

TEST(line_edit_strips_control_characters)
{
    tokotachi::Window window("t", 300, 100);
    tokotachi::LineEdit* edit = new tokotachi::LineEdit(&window, "a\nb\tc");
    CHECK(edit->text == "abc");
    edit->insert("x\r\ny");
    CHECK(edit->text == "abcxy");
    edit->text = "1\n2";
    CHECK(edit->text == "12");
}

TEST(button_click_by_key)
{
    tokotachi::Window window("t", 300, 100);
    TestButton* button = new TestButton(&window);
    int clicks = 0;
    button->on_click = [&](tokotachi::Object*) { ++clicks; };
    button->key(tokotachi::Key::SPACE);
    button->key(tokotachi::Key::RETURN);
    button->key(tokotachi::Key::A);
    CHECK(clicks == 2);
}

TEST(focus_cleared_when_hidden)
{
    tokotachi::Window window("t", 300, 100);
    tokotachi::Widget* panel = new tokotachi::Widget(&window);
    tokotachi::LineEdit* edit = new tokotachi::LineEdit(panel);
    edit->focused = true;
    CHECK(window.focused_widget == static_cast<tokotachi::Widget*>(edit));
    panel->visible = false;
    CHECK(window.focused_widget == nullptr);
    panel->visible = true;
    edit->focused = true;
    delete panel;  // フォーカスのあるウィジェットを破棄してもぶら下がらない
    CHECK(window.focused_widget == nullptr);
}

TEST(timer_repeats_on_main_thread)
{
    tokotachi::Application* app = tokotachi::Application::instance();
    tokotachi::Window window("t", 100, 100);
    window.visible = true;
    tokotachi::Timer timer;
    timer.interval = 10;
    int fired = 0;
    timer.on_timer = [&](tokotachi::Object* sender)
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
    tokotachi::Application* app = tokotachi::Application::instance();
    tokotachi::Window window("t", 100, 100);
    window.visible = true;
    tokotachi::Timer once;
    once.interval = 5;
    once.repeat = false;
    int fired = 0;
    once.on_timer = [&](tokotachi::Object*) { ++fired; };
    once.enabled = true;

    tokotachi::Timer stopper;  // once が 2 回目を呼ばないことを確かめるため、少し待ってから終える
    stopper.interval = 60;
    stopper.on_timer = [&](tokotachi::Object*) { app->quit(); };
    stopper.enabled = true;
    app->exec();
    CHECK(fired == 1);
    CHECK(!once.enabled);  // 1 回だけのタイマーは呼んだ後に止まる
}

TEST(timer_destroyed_while_running)
{
    tokotachi::Application* app = tokotachi::Application::instance();
    tokotachi::Window window("t", 100, 100);
    window.visible = true;
    int fired = 0;
    {
        tokotachi::Timer t;
        t.interval = 1;
        t.on_timer = [&](tokotachi::Object*) { ++fired; };
        t.enabled = true;
    }  // 破棄すると止まる(すでに積まれたイベントが届いても呼ばない)
    tokotachi::Timer stopper;
    stopper.interval = 30;
    stopper.on_timer = [&](tokotachi::Object*) { app->quit(); };
    stopper.enabled = true;
    app->exec();
    CHECK(fired == 0);
}

TEST(handler_shared_between_widget_and_timer)
{
    tokotachi::Window window("t", 100, 100);
    TestButton* button = new TestButton(&window);
    tokotachi::Timer timer;
    int from_button = 0, from_timer = 0;
    // sender は Object*。種類は dynamic_cast で見分ける。
    const tokotachi::NotifyHandler refresh = [&](tokotachi::Object* sender)
    {
        if (dynamic_cast<tokotachi::Button*>(sender))
        {
            ++from_button;
        }
        else if (dynamic_cast<tokotachi::Timer*>(sender))
        {
            ++from_timer;
        }
    };
    button->on_click = refresh;
    timer.on_timer = button->on_click;
    button->key(tokotachi::Key::SPACE);
    timer.on_timer.get()(&timer);
    CHECK(from_button == 1);
    CHECK(from_timer == 1);
}

struct CountedObject : tokotachi::Object
{
    CountedObject(tokotachi::Object* owner_object, int* counter) : tokotachi::Object(owner_object), counter_(counter) {}
    ~CountedObject() { ++*counter_; }

private:
    int* counter_;
};

TEST(object_ownership)
{
    int destroyed = 0;
    {
        tokotachi::Window window("t", 100, 100);
        tokotachi::Widget* panel = new tokotachi::Widget(&window);
        new CountedObject(&window, &destroyed);
        new CountedObject(panel, &destroyed);  // ウィジェットもウィジェット以外のオブジェクトを所有できる
        tokotachi::Timer* timer = new tokotachi::Timer(&window);
        CHECK(timer->owner == static_cast<tokotachi::Object*>(&window));
        CHECK(panel->owner == static_cast<tokotachi::Object*>(&window));
        CountedObject* early = new CountedObject(&window, &destroyed);
        delete early;  // 先に破棄すると所有者の一覧から外れる
        CHECK(destroyed == 1);
    }
    CHECK(destroyed == 3);
}

TEST(binding_variable_and_widgets)
{
    tokotachi::Window window("t", 300, 100);
    TestEdit* edit = new TestEdit(&window);
    edit->bounds = tokotachi::Rect(0, 0, 200, 30);
    tokotachi::Label* label = new tokotachi::Label(&window);
    tokotachi::StringVariable* name = new tokotachi::StringVariable(&window, "初期値");
    int changes = 0, sets = 0;
    name->on_change = [&](tokotachi::Object* sender)
    {
        CHECK(sender == name);
        ++changes;
    };
    name->on_set = [&](tokotachi::Object*) { ++sets; };

    tokotachi::bind(edit->text, name->value);  // 結んだ時点で edit を name の値にする
    tokotachi::bind(label->text, name->value);
    CHECK(edit->text == "初期値");
    CHECK(label->text == "初期値");
    CHECK(changes == 0);

    // 利用者の操作 → 変数 → もう一方のウィジェット
    edit->focused = true;
    edit->key(tokotachi::Key::END);
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
    tokotachi::Window window("t", 300, 100);
    TestEdit* edit = new TestEdit(&window, "hello");
    tokotachi::Label* label = new tokotachi::Label(&window);
    tokotachi::bind(label->text, edit->selected_text);
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
    tokotachi::Window window("t", 300, 100);
    tokotachi::Widget* w = new tokotachi::Widget(&window);
    tokotachi::Variable<float>* width = new tokotachi::Variable<float>(&window, 0);
    int changes = 0;
    width->on_change = [&](tokotachi::Object*) { ++changes; };
    tokotachi::bind(width->value, w->width);  // width->value を w の幅にする
    w->bounds = tokotachi::Rect(0, 0, 40, 10);  // bounds を変えると、一緒に変わった width も知らせる
    CHECK(width->value == 40.0f);
    width->value = 25;
    CHECK(w->width == 25.0f);
    const tokotachi::Rect b = w->bounds;
    CHECK(b.width == 25.0f);
    w->x = 5;  // 幅は変わらないので知らせない
    CHECK(changes == 2);  // bounds の代入・value の代入(結んだときは同じ値なので知らせない)
}

TEST(binding_focus)
{
    tokotachi::Window window("t", 300, 100);
    tokotachi::LineEdit* a = new tokotachi::LineEdit(&window);
    tokotachi::LineEdit* b = new tokotachi::LineEdit(&window);
    tokotachi::BoolVariable* a_focused = new tokotachi::BoolVariable(&window, false);
    tokotachi::bind(a_focused->value, a->focused);
    a->focused = true;
    CHECK(a_focused->value);
    b->focused = true;  // フォーカスが移ると、移る前のウィジェットの focused も知らせる
    CHECK(!a_focused->value);
    a_focused->value = true;  // 変数からフォーカスを動かす
    CHECK(a->focused);
}

TEST(binding_cycle_terminates)
{
    tokotachi::Window window("t", 100, 100);
    tokotachi::IntVariable* a = new tokotachi::IntVariable(&window);
    tokotachi::IntVariable* b = new tokotachi::IntVariable(&window);
    tokotachi::IntVariable* c = new tokotachi::IntVariable(&window);
    tokotachi::bind(a->value, b->value);
    tokotachi::bind(b->value, c->value);
    tokotachi::bind(c->value, a->value);
    int changes = 0;
    a->on_change = [&](tokotachi::Object*) { ++changes; };
    a->value = 7;  // 循環していても止まる。値が変わったときだけ伝えるので、通知は重複しない
    CHECK(b->value == 7);
    CHECK(c->value == 7);
    CHECK(changes == 1);
}

TEST(binding_removed_with_object)
{
    tokotachi::Window window("t", 100, 100);
    const std::size_t before = tokotachi::detail::binding_count();
    tokotachi::StringVariable* v = new tokotachi::StringVariable(&window, "a");
    tokotachi::Label* label = new tokotachi::Label(&window);
    tokotachi::LineEdit* edit = new tokotachi::LineEdit(&window);
    tokotachi::bind(label->text, v->value);
    tokotachi::bind(edit->text, v->value);
    tokotachi::bind(edit->text, v->value);  // 同じ組は 2 度結ばない
    CHECK(tokotachi::detail::binding_count() == before + 2);
    delete label;  // 破棄すると、その結びつきは外れる
    CHECK(tokotachi::detail::binding_count() == before + 1);
    v->value = "b";
    CHECK(edit->text == "b");
    tokotachi::unbind(edit->text, v->value);
    CHECK(tokotachi::detail::binding_count() == before);
    v->value = "c";
    CHECK(edit->text == "b");
}

struct TestCheck : tokotachi::CheckBox
{
    explicit TestCheck(tokotachi::Widget* parent_widget) : tokotachi::CheckBox(parent_widget, "c") {}
    void press_space()
    {
        tokotachi::KeyEvent e(tokotachi::Key::SPACE, tokotachi::MODIFIER_NONE, false, 0);
        key_down(e);
    }
};

struct TestRadio : tokotachi::RadioButton
{
    TestRadio(tokotachi::Widget* parent_widget, int v) : tokotachi::RadioButton(parent_widget, "r", v) {}
    void press_space()
    {
        tokotachi::KeyEvent e(tokotachi::Key::SPACE, tokotachi::MODIFIER_NONE, false, 0);
        key_down(e);
    }
};

TEST(check_box_toggles_and_binds)
{
    tokotachi::Window window("t", 200, 100);
    TestCheck* check = new TestCheck(&window);
    tokotachi::BoolVariable* flag = new tokotachi::BoolVariable(&window, true);
    int changes = 0, clicks = 0;
    check->on_change = [&](tokotachi::Object*) { ++changes; };
    check->on_click = [&](tokotachi::Object*) { ++clicks; };
    tokotachi::bind(check->checked, flag->value);
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
    tokotachi::Window window("t", 200, 100);
    TestRadio* r1 = new TestRadio(&window, 1);
    TestRadio* r2 = new TestRadio(&window, 2);
    TestRadio* r3 = new TestRadio(&window, 3);
    tokotachi::IntVariable* choice = new tokotachi::IntVariable(&window, 1);
    int choice_changes = 0, r1_changes = 0;
    choice->on_change = [&](tokotachi::Object*) { ++choice_changes; };
    r1->on_change = [&](tokotachi::Object*) { ++r1_changes; };
    tokotachi::bind(r1->group_value, choice->value);
    tokotachi::bind(r2->group_value, choice->value);
    tokotachi::bind(r3->group_value, choice->value);
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
struct Fixed : tokotachi::Widget
{
    Fixed(tokotachi::Widget* parent_widget, float w, float h, float min_w = 0, float min_h = 0)
        : tokotachi::Widget(parent_widget), preferred_(w, h), minimum_(min_w, min_h)
    {
    }

    void set_preferred(float w, float h)
    {
        preferred_ = tokotachi::Size(w, h);
        request_layout();
    }

protected:
    tokotachi::Size compute_preferred_size() const override { return preferred_; }
    tokotachi::Size compute_minimum_size() const override { return minimum_; }

private:
    tokotachi::Size preferred_;
    tokotachi::Size minimum_;
};

bool rect_is(const tokotachi::Widget* w, float x, float y, float width, float height)
{
    const tokotachi::Rect r = w->bounds;
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
    tokotachi::Window window("t", 400, 200);
    tokotachi::Widget* panel = new tokotachi::Widget(&window);
    panel->bounds = tokotachi::Rect(0, 0, 300, 40);
    tokotachi::BoxLayout* box = new tokotachi::BoxLayout(panel, tokotachi::Orientation::HORIZONTAL);
    CHECK(panel->layout == static_cast<tokotachi::Layout*>(box));
    box->spacing = 10;
    Fixed* a = new Fixed(panel, 50, 20);
    Fixed* b = new Fixed(panel, 20, 20);
    Fixed* c = new Fixed(panel, 30, 20);
    box->add(a);
    box->add(b, 1);  // 余った幅を受け取る
    box->add(c, 0, tokotachi::Align::CENTER);
    panel->apply_layout();
    CHECK(rect_is(a, 0, 0, 50, 40));    // 直交する向きは既定で FILL
    CHECK(rect_is(b, 60, 0, 200, 40));  // 300 - 50 - 30 - 10 * 2
    CHECK(rect_is(c, 270, 10, 30, 20)); // CENTER
    // 望む大きさ: 並べる向きは合計 + 間隔、直交する向きは最大
    const tokotachi::Size pref = panel->preferred_size;
    CHECK(near(pref.width, 120) && near(pref.height, 20));

    b->visible = false;  // 隠れた項目は詰める
    panel->apply_layout();
    CHECK(rect_is(c, 60, 10, 30, 20));
}

TEST(box_layout_shrinks_to_minimum)
{
    tokotachi::Window window("t", 400, 200);
    tokotachi::Widget* panel = new tokotachi::Widget(&window);
    panel->bounds = tokotachi::Rect(0, 0, 40, 100);
    tokotachi::BoxLayout* box = new tokotachi::BoxLayout(panel);  // 縦
    box->spacing = 0;
    Fixed* a = new Fixed(panel, 10, 60, 0, 40);  // 縮められるのは 20
    Fixed* b = new Fixed(panel, 10, 60, 0, 20);  // 縮められるのは 40
    box->add(a);
    box->add(b);
    panel->apply_layout();  // 120 → 100: 差 20 を 1:2 で縮める(53.3 と 46.7。辺はピクセルに揃える)
    CHECK(rect_is(a, 0, 0, 40, 53));
    CHECK(rect_is(b, 0, 53, 40, 47));
    const tokotachi::Size min = panel->minimum_hint;
    CHECK(near(min.height, 60));
}

TEST(box_layout_spacers_and_margins)
{
    tokotachi::Window window("t", 400, 200);
    tokotachi::Widget* panel = new tokotachi::Widget(&window);
    panel->bounds = tokotachi::Rect(0, 0, 200, 50);
    tokotachi::BoxLayout* box = new tokotachi::BoxLayout(panel, tokotachi::Orientation::HORIZONTAL);
    box->spacing = 0;
    box->margins = tokotachi::Margins(5);
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
    tokotachi::Window window("t", 400, 200);
    tokotachi::Widget* panel = new tokotachi::Widget(&window);
    panel->bounds = tokotachi::Rect(0, 0, 300, 100);
    tokotachi::GridLayout* grid = new tokotachi::GridLayout(panel);
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
    const tokotachi::Size pref = panel->preferred_size;
    CHECK(near(pref.width, 170) && near(pref.height, 60));
}

TEST(grid_layout_span_widens_columns)
{
    tokotachi::Window window("t", 400, 200);
    tokotachi::Widget* panel = new tokotachi::Widget(&window);
    tokotachi::GridLayout* grid = new tokotachi::GridLayout(panel);
    grid->column_spacing = 0;
    new Fixed(panel, 10, 10);
    Fixed* a = new Fixed(panel, 20, 10);
    Fixed* b = new Fixed(panel, 20, 10);
    Fixed* span = new Fixed(panel, 100, 10);
    grid->add(a, 0, 0);
    grid->add(b, 0, 1);
    grid->add(span, 1, 0, 1, 2);  // 40 では足りないので、2 列に 30 ずつ足す
    const tokotachi::Size pref = panel->preferred_size;
    CHECK(near(pref.width, 100));
}

TEST(place_layout_relative_and_anchor)
{
    tokotachi::Window window("t", 400, 200);
    tokotachi::Widget* panel = new tokotachi::Widget(&window);
    panel->bounds = tokotachi::Rect(0, 0, 200, 100);
    tokotachi::PlaceLayout* place = new tokotachi::PlaceLayout(panel);
    Fixed* center = new Fixed(panel, 40, 20);
    Fixed* bar = new Fixed(panel, 10, 10);
    tokotachi::Placement p;
    p.relative_x = 0.5f;
    p.relative_y = 0.5f;
    p.anchor = tokotachi::Anchor::CENTER;
    place->add(center, p);
    tokotachi::Placement q;
    q.relative_y = 1;
    q.relative_width = 1;
    q.height = 8;
    q.anchor = tokotachi::Anchor::BOTTOM_LEFT;
    place->add(bar, q);
    panel->apply_layout();
    CHECK(rect_is(center, 80, 40, 40, 20));
    CHECK(rect_is(bar, 0, 92, 200, 8));
    panel->bounds = tokotachi::Rect(0, 0, 100, 50);  // 親の大きさが変わると置き直す
    panel->apply_layout();
    CHECK(rect_is(center, 30, 15, 40, 20));
}

TEST(layout_nesting_through_container_widgets)
{
    tokotachi::Window window("t", 400, 200);
    tokotachi::BoxLayout* column = new tokotachi::BoxLayout(&window);  // ウィンドウ自身にもレイアウトを付けられる
    column->spacing = 0;
    Fixed* top = new Fixed(&window, 50, 30);
    tokotachi::Widget* row = new tokotachi::Widget(&window);  // 入れ子は入れ物のウィジェットを介す
    tokotachi::BoxLayout* row_box = new tokotachi::BoxLayout(row, tokotachi::Orientation::HORIZONTAL);
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
    tokotachi::Window window("t", 400, 200);
    tokotachi::Widget* panel = new tokotachi::Widget(&window);
    panel->bounds = tokotachi::Rect(0, 0, 400, 40);
    tokotachi::BoxLayout* box = new tokotachi::BoxLayout(panel, tokotachi::Orientation::HORIZONTAL);
    tokotachi::Label* label = new tokotachi::Label(panel, "a");
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
    tokotachi::Window window("t", 400, 200);
    tokotachi::Widget* panel = new tokotachi::Widget(&window);
    panel->bounds = tokotachi::Rect(0, 0, 100, 100);
    int destroyed = 0;
    struct CountedLayout : tokotachi::BoxLayout
    {
        CountedLayout(tokotachi::Widget* t, int* c) : tokotachi::BoxLayout(t), counter(c) {}
        ~CountedLayout() { ++*counter; }
        int* counter;
    };
    new CountedLayout(panel, &destroyed);
    tokotachi::GridLayout* grid = new tokotachi::GridLayout(panel);  // 2 つ目を付けると古いレイアウトは破棄される
    CHECK(destroyed == 1);
    CHECK(panel->layout == static_cast<tokotachi::Layout*>(grid));

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
    b->bounds = tokotachi::Rect(1, 2, 3, 4);
    panel->apply_layout();
    CHECK(rect_is(b, 1, 2, 3, 4));
}

TEST(layout_notifies_bindings)
{
    tokotachi::Window window("t", 400, 200);
    tokotachi::Widget* panel = new tokotachi::Widget(&window);
    panel->bounds = tokotachi::Rect(0, 0, 200, 40);
    tokotachi::BoxLayout* box = new tokotachi::BoxLayout(panel, tokotachi::Orientation::HORIZONTAL);
    Fixed* a = new Fixed(panel, 10, 10);
    box->add(a, 1);
    tokotachi::Variable<float>* width = new tokotachi::Variable<float>(&window, 0);
    tokotachi::bind(width->value, a->width);
    panel->apply_layout();
    CHECK(near(width->value, 200));  // レイアウトが決めた幅が、結んだ変数に伝わる
    panel->width = 120;
    panel->apply_layout();
    CHECK(near(width->value, 120));
}

TEST(text_input_enabled_when_shown_after_focus)
{
    // 表示の前にフォーカスを移しても、表示したときに文字入力(IME)が有効になる(サンプルで見つかった不具合)。
    tokotachi::Window window("t", 200, 100);
    tokotachi::LineEdit* edit = new tokotachi::LineEdit(&window);
    edit->focused = true;
    CHECK(!window.impl()->text_input_active);  // 表示されていないうちは有効にしない
    window.visible = true;
    CHECK(window.impl()->text_input_active);
    CHECK(SDL_TextInputActive(window.impl()->window));

    tokotachi::Widget* panel = new tokotachi::Widget(&window);
    tokotachi::LineEdit* inner = new tokotachi::LineEdit(panel);
    panel->visible = false;
    inner->focused = true;  // 隠れているのでフォーカスは移らない
    CHECK(window.focused_widget == static_cast<tokotachi::Widget*>(edit));
}

TEST(reparent_moves_ownership_and_layout)
{
    tokotachi::Window window("t", 300, 200);
    tokotachi::Widget* a = new tokotachi::Widget(&window);
    tokotachi::Widget* b = new tokotachi::Widget(&window);
    tokotachi::BoxLayout* box_a = new tokotachi::BoxLayout(a);
    int destroyed = 0;
    Counted* child = new Counted(a, &destroyed);
    box_a->add(child);
    CHECK(a->children.get().size() == 1);

    child->parent = b;  // 付け替えると、表示のツリー・所有者・レイアウトから移る
    CHECK(child->parent == b);
    CHECK(child->owner == static_cast<tokotachi::Object*>(b));
    CHECK(a->children.get().empty());
    CHECK(b->children.get().size() == 1);
    delete a;  // 元の親を破棄しても、移ったウィジェットは残る
    CHECK(destroyed == 0);
    delete b;  // 新しい親と一緒に破棄される
    CHECK(destroyed == 1);
}

TEST(reparent_refuses_cycles_and_windows)
{
    tokotachi::Window window("t", 300, 200);
    tokotachi::Widget* a = new tokotachi::Widget(&window);
    tokotachi::Widget* b = new tokotachi::Widget(a);
    a->parent = b;  // 子孫を親にはできない
    CHECK(a->parent == static_cast<tokotachi::Widget*>(&window));
    tokotachi::Window other("u", 100, 100);
    other.parent = a;  // ウィンドウは付け替えられない
    CHECK(other.parent == nullptr);
}

TEST(reparent_to_other_window_clears_focus)
{
    tokotachi::Window w1("t", 300, 200);
    tokotachi::Window w2("u", 300, 200);
    tokotachi::LineEdit* edit = new tokotachi::LineEdit(&w1);
    edit->focused = true;
    CHECK(w1.focused_widget == static_cast<tokotachi::Widget*>(edit));
    edit->parent = &w2;  // 別のウィンドウへ移ると、元のウィンドウのフォーカスは外れる
    CHECK(w1.focused_widget == nullptr);
    CHECK(edit->window == &w2);
    edit->focused = true;
    CHECK(w2.focused_widget == static_cast<tokotachi::Widget*>(edit));
}

TEST(label_alignment)
{
    tokotachi::Window window("t", 300, 200);
    tokotachi::Label* label = new tokotachi::Label(&window, "abc");
    label->alignment = tokotachi::Align::END;
    CHECK(label->alignment == tokotachi::Align::END);
}

// 一色の画像。
tokotachi::Image solid_image(int w, int h, const tokotachi::Color& c)
{
    return tokotachi::Image::from_pixels(w, h, std::vector<tokotachi::Color>(static_cast<std::size_t>(w * h), c));
}

TEST(image_box_fit)
{
    tokotachi::Window window("t", 100, 100);
    window.background = tokotachi::Color(0, 0, 255);
    tokotachi::ImageBox* image_box = new tokotachi::ImageBox(&window, solid_image(4, 2, tokotachi::Color(255, 0, 0)));
    CHECK(image_box->preferred_size.get() == tokotachi::Size(4, 2));
    image_box->bounds = tokotachi::Rect(0, 0, 40, 40);
    window.visible = true;
    const auto red_at = [&](float x, float y)
    {
        tokotachi::Bitmap b = window.capture();
        const float s = static_cast<float>(b.width()) / 100;
        const tokotachi::Color c = b.pixel(static_cast<int>(x * s), static_cast<int>(y * s));
        return c.r == 255 && c.b == 0;
    };
    // NONE: 元の大きさで中ほど
    CHECK(red_at(20, 20) && !red_at(10, 20));
    // CONTAIN: 40×20 に広げて縦の中ほど
    image_box->fit = tokotachi::ImageFit::CONTAIN;
    CHECK(red_at(2, 20) && red_at(38, 12) && !red_at(20, 5));
    // COVER: 隙間なく覆う(はみ出した部分は部品の外に描かない)
    image_box->fit = tokotachi::ImageFit::COVER;
    CHECK(red_at(20, 2) && red_at(20, 38) && !red_at(60, 20));
    // STRETCH: 部品の大きさ
    image_box->fit = tokotachi::ImageFit::STRETCH;
    CHECK(red_at(2, 2) && red_at(38, 38));
    // 空の画像なら何も描かず、望む大きさは 0
    image_box->image = tokotachi::Image();
    CHECK(!red_at(20, 20) && image_box->preferred_size.get() == tokotachi::Size(0, 0));
}

TEST(label_and_button_icons)
{
    tokotachi::Window window("t", 300, 200);
    const tokotachi::Image icon = solid_image(32, 32, tokotachi::Color(255, 0, 0));
    tokotachi::Label* label = new tokotachi::Label(&window, "abc");
    const tokotachi::Size plain = label->preferred_size;
    label->icon = icon;
    // 画像(既定 16)と間(6)の分だけ広がる
    CHECK(label->preferred_size.get().width == plain.width + 22);
    label->icon_size = 40;
    CHECK(label->preferred_size.get().width == plain.width + 46 && label->preferred_size.get().height == 40);
    label->text = "";
    CHECK(label->preferred_size.get() == tokotachi::Size(40, 40));

    tokotachi::Button* button = new tokotachi::Button(&window, "OK");
    const tokotachi::Size plain_button = button->preferred_size;
    button->icon = icon;
    CHECK(button->preferred_size.get().width == plain_button.width + 22);
    // 文字が無ければ、画像だけの正方形に近いボタン
    button->text = "";
    const tokotachi::Size only = button->preferred_size;
    CHECK(only.width == only.height && only.width < plain_button.width);
    // 押したときに描く(画像の色が出る)
    button->bounds = tokotachi::Rect(0, 0, only.width, only.height);
    label->visible = false;
    window.visible = true;
    tokotachi::Bitmap b = window.capture();
    const float s = static_cast<float>(b.width()) / 300;
    const tokotachi::Color c = b.pixel(static_cast<int>(only.width / 2 * s), static_cast<int>(only.height / 2 * s));
    CHECK(c.r == 255 && c.g == 0 && c.b == 0);
}

TEST(line_edit_word_navigation)
{
    tokotachi::Window window("t", 300, 100);
    TestEdit* edit = new TestEdit(&window, "hello world foo");
    edit->bounds = tokotachi::Rect(0, 0, 200, 30);
    edit->focused = true;
    edit->key(tokotachi::Key::HOME);
    edit->key(tokotachi::Key::RIGHT, tokotachi::MODIFIER_CONTROL);
    CHECK(edit->cursor_position == 6u);
    edit->key(tokotachi::Key::RIGHT, tokotachi::MODIFIER_CONTROL | tokotachi::MODIFIER_SHIFT);
    CHECK(edit->selected_text == "world ");
    edit->key(tokotachi::Key::END);
    edit->key(tokotachi::Key::BACKSPACE, tokotachi::MODIFIER_CONTROL);  // 単語の始めまで消す
    CHECK(edit->text == "hello world ");
    edit->key(tokotachi::Key::HOME);
    edit->key(tokotachi::Key::DEL, tokotachi::MODIFIER_CONTROL);  // 次の単語の始めまで消す
    CHECK(edit->text == "world ");
}

TEST(line_edit_rtl_text)
{
    tokotachi::Window window("t", 300, 100);
    TestEdit* edit = new TestEdit(&window, "abc ");
    edit->bounds = tokotachi::Rect(0, 0, 200, 30);
    edit->focused = true;
    edit->type("שלום");  // 右から左の文字も入力できる
    CHECK(edit->text == "abc שלום");
    edit->key(tokotachi::Key::LEFT);  // 移動は論理の順
    CHECK(edit->cursor_position == 10u);
    edit->key(tokotachi::Key::BACKSPACE);  // 論理の直前の文字(ו)を消す
    CHECK(edit->text == "abc שלם");
}

TEST(label_word_wrap)
{
    tokotachi::Window window("t", 400, 300);
    tokotachi::BoxLayout* column = new tokotachi::BoxLayout(&window);
    column->margins = tokotachi::Margins(0);
    tokotachi::Label* label = new tokotachi::Label(&window, "折り返しのあるラベルは、幅に応じて高さが変わる。Wrapping labels change their height.");
    label->word_wrap = true;
    column->add(label);
    column->add_stretch();
    window.apply_layout();
    const float wide_height = label->height;
    window.width = 120;  // 幅を狭めると行が増え、高さが増える
    window.visible = true;
    window.apply_layout();
    CHECK(label->height > wide_height);
    const tokotachi::Size min = label->minimum_hint;
    CHECK(min.width < 120);  // 最小の幅は最も長い単語の幅
}

// 保護された仮想関数・状態を確かめるための TextEdit。
struct TestMemo : tokotachi::TextEdit
{
    explicit TestMemo(tokotachi::Widget* parent_widget, const std::string& s = std::string()) : tokotachi::TextEdit(parent_widget, s) {}

    void key(tokotachi::Key k, tokotachi::Modifiers m = tokotachi::MODIFIER_NONE)
    {
        tokotachi::KeyEvent e(k, m, false, 0);
        key_down(e);
    }

    void type(const std::string& s)
    {
        tokotachi::TextInputEvent e(s);
        text_input(e);
    }

    bool wheel(float dy)
    {
        tokotachi::WheelEvent e(tokotachi::Point(10, 10), tokotachi::Point(0, dy), tokotachi::MODIFIER_NONE);
        mouse_wheel(e);
        return e.accepted;
    }

    tokotachi::Point scroll() const { return scroll_offset(); }
    std::size_t lines() const { return text_layout().line_count(); }
    tokotachi::Rect area() const { return text_area(); }
};

TEST(text_edit_newlines_and_navigation)
{
    tokotachi::Window window("t", 300, 200);
    TestMemo* memo = new TestMemo(&window);
    memo->bounds = tokotachi::Rect(0, 0, 240, 120);
    memo->focused = true;
    memo->type("ab");
    memo->key(tokotachi::Key::RETURN);  // 改行を入れる
    memo->type("cd");
    CHECK(memo->text == "ab\ncd");
    memo->key(tokotachi::Key::UP);  // 上の行へ(横の位置を保つ)
    CHECK(memo->cursor_position.get() <= 2u);
    memo->key(tokotachi::Key::END);  // 行の終わり
    CHECK(memo->cursor_position == 2u);
    memo->key(tokotachi::Key::DOWN);
    memo->key(tokotachi::Key::HOME);  // 行の始め
    CHECK(memo->cursor_position == 3u);
    memo->key(tokotachi::Key::END, tokotachi::MODIFIER_CONTROL);  // 全体の終わり
    CHECK(memo->cursor_position == 5u);
    memo->key(tokotachi::Key::HOME, tokotachi::MODIFIER_CONTROL | tokotachi::MODIFIER_SHIFT);  // 全体を選ぶ
    CHECK(memo->selected_text == "ab\ncd");
}

TEST(text_edit_normalizes_text)
{
    tokotachi::Window window("t", 300, 200);
    tokotachi::TextEdit* memo = new tokotachi::TextEdit(&window);
    memo->insert("a\r\nb\rc\td");  // 改行は \n にそろえ、タブは空白に
    CHECK(memo->text == "a\nb\nc d");
}

TEST(text_edit_scrolls_to_caret)
{
    tokotachi::Window window("t", 300, 300);
    TestMemo* memo = new TestMemo(&window);
    memo->bounds = tokotachi::Rect(0, 0, 200, 80);
    memo->focused = true;
    for (int i = 0; i < 20; ++i)
    {
        memo->type("line\n");
    }
    window.apply_layout();
    CHECK(memo->scroll().y > 0);  // キャレット(最後の行)が見えるようにスクロールする
    memo->key(tokotachi::Key::HOME, tokotachi::MODIFIER_CONTROL);
    CHECK(memo->scroll().y == 0);
    CHECK(memo->wheel(-1));  // ホイールを手前に回すと下へ
    CHECK(memo->scroll().y > 0);
    memo->key(tokotachi::Key::END, tokotachi::MODIFIER_CONTROL);
    const float bottom = memo->scroll().y;
    memo->wheel(-100);  // 範囲の外へは動かない
    CHECK(memo->scroll().y == bottom);
}

TEST(text_edit_word_wrap)
{
    tokotachi::Window window("t", 300, 300);
    TestMemo* memo = new TestMemo(&window, "a long line of text that does not fit in a narrow text edit");
    memo->bounds = tokotachi::Rect(0, 0, 120, 200);
    CHECK(memo->lines() > 1);
    memo->word_wrap = false;
    CHECK(memo->lines() == 1u);
    CHECK(!memo->wheel(-1));  // 縦にスクロールできなければ受け取らない(親へ送る)
}

TEST(scroll_bar_range)
{
    tokotachi::Window window("t", 300, 300);
    tokotachi::ScrollBar* bar = new tokotachi::ScrollBar(&window);
    int changes = 0;
    bar->on_change = [&](tokotachi::Object*) { ++changes; };
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

struct TestScroll : tokotachi::ScrollArea
{
    explicit TestScroll(tokotachi::Widget* parent_widget) : tokotachi::ScrollArea(parent_widget) {}
    bool wheel(float dy)
    {
        tokotachi::WheelEvent e(tokotachi::Point(10, 10), tokotachi::Point(0, dy), tokotachi::MODIFIER_NONE);
        mouse_wheel(e);
        return e.accepted;
    }
};

TEST(scroll_area_scrolls_content)
{
    tokotachi::Window window("t", 300, 300);
    TestScroll* area = new TestScroll(&window);
    area->bounds = tokotachi::Rect(0, 0, 200, 100);
    Fixed* content = new Fixed(nullptr, 150, 500);
    area->content = content;  // 中身はスクロール領域の中に移る
    CHECK(content->parent != nullptr);
    window.apply_layout();
    // 縦にだけスクロールし、中身の幅は見えている幅(スクロールバーの分だけ狭い)に合わせる。
    CHECK(near(content->width, 200 - tokotachi::ScrollBar::THICKNESS));
    CHECK(near(content->height, 500));
    area->scroll_position = tokotachi::Point(0, 50);
    CHECK(near(content->y, -50));
    CHECK(area->wheel(-1));
    CHECK(content->y < -50.0f);
    area->scroll_to_visible(tokotachi::Rect(0, 490, 10, 10));  // 最後まで
    CHECK(near(area->scroll_position.get().y, 400));
    area->scroll_position = tokotachi::Point(0, 10000);  // 範囲に丸める
    CHECK(near(area->scroll_position.get().y, 400));

    content->set_preferred(150, 50);  // 中身が収まると、スクロールバーを消して位置を 0 にする
    window.apply_layout();
    CHECK(near(area->scroll_position.get().y, 0));
    CHECK(near(content->width, 200));
    CHECK(!area->wheel(-1));  // スクロールできなければ受け取らない
}

TEST(wheel_goes_to_widget_under_pointer_then_parents)
{
    tokotachi::Window window("t", 300, 300);
    window.visible = true;
    tokotachi::ScrollArea* area = new tokotachi::ScrollArea(&window);
    area->bounds = tokotachi::Rect(0, 0, 200, 100);
    tokotachi::Widget* content = new tokotachi::Widget();
    content->minimum_size = tokotachi::Size(100, 400);
    tokotachi::Label* label = new tokotachi::Label(content, "ラベル");
    label->bounds = tokotachi::Rect(0, 0, 100, 20);
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
void dispatch_key(tokotachi::Window& window, SDL_Keycode key, bool down = true, SDL_Keymod mod = SDL_KMOD_NONE)
{
    SDL_Event e;
    SDL_zero(e);
    e.type = down ? SDL_EVENT_KEY_DOWN : SDL_EVENT_KEY_UP;
    e.key.windowID = SDL_GetWindowID(window.impl()->window);
    e.key.key = key;
    e.key.mod = mod;
    e.key.down = down;
    tokotachi::Application::instance()->impl()->dispatch(e);
}

void dispatch_click(tokotachi::Window& window, const tokotachi::Point& p)
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
        tokotachi::Application::instance()->impl()->dispatch(e);
    }
}

TEST(popup_opens_relative_to_owner)
{
    tokotachi::Window window("t", 300, 200);
    window.visible = true;
    tokotachi::Button* button = new tokotachi::Button(&window, "b");
    button->bounds = tokotachi::Rect(20, 30, 80, 24);
    tokotachi::Popup* popup = new tokotachi::Popup(button);
    int closed = 0;
    popup->on_closed = [&](tokotachi::Object*) { ++closed; };
    popup->open(button->local_rect, tokotachi::Size(120, 60));
    CHECK(popup->visible);
    CHECK(popup->impl()->window != nullptr);
    CHECK(near(popup->width, 120) && near(popup->height, 60));
    CHECK(tokotachi::Application::instance()->impl()->open_menus.size() == 1);
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

    dispatch_click(window, tokotachi::Point(250, 150));  // 外側を押すと閉じる(押したことは捨てる)
    CHECK(!popup->visible);
    CHECK(closed == 1);
    CHECK(tokotachi::Application::instance()->impl()->open_menus.empty());
    delete button;  // 持ち主と一緒に破棄される(ぶら下がらない)
}

TEST(combo_box_selects_with_keys_and_popup)
{
    tokotachi::Window window("t", 300, 300);
    window.visible = true;
    tokotachi::ComboBox* combo = new tokotachi::ComboBox(&window);
    combo->bounds = tokotachi::Rect(10, 10, 150, 30);
    std::vector<std::string> items;
    items.push_back("小");
    items.push_back("中");
    items.push_back("大");
    combo->items = items;
    CHECK(combo->current_index == -1);
    int changes = 0;
    combo->on_change = [&](tokotachi::Object*) { ++changes; };
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
    tokotachi::Window window("t", 400, 300);
    window.visible = true;
    tokotachi::Widget* area = new tokotachi::Widget(&window);
    area->bounds = tokotachi::Rect(0, 0, 400, 300);
    tokotachi::Menu* menu = new tokotachi::Menu(&window);
    std::string chosen;
    menu->add_item("切り取り", [&](tokotachi::Object*) { chosen = "cut"; });
    tokotachi::Action* copy = menu->add_item("コピー", [&](tokotachi::Object* sender) {
        chosen = static_cast<tokotachi::Action*>(sender)->text;
    });
    copy->shortcut = "Ctrl+C";
    menu->add_separator();
    tokotachi::Action* disabled = menu->add_item("無効", [&](tokotachi::Object*) { chosen = "disabled"; });
    disabled->enabled = false;
    tokotachi::Menu* sort = menu->add_submenu("並べ替え");
    sort->add_item("名前順", [&](tokotachi::Object*) { chosen = "name"; });
    sort->add_item("日付順", [&](tokotachi::Object*) { chosen = "date"; });

    menu->popup(area, tokotachi::Point(50, 40));
    CHECK(menu->is_open());
    dispatch_key(window, SDLK_DOWN);
    dispatch_key(window, SDLK_DOWN);
    dispatch_key(window, SDLK_RETURN);  // 2 つ目(コピー)を選ぶ
    CHECK(chosen == "コピー");
    CHECK(!menu->is_open());  // 選ぶとすべて閉じる

    menu->popup(area, tokotachi::Point(50, 40));
    dispatch_key(window, SDLK_UP);    // 何も強調していなければ、↑ は最後の項目(並べ替え)
    dispatch_key(window, SDLK_UP);    // 無効な項目・区切りを飛ばしてコピー
    dispatch_key(window, SDLK_DOWN);  // 区切り・無効な項目を飛ばして並べ替えへ戻る
    dispatch_key(window, SDLK_RIGHT);  // サブメニューを開き、最初の項目を強調する
    CHECK(sort->is_open());
    dispatch_key(window, SDLK_DOWN);
    dispatch_key(window, SDLK_RETURN);
    CHECK(chosen == "date");
    CHECK(!sort->is_open() && !menu->is_open());

    menu->popup(area, tokotachi::Point(50, 40));
    dispatch_key(window, SDLK_UP);
    dispatch_key(window, SDLK_RIGHT);
    dispatch_key(window, SDLK_LEFT);  // サブメニューだけ閉じる
    CHECK(!sort->is_open() && menu->is_open());
    dispatch_key(window, SDLK_ESCAPE);
    CHECK(!menu->is_open());

    menu->popup(area, tokotachi::Point(50, 40));
    dispatch_click(window, tokotachi::Point(350, 250));  // 外側を押すと閉じる
    CHECK(!menu->is_open());
    CHECK(tokotachi::Application::instance()->impl()->open_menus.empty());
}

TEST(tooltip_property)
{
    tokotachi::Window window("t", 300, 200);
    tokotachi::Button* button = new tokotachi::Button(&window, "b");
    button->tooltip = "説明";
    CHECK(button->tooltip == "説明");
}

TEST(window_close_hides)
{
    tokotachi::Window window("t", 100, 100);
    window.visible = true;
    window.close();  // close() は on_close を呼ばずに閉じる
    CHECK(!window.visible);
}

TEST(window_capture)
{
    tokotachi::Window window("t", 120, 60);
    window.background = tokotachi::Color(0x10, 0x20, 0x30);
    window.visible = true;
    tokotachi::Bitmap b = window.capture();
    CHECK(b.width() == window.impl()->pixel_width && b.height() == window.impl()->pixel_height);
    CHECK(b.width() >= 120 && b.height() >= 60);
    // 背景の色(不透明)。描画先の画素と同じ
    const tokotachi::Color c = b.pixel(5, 5);
    CHECK(c.r == 0x10 && c.g == 0x20 && c.b == 0x30 && c.a == 255);
    CHECK((b.pixels()[0] | 0xff000000u) == (window.impl()->buffer[0] | 0xff000000u));
}

TEST(save_png)
{
    tokotachi::Window window("t", 120, 60);
    new tokotachi::Label(&window, "テスト");
    window.visible = true;
    CHECK(window.save_png("tokotachi_test_save.png"));
    std::FILE* f = std::fopen("tokotachi_test_save.png", "rb");
    CHECK(f != nullptr);
    if (f)
    {
        std::fseek(f, 0, SEEK_END);
        CHECK(std::ftell(f) > 100);
        std::fclose(f);
        std::remove("tokotachi_test_save.png");
    }
}

void dispatch_text(tokotachi::Window& window, const char* text)
{
    SDL_Event e;
    SDL_zero(e);
    e.type = SDL_EVENT_TEXT_INPUT;
    e.text.windowID = SDL_GetWindowID(window.impl()->window);
    e.text.text = text;
    tokotachi::Application::instance()->impl()->dispatch(e);
}

TEST(range_base_clamps_and_moves_bounds)
{
    tokotachi::Window window("t", 300, 100);
    tokotachi::Slider* slider = new tokotachi::Slider(&window);
    int changes = 0;
    slider->on_change = [&](tokotachi::Object*) { ++changes; };
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

    tokotachi::DoubleVariable* level = new tokotachi::DoubleVariable(&window, 0);
    tokotachi::bind(slider->value, level->value);
    level->value = 75;
    CHECK(slider->value == 75.0);
}

TEST(slider_keys_and_mouse)
{
    tokotachi::Window window("t", 300, 100);
    window.visible = true;
    tokotachi::Slider* slider = new tokotachi::Slider(&window);
    slider->bounds = tokotachi::Rect(10, 10, 216, 24);  // つまみの中心が動ける長さは 200
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
    dispatch_click(window, tokotachi::Point(10 + 8 + 100.4f, 22));
    CHECK(slider->value == 50.0);
    // 代入した値は合わせないが、操作した値は single_step に合わせる。
    slider->single_step = 10;
    slider->value = 33;
    CHECK(slider->value == 33.0);
    dispatch_key(window, SDLK_RIGHT);
    CHECK(slider->value == 40.0);

    // 縦は下が minimum。
    slider->orientation = tokotachi::Orientation::VERTICAL;
    slider->bounds = tokotachi::Rect(10, 10, 24, 216);
    window.apply_layout();
    slider->single_step = 1;
    dispatch_click(window, tokotachi::Point(22, 10 + 8 + 50));  // 上から 1/4 の位置
    CHECK(slider->value == 75.0);
}

TEST(spin_box_commit_and_steps)
{
    tokotachi::Window window("t", 300, 100);
    window.visible = true;
    tokotachi::SpinBox* spin = new tokotachi::SpinBox(&window);
    spin->bounds = tokotachi::Rect(10, 10, 120, 32);
    spin->maximum = 10;
    spin->decimals = 1;
    spin->value = 2.5;
    window.apply_layout();
    CHECK(spin->text == "2.5");
    int changes = 0;
    spin->on_change = [&](tokotachi::Object*) { ++changes; };

    // 入力欄を押し、打ち込んで Enter で決める(桁数に丸める)。
    dispatch_click(window, tokotachi::Point(30, 26));
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
    dispatch_click(window, tokotachi::Point(10 + 120 - 10, 14));
    CHECK(spin->value == 10.0);
    dispatch_click(window, tokotachi::Point(10 + 120 - 10, 38));
    CHECK(spin->value == 9.0);

    spin->decimals = 0;
    CHECK(spin->text == "9");
}

TEST(progress_bar_range)
{
    tokotachi::Window window("t", 300, 100);
    tokotachi::ProgressBar* progress = new tokotachi::ProgressBar(&window);
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
std::uint32_t packed(const tokotachi::Color& c)
{
    return (std::uint32_t(c.a) << 24) | (std::uint32_t(c.r) << 16) | (std::uint32_t(c.g) << 8) | c.b;
}

TEST(theme_color_resolves_role_or_fixed)
{
    tokotachi::Application* app = tokotachi::Application::instance();
    app->color_scheme = tokotachi::ColorScheme::LIGHT;
    const tokotachi::ThemeColor role(tokotachi::ColorRole::ACCENT);
    const tokotachi::ThemeColor fixed(tokotachi::Color::rgb(0x123456));
    CHECK(role.is_role() && role.role() == tokotachi::ColorRole::ACCENT);
    CHECK(!fixed.is_role() && fixed.color() == tokotachi::Color::rgb(0x123456));
    CHECK(role.resolve() == tokotachi::Theme::light()[tokotachi::ColorRole::ACCENT]);
    CHECK(role != fixed);
    CHECK(tokotachi::ThemeColor(tokotachi::ColorRole::TEXT) == tokotachi::ThemeColor());
    app->color_scheme = tokotachi::ColorScheme::DARK;
    CHECK(role.resolve() == tokotachi::Theme::dark()[tokotachi::ColorRole::ACCENT]);
    CHECK(fixed.resolve() == tokotachi::Color::rgb(0x123456));  // 固定の色はテーマで変わらない
    app->color_scheme = tokotachi::ColorScheme::SYSTEM;
}

TEST(theme_switch_notifies_and_repaints)
{
    tokotachi::Application* app = tokotachi::Application::instance();
    app->color_scheme = tokotachi::ColorScheme::LIGHT;
    int changes = 0;
    int dark_changes = 0;
    app->on_theme_change = [&](tokotachi::Object* sender)
    {
        CHECK(sender == app);
        ++changes;
    };
    tokotachi::BoolVariable* dark = new tokotachi::BoolVariable(nullptr, false);
    tokotachi::bind(dark->value, app->dark);
    dark->on_change = [&](tokotachi::Object*) { ++dark_changes; };

    tokotachi::Window window("t", 120, 60);
    tokotachi::Label* label = new tokotachi::Label(&window, "テスト");
    window.visible = true;
    window.impl()->render(false);
    CHECK(window.background.get() == tokotachi::ThemeColor(tokotachi::ColorRole::WINDOW));
    CHECK(window.impl()->buffer[0] == packed(tokotachi::Theme::light()[tokotachi::ColorRole::WINDOW]));

    app->color_scheme = tokotachi::ColorScheme::DARK;
    CHECK(app->dark);
    CHECK(app->theme.get() == tokotachi::Theme::dark());
    CHECK(changes == 1);
    CHECK(dark_changes == 1 && dark->value);
    CHECK(!window.impl()->dirty.empty());  // 描き直しを依頼済み
    window.impl()->render(false);
    CHECK(window.impl()->buffer[0] == packed(tokotachi::Theme::dark()[tokotachi::ColorRole::WINDOW]));
    CHECK(label->color.get().resolve() == tokotachi::Theme::dark()[tokotachi::ColorRole::TEXT]);

    app->color_scheme = tokotachi::ColorScheme::DARK;  // 同じ値: 知らせない
    CHECK(changes == 1);

    // 使っていない方のテーマを変えても、使うテーマは変わらない。使っている方を変えると変わる。
    tokotachi::Theme custom = tokotachi::Theme::light();
    custom[tokotachi::ColorRole::ACCENT] = tokotachi::Color::rgb(0x059669);
    app->light_theme = custom;
    CHECK(changes == 1);
    app->color_scheme = tokotachi::ColorScheme::LIGHT;
    CHECK(changes == 2 && !app->dark);
    CHECK(tokotachi::ThemeColor(tokotachi::ColorRole::ACCENT).resolve() == tokotachi::Color::rgb(0x059669));

    app->on_theme_change = tokotachi::NotifyHandler();
    delete dark;
    app->light_theme = tokotachi::Theme::light();
    app->color_scheme = tokotachi::ColorScheme::SYSTEM;
}

// 望む大きさの計算の中で、今のテーマの ACCENT が赤(0xdc2626)かを幅にする。
struct ThemeProbe : tokotachi::Widget
{
    explicit ThemeProbe(tokotachi::Widget* parent) : tokotachi::Widget(parent) {}
    tokotachi::Size compute_preferred_size() const override
    {
        const bool red = tokotachi::ThemeColor(tokotachi::ColorRole::ACCENT).resolve() == tokotachi::Color::rgb(0xdc2626);
        return tokotachi::Size(red ? 1.0f : 0.0f, 0);
    }
};

TEST(theme_override_inherits_to_descendants)
{
    // ウィジェットごとのテーマの上書き: 自分と子孫に効き、上書きしていない役割は親から継ぐ(docs/adr/0042)。
    tokotachi::Application* app = tokotachi::Application::instance();
    app->color_scheme = tokotachi::ColorScheme::LIGHT;
    const tokotachi::Color red = tokotachi::Color::rgb(0xdc2626);
    const tokotachi::Color dark_red = tokotachi::Color::rgb(0x7f1d1d);
    const tokotachi::Color pale = tokotachi::Color::rgb(0xfef2f2);

    tokotachi::Window window("t", 200, 100);
    tokotachi::Widget* panel = new tokotachi::Widget(&window);
    panel->bounds = tokotachi::Rect(0, 0, 100, 100);
    tokotachi::Widget* inner = new tokotachi::Widget(panel);
    inner->bounds = tokotachi::Rect(10, 10, 20, 20);
    tokotachi::Widget* outside = new tokotachi::Widget(&window);
    outside->bounds = tokotachi::Rect(150, 10, 20, 20);
    // 描くときに読む「今のテーマ」の色を記録する
    tokotachi::Color painted_inner, painted_outside;
    inner->on_paint = [&](tokotachi::Object*, tokotachi::Canvas& c)
    {
        painted_inner = tokotachi::ThemeColor(tokotachi::ColorRole::ACCENT).resolve();
        c.fill_rect(tokotachi::Rect(0, 0, 20, 20), painted_inner);
    };
    outside->on_paint = [&](tokotachi::Object*, tokotachi::Canvas&)
    {
        painted_outside = tokotachi::ThemeColor(tokotachi::ColorRole::ACCENT).resolve();
    };

    tokotachi::ThemeOverride accent;
    CHECK(accent.empty() && !accent.has(tokotachi::ColorRole::ACCENT));
    accent.set(tokotachi::ColorRole::ACCENT, red, dark_red);
    CHECK(!accent.empty() && accent.light(tokotachi::ColorRole::ACCENT) == red && accent.dark(tokotachi::ColorRole::ACCENT) == dark_red);
    panel->theme_override = accent;
    CHECK(panel->theme_override.get() == accent);
    CHECK(panel->theme.get()[tokotachi::ColorRole::ACCENT] == red);
    CHECK(inner->theme.get()[tokotachi::ColorRole::ACCENT] == red);  // 子孫は継ぐ
    CHECK(outside->theme.get()[tokotachi::ColorRole::ACCENT] == tokotachi::Theme::light()[tokotachi::ColorRole::ACCENT]);
    CHECK(inner->theme.get()[tokotachi::ColorRole::WINDOW] == tokotachi::Theme::light()[tokotachi::ColorRole::WINDOW]);

    window.visible = true;
    window.impl()->render(false);
    CHECK(painted_inner == red && painted_outside == tokotachi::Theme::light()[tokotachi::ColorRole::ACCENT]);
    // 描き終えたら、今のテーマはアプリケーションのものに戻る
    CHECK(tokotachi::ThemeColor(tokotachi::ColorRole::ACCENT).resolve() == tokotachi::Theme::light()[tokotachi::ColorRole::ACCENT]);
    const float scale = window.display_scale;
    const int px = static_cast<int>(15 * scale), stride = static_cast<int>(200 * scale);
    CHECK(window.impl()->buffer[static_cast<std::size_t>(px * stride + px)] == packed(red));

    // 子で別の役割を上書きすると、親の上書きと合わさる。ダークにすると、ダークの色を使う。
    tokotachi::ThemeOverride field;
    field.set(tokotachi::ColorRole::FIELD, pale);
    field.set_field_padding(20);
    inner->theme_override = field;
    CHECK(inner->theme.get()[tokotachi::ColorRole::ACCENT] == red && inner->theme.get()[tokotachi::ColorRole::FIELD] == pale);
    CHECK(inner->theme.get().field_padding == 20 && panel->theme.get().field_padding != 20);
    app->color_scheme = tokotachi::ColorScheme::DARK;
    CHECK(inner->theme.get()[tokotachi::ColorRole::ACCENT] == dark_red && inner->theme.get()[tokotachi::ColorRole::FIELD] == pale);
    CHECK(inner->theme.get()[tokotachi::ColorRole::WINDOW] == tokotachi::Theme::dark()[tokotachi::ColorRole::WINDOW]);
    window.impl()->render(false);
    CHECK(painted_inner == dark_red);
    app->color_scheme = tokotachi::ColorScheme::LIGHT;

    // 望む大きさを計算するときも、自分に効いているテーマを読む。
    ThemeProbe* probe = new ThemeProbe(panel);
    CHECK(near(probe->preferred_size.get().width, 1));  // ACCENT が red(上書き)なら 1
    probe->parent = &window;
    CHECK(near(probe->preferred_size.get().width, 0));

    // ポップアップは、開いた元のウィジェットのテーマを継ぐ。
    tokotachi::Popup* popup = new tokotachi::Popup(inner);
    CHECK(popup->theme.get()[tokotachi::ColorRole::ACCENT] == red && popup->theme.get()[tokotachi::ColorRole::FIELD] == pale);
    delete popup;

    // 上書きをやめると、親のものに戻る。
    accent.unset(tokotachi::ColorRole::ACCENT);
    CHECK(accent.empty());
    panel->theme_override = accent;
    CHECK(inner->theme.get()[tokotachi::ColorRole::ACCENT] == tokotachi::Theme::light()[tokotachi::ColorRole::ACCENT]);
    app->color_scheme = tokotachi::ColorScheme::SYSTEM;
}

TEST(tab_widget_add_insert_remove)
{
    tokotachi::Window window("t", 300, 200);
    tokotachi::TabWidget* tabs = new tokotachi::TabWidget(&window);
    window.visible = true;
    CHECK(tabs->current_index == -1 && tabs->current_tab == nullptr);
    int changes = 0;
    tabs->on_change = [&](tokotachi::Object* sender)
    {
        CHECK(sender == tabs);
        ++changes;
    };

    tokotachi::Widget* a = new tokotachi::Widget();
    tokotachi::Widget* b = new tokotachi::Widget();
    tokotachi::Widget* c = new tokotachi::Widget();
    tokotachi::Tab* ta = tabs->add_tab(a, "A");
    CHECK(ta && ta->page == a && ta->title.get() == "A" && a->parent == tabs);
    CHECK(tabs->current_index == 0 && tabs->current_tab == ta && a->showing && changes == 1);
    tokotachi::Tab* tb = tabs->add_tab(b, "B");
    CHECK(tabs->current_index == 0 && !b->showing && b->visible && changes == 1);  // 2 つ目以降は選ばない(visible は変えない)
    CHECK(tabs->add_tab(b, "B2") == tb && tabs->tabs.get().size() == 2);  // 同じページは加えない
    tokotachi::Tab* tc = tabs->insert_tab(0, c, "C");
    CHECK(tabs->tabs.get().size() == 3 && tabs->tabs.get()[0] == tc);
    CHECK(tabs->current_tab == ta && tabs->current_index == 1 && changes == 2);  // 選んでいるタブは同じで、番号がずれる

    tabs->current_index = 2;
    CHECK(tabs->current_tab == tb && b->showing && !a->showing && !c->showing && changes == 3);
    tabs->current_index = 5;  // 範囲の外は無視する
    tabs->current_index = -1;
    CHECK(tabs->current_index == 2 && changes == 3);

    // 外したページは親が無くなり、見える状態で返る。選んでいたタブなら隣を選ぶ。
    CHECK(tabs->remove_tab(tb) == b);
    CHECK(b->parent == nullptr && b->visible && tabs->tabs.get().size() == 2);
    CHECK(tabs->current_tab == ta && a->showing && changes == 4);
    delete b;

    // ページを破棄する・別の親へ移すと、そのタブも消える。
    delete a;
    CHECK(tabs->tabs.get().size() == 1 && tabs->current_tab == tc && c->showing && changes == 5);
    c->parent = &window;
    CHECK(tabs->tabs.get().empty() && tabs->current_index == -1 && tabs->current_tab == nullptr && changes == 6);
    CHECK(c->showing);

    // 別の TabWidget へ加えると、元のタブは消える。
    tokotachi::TabWidget* other = new tokotachi::TabWidget(&window);
    tabs->add_tab(c, "C");
    other->add_tab(c, "C");
    CHECK(tabs->tabs.get().empty() && other->tabs.get().size() == 1 && c->parent == other);
    CHECK(c->showing);
    CHECK(tabs->insert_tab(0, tabs, "self") == nullptr);

    // 利用者が隠したページは、選んでも見せない(TabWidget は visible を変えない)。
    tokotachi::Widget* d = new tokotachi::Widget();
    d->visible = false;
    other->add_tab(d, "D");
    other->current_index = 1;
    CHECK(other->current_index == 1 && !d->visible && !d->showing && !c->showing);
    d->visible = true;
    CHECK(d->showing);
    tabs->on_change = tokotachi::NotifyHandler();
}

TEST(tab_widget_keys_and_clicks)
{
    tokotachi::Window window("t", 400, 200);
    window.visible = true;
    tokotachi::TabWidget* tabs = new tokotachi::TabWidget(&window);
    tabs->bounds = tokotachi::Rect(0, 0, 400, 200);
    tokotachi::LineEdit* edit = new tokotachi::LineEdit();
    tabs->add_tab(edit, "1");
    tokotachi::Tab* second = tabs->add_tab(new tokotachi::Widget(), "2");
    tabs->add_tab(new tokotachi::Widget(), "3");
    second->enabled = false;
    window.apply_layout();
    CHECK(edit->bounds.get().y >= 20 && edit->bounds.get().width == 400);  // 見出しの下に並べる

    // 見出しは最小の幅(48dp)で左から並ぶ。押すとフォーカスが移る。
    dispatch_click(window, tokotachi::Point(120, 10));
    CHECK(tabs->current_index == 2 && tabs->focused);
    dispatch_click(window, tokotachi::Point(72, 10));  // 無効なタブは選べない
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
    tokotachi::Window window("t", 400, 200);
    window.visible = true;
    tokotachi::TabWidget* tabs = new tokotachi::TabWidget(&window);
    tabs->bounds = tokotachi::Rect(0, 0, 150, 120);
    for (int i = 0; i < 6; ++i)
    {
        tabs->add_tab(new tokotachi::Widget(), std::string(1, char('1' + i)));
    }
    window.apply_layout();
    CHECK(tabs->minimum_hint.get().width >= 48 + 24 * 2);  // 見出し 1 つと ◀▶

    // 見出しは 6 × 48 = 288dp、見せられる幅は 150 - 48 = 102dp。最後を選ぶと、右端が見えるまでスクロールする。
    tabs->current_index = 5;
    dispatch_click(window, tokotachi::Point(50, 10));  // 50 + 186 = 236 → 5 つ目(192〜240)
    CHECK(tabs->current_index == 4);
    dispatch_click(window, tokotachi::Point(114, 10));  // ◀ で 80dp 戻す
    dispatch_click(window, tokotachi::Point(10, 10));   // 10 + 106 = 116 → 3 つ目(96〜144)
    CHECK(tabs->current_index == 2);
    dispatch_click(window, tokotachi::Point(138, 10));  // ▶ では選ぶタブは変わらない
    CHECK(tabs->current_index == 2);
}

// マウスのボタン・移動のイベント(位置は dp)。修飾キーは SDL_SetModState で決める。
void dispatch_mouse(tokotachi::Window& window, SDL_EventType type, const tokotachi::Point& p, int clicks = 1)
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
    tokotachi::Application::instance()->impl()->dispatch(e);
}

void click_with(tokotachi::Window& window, const tokotachi::Point& p, SDL_Keymod mod, int clicks = 1)
{
    SDL_SetModState(mod);
    dispatch_mouse(window, SDL_EVENT_MOUSE_BUTTON_DOWN, p, clicks);
    dispatch_mouse(window, SDL_EVENT_MOUSE_BUTTON_UP, p, clicks);
    SDL_SetModState(SDL_KMOD_NONE);
}

// ItemView の行の高さ(倍率 1)と、行 r の中ほど(枠の 1dp と見出しの高さを足す)。
float view_row_height() { return std::max(std::ceil(tokotachi::TextLayout("Ag", tokotachi::Font(), 1).height()), 16.0f) + 8; }

tokotachi::Point row_point(int r, float x = 100, float header = 0)
{
    const float h = view_row_height();
    return tokotachi::Point(x, 1 + header + r * h + h / 2);
}

TEST(image_from_pixels_and_draw)
{
    std::vector<tokotachi::Color> pixels(4, tokotachi::Color(255, 0, 0, 255));
    pixels[3] = tokotachi::Color(0, 0, 255, 128);
    const tokotachi::Image image = tokotachi::Image::from_pixels(2, 2, pixels);
    CHECK(!image.is_null() && image.width() == 2 && image.height() == 2);
    CHECK(image.pixel(0, 0) == tokotachi::Color(255, 0, 0, 255));
    CHECK(image.pixel(1, 1).a == 128 && image.pixel(1, 1).b >= 254);  // アルファ乗算を戻す
    CHECK(image.pixel(5, 5).a == 0);
    CHECK(tokotachi::Image::from_pixels(3, 3, pixels).is_null());  // 数が合わない
    CHECK(tokotachi::Image::load("no-such-file.png").is_null());

    // 描くと、伸縮して矩形を塗る。
    tokotachi::Window window("t", 40, 40);
    tokotachi::Widget* w = new tokotachi::Widget(&window);
    w->bounds = tokotachi::Rect(0, 0, 40, 40);
    w->on_paint = [=](tokotachi::Object*, tokotachi::Canvas& canvas) { canvas.draw_image(image, tokotachi::Rect(0, 0, 20, 20)); };
    window.visible = true;
    window.impl()->render(false);
    const int stride = window.impl()->pixel_width;
    CHECK(window.impl()->buffer[2 * stride + 2] == 0xffff0000u);
}

// OS からのドロップ(SDL_EVENT_DROP_*。位置は dp)。
void dispatch_drop(tokotachi::Window& window, SDL_EventType type, const tokotachi::Point& p, const char* data = nullptr)
{
    const float k = window.impl()->scale / window.impl()->density;
    SDL_Event e;
    SDL_zero(e);
    e.type = type;
    e.drop.windowID = SDL_GetWindowID(window.impl()->window);
    e.drop.x = p.x * k;
    e.drop.y = p.y * k;
    e.drop.data = data;
    tokotachi::Application::instance()->impl()->dispatch(e);
}

TEST(drag_and_drop_in_window_and_from_outside)
{
    tokotachi::Window window("t", 300, 200);
    window.visible = true;
    tokotachi::Widget* source = new tokotachi::Widget(&window);
    source->bounds = tokotachi::Rect(0, 0, 100, 100);
    tokotachi::Widget* box = new tokotachi::Widget(&window);  // 受け取る(中の子の上でも、親へ送られて受け取る)
    box->bounds = tokotachi::Rect(150, 0, 150, 200);
    tokotachi::Widget* inner = new tokotachi::Widget(box);
    inner->bounds = tokotachi::Rect(10, 10, 50, 50);
    box->accepts_drops = true;

    // 押している間に始める。ボタンを押していなければ始められない
    tokotachi::DragData data;
    data.text = "りんご";
    data.formats["app/id"] = "7";
    CHECK(!source->start_drag(data));
    bool started = false;
    source->on_mouse_down = [&](tokotachi::Object*, tokotachi::MouseEvent&) {};
    source->on_mouse_move = [&](tokotachi::Object*, tokotachi::MouseEvent&)
    {
        if (!started)
        {
            started = source->start_drag(data, "りんご");
        }
    };
    std::vector<std::string> log;
    tokotachi::DropAction finished = tokotachi::DropAction::COPY;
    source->on_drag_finish = [&](tokotachi::Object*, tokotachi::DropAction a) { finished = a; };
    box->on_drag_move = [&](tokotachi::Object*, tokotachi::DragEvent& e)
    {
        log.push_back("move");
        CHECK(e.data.source() == source && !e.data.from_outside() && e.data.formats.at("app/id") == "7");
        e.accepted = e.pos.x < 100;  // 左の 100dp だけで受け取る
    };
    box->on_drag_leave = [&](tokotachi::Object*) { log.push_back("leave"); };
    box->on_drop = [&](tokotachi::Object*, tokotachi::DragEvent& e)
    {
        log.push_back("drop " + e.data.text);
        CHECK(e.action == tokotachi::DropAction::MOVE && e.pos.x == 20 && e.pos.y == 30);
    };
    dispatch_mouse(window, SDL_EVENT_MOUSE_BUTTON_DOWN, tokotachi::Point(10, 10));
    dispatch_mouse(window, SDL_EVENT_MOUSE_MOTION, tokotachi::Point(20, 10));
    CHECK(started && log.empty());
    dispatch_mouse(window, SDL_EVENT_MOUSE_MOTION, tokotachi::Point(170, 20));  // inner の上: box が受け取る
    dispatch_mouse(window, SDL_EVENT_MOUSE_MOTION, tokotachi::Point(280, 20));  // 受け取らない位置
    dispatch_mouse(window, SDL_EVENT_MOUSE_MOTION, tokotachi::Point(170, 30));
    window.impl()->render(false);                                              // 像を描ける
    dispatch_mouse(window, SDL_EVENT_MOUSE_BUTTON_UP, tokotachi::Point(170, 30));
    CHECK(log.size() == 6 && log[0] == "move" && log[1] == "move" && log[2] == "leave" && log[5] == "drop りんご");  // 離した位置でも聞き直す
    CHECK(finished == tokotachi::DropAction::MOVE);

    // Esc でやめると、受け取らなかったことになる
    started = false;
    log.clear();
    dispatch_mouse(window, SDL_EVENT_MOUSE_BUTTON_DOWN, tokotachi::Point(10, 10));
    dispatch_mouse(window, SDL_EVENT_MOUSE_MOTION, tokotachi::Point(170, 20));
    dispatch_key(window, SDLK_ESCAPE);
    CHECK(started && finished == tokotachi::DropAction::NONE && log.back() == "leave");
    dispatch_mouse(window, SDL_EVENT_MOUSE_BUTTON_UP, tokotachi::Point(170, 20));
    CHECK(log.back() == "leave");  // 落とさない

    // OS からのファイル: 重ねている間は中身が分からず、落としたときに入る
    std::vector<std::string> files;
    box->on_drag_move = [&](tokotachi::Object*, tokotachi::DragEvent& e)
    {
        CHECK(e.data.from_outside() && e.data.source() == nullptr && e.action == tokotachi::DropAction::COPY);
    };
    box->on_drop = [&](tokotachi::Object*, tokotachi::DragEvent& e) { files = e.data.files; };
    dispatch_drop(window, SDL_EVENT_DROP_BEGIN, tokotachi::Point());
    dispatch_drop(window, SDL_EVENT_DROP_POSITION, tokotachi::Point(200, 50));
    dispatch_drop(window, SDL_EVENT_DROP_FILE, tokotachi::Point(200, 50), "C:/a.txt");
    dispatch_drop(window, SDL_EVENT_DROP_FILE, tokotachi::Point(200, 50), "C:/b.txt");
    dispatch_drop(window, SDL_EVENT_DROP_COMPLETE, tokotachi::Point(200, 50));
    CHECK(files.size() == 2 && files[1] == "C:/b.txt");
    // 落とさずに離れた(中身が無い)ときは drop を送らない
    files.clear();
    dispatch_drop(window, SDL_EVENT_DROP_BEGIN, tokotachi::Point());
    dispatch_drop(window, SDL_EVENT_DROP_POSITION, tokotachi::Point(200, 50));
    dispatch_drop(window, SDL_EVENT_DROP_COMPLETE, tokotachi::Point(200, 50));
    CHECK(files.empty() && !window.impl()->drag.active);
}

TEST(list_view_selection_and_keys)
{
    tokotachi::Window window("t", 300, 300);
    window.visible = true;
    tokotachi::ListModel* list = new tokotachi::ListModel(&window);
    std::vector<tokotachi::Item*> items;
    for (int i = 0; i < 5; ++i)
    {
        items.push_back(list->add_item("項目 " + std::to_string(i)));
    }
    items[2]->enabled = false;
    tokotachi::ListView* view = new tokotachi::ListView(&window);
    view->bounds = tokotachi::Rect(0, 0, 200, 250);
    view->model = list;
    window.apply_layout();
    int changes = 0;
    view->on_selection_change = [&](tokotachi::Object* sender)
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
    view->selection_mode = tokotachi::SelectionMode::MULTI;
    click_with(window, row_point(3), SDL_KMOD_LCTRL);
    CHECK(view->selection.get().size() == 2);
    click_with(window, row_point(4), SDL_KMOD_LSHIFT);  // 起点は 3
    CHECK(view->selection.get().size() == 2 && view->is_selected(list->index_of(items[4])));
    click_with(window, row_point(0), SDL_KMOD_LSHIFT);
    const std::vector<tokotachi::ModelIndex> range = view->selection;
    CHECK(range.size() == 3 && range[0] == list->index_of(items[0]) && range[2] == list->index_of(items[3]));  // 表示の順
    dispatch_key(window, SDLK_A, true, SDL_KMOD_LCTRL);
    CHECK(view->selection.get().size() == 4);
    dispatch_key(window, SDLK_DOWN, true, SDL_KMOD_LSHIFT);  // current は 0 から 1 へ。起点 3 から 1 まで
    CHECK(view->selection.get().size() == 2 && view->current == list->index_of(items[1]));
    click_with(window, row_point(4), SDL_KMOD_NONE);
    CHECK(view->selection.get().size() == 1);

    // 行の無いところを押すと選択を外す。
    dispatch_click(window, tokotachi::Point(100, 240));
    CHECK(view->selection.get().empty());

    // ダブルクリック・Enter で on_activate。
    int activations = 0;
    view->on_activate = [&](tokotachi::Object*) { ++activations; };
    click_with(window, row_point(1), SDL_KMOD_NONE, 2);
    dispatch_key(window, SDLK_RETURN);
    CHECK(activations == 2);
    view->on_selection_change = tokotachi::NotifyHandler();
}

TEST(item_view_text_style_search_and_edit)
{
    tokotachi::Window window("t", 300, 300);
    window.visible = true;
    tokotachi::ListModel* list = new tokotachi::ListModel(&window);
    const char* const names[] = {"apple", "Banana", "blueberry", "cherry", "bread"};
    std::vector<tokotachi::Item*> items;
    for (int i = 0; i < 5; ++i)
    {
        items.push_back(list->add_item(names[i]));
    }
    tokotachi::ListView* view = new tokotachi::ListView(&window);
    view->bounds = tokotachi::Rect(0, 0, 200, 250);
    view->model = list;
    window.apply_layout();

    // 項目ごとの文字の色・フォント(既定はビューの標準)
    items[1]->text_color = tokotachi::ThemeColor(tokotachi::Color(220, 38, 38));
    items[1]->font = tokotachi::Font(14, true);
    CHECK(list->text_color(list->index_of(items[1]), 0) == tokotachi::ThemeColor(tokotachi::Color(220, 38, 38)));
    CHECK(list->font(list->index_of(items[1]), 0).bold);
    CHECK(list->text_color(list->index_of(items[0]), 0) == tokotachi::ThemeColor());
    window.impl()->render(false);  // 描ける

    // 押した文字で始まる項目へ(大文字・小文字は区別しない。続けて押すと前方一致)
    dispatch_click(window, row_point(0));
    dispatch_key(window, SDLK_B);
    CHECK(view->current == list->index_of(items[1]));
    dispatch_key(window, SDLK_L);  // "bl"
    CHECK(view->current == list->index_of(items[2]));
    SDL_Delay(1100);               // 間が空くと、新しい検索
    dispatch_key(window, SDLK_B);  // 同じ文字は、次の項目へ
    CHECK(view->current == list->index_of(items[4]));
    dispatch_key(window, SDLK_B);  // "bb" も次へ(終わりから始めへ戻る)
    CHECK(view->current == list->index_of(items[1]) && view->is_selected(list->index_of(items[1])));

    // F2 で編集し、Enter で確定する(モデルの set_text)。編集できない項目では始めない
    dispatch_key(window, SDLK_F2);
    CHECK(!view->is_editing());
    items[1]->editable = true;
    SDL_Delay(1100);
    dispatch_key(window, SDLK_F2);
    CHECK(view->is_editing());
    dispatch_key(window, SDLK_END);  // 編集の欄が受け取り、current は動かない
    dispatch_text(window, "s");
    CHECK(view->current == list->index_of(items[1]));
    dispatch_key(window, SDLK_RETURN);
    CHECK(!view->is_editing() && items[1]->text.get() == "Bananas" && view->focused);
    // Esc でやめる
    view->edit(list->index_of(items[1]));
    dispatch_text(window, "x");
    dispatch_key(window, SDLK_ESCAPE);
    CHECK(!view->is_editing() && items[1]->text.get() == "Bananas");
    // 2 列目以降は column_texts に入る
    view->add_column("名前", 100);
    view->add_column("色", 80);
    view->edit(list->index_of(items[1]), 1);
    dispatch_text(window, "黄");
    dispatch_key(window, SDLK_RETURN);
    CHECK(items[1]->column_texts.get().size() == 1 && items[1]->column_texts.get()[0] == "黄");
    // モデルの行が変わったらやめる
    view->edit(list->index_of(items[1]));
    list->add_item("date");
    CHECK(!view->is_editing());
}

// ドラッグ: from で押し、少しずつ動かして to で離す。
void drag_mouse(tokotachi::Window& window, const tokotachi::Point& from, const tokotachi::Point& to)
{
    dispatch_mouse(window, SDL_EVENT_MOUSE_BUTTON_DOWN, from);
    dispatch_mouse(window, SDL_EVENT_MOUSE_MOTION, tokotachi::Point(from.x, from.y + 6));
    dispatch_mouse(window, SDL_EVENT_MOUSE_MOTION, to);
    dispatch_mouse(window, SDL_EVENT_MOUSE_BUTTON_UP, to);
}

std::string texts_of(const tokotachi::ListModel* list)
{
    std::string out;
    const std::vector<tokotachi::Item*> items = list->items;
    for (std::size_t i = 0; i < items.size(); ++i)
    {
        out += items[i]->text.get();
    }
    return out;
}

TEST(item_view_drag_to_reorder)
{
    tokotachi::Window window("t", 300, 300);
    window.visible = true;
    tokotachi::ListModel* list = new tokotachi::ListModel(&window);
    for (const char* t : {"a", "b", "c", "d", "e"})
    {
        list->add_item(t);
    }
    tokotachi::ListView* view = new tokotachi::ListView(&window);
    view->bounds = tokotachi::Rect(0, 0, 200, 250);
    view->model = list;
    view->selection_mode = tokotachi::SelectionMode::MULTI;
    view->drag_enabled = true;
    view->drop_enabled = true;
    window.apply_layout();
    const float h = view_row_height();

    // a を d の後ろ(d の下半分)へ
    dispatch_click(window, row_point(0));
    drag_mouse(window, row_point(0), tokotachi::Point(100, 1 + 3 * h + h * 0.75f));
    CHECK(texts_of(list) == "bcdae");
    CHECK(view->current == list->index_of(list->items.get()[3]) && view->selection.get().size() == 1);  // 移した項目を選んだまま

    // 選んでいる複数の項目(b・e)をまとめて先頭へ。選んでいる項目を押してもほかの選択は外れない
    click_with(window, row_point(0), SDL_KMOD_NONE);   // b
    click_with(window, row_point(4), SDL_KMOD_LCTRL);  // と e
    CHECK(view->selection.get().size() == 2);
    drag_mouse(window, row_point(4), tokotachi::Point(100, 1 + h * 0.25f));
    CHECK(texts_of(list) == "becda");
    // ドラッグしなければ、ふつうのクリック(選択がその項目だけになる)
    click_with(window, row_point(0), SDL_KMOD_NONE);
    CHECK(view->selection.get().size() == 1);
    // ListModel::move_item(移した後の位置)
    list->move_item(list->items.get()[0], 2);
    CHECK(texts_of(list) == "ecbda");

    // 木: 項目の上に落とすと子に、自分の子孫には落とせない
    tokotachi::TreeModel* tree = new tokotachi::TreeModel(&window);
    tokotachi::TreeItem* docs = tree->add_item("docs");
    tokotachi::TreeItem* work = docs->add_child("work");
    tokotachi::TreeItem* music = tree->add_item("music");
    tokotachi::TreeView* tree_view = new tokotachi::TreeView(&window);
    tree_view->bounds = tokotachi::Rect(0, 0, 200, 250);
    tree_view->model = tree;
    tree_view->drag_enabled = true;
    tree_view->drop_enabled = true;
    tree_view->expand_all();
    view->visible = false;
    window.apply_layout();
    // 行: docs(0)・work(1)・music(2)。music を docs の上(中ほど)へ
    dispatch_click(window, row_point(2));
    drag_mouse(window, row_point(2), row_point(0));
    CHECK(music->parent_item.get() == docs && docs->children.get().size() == 2 && docs->children.get()[1] == music);
    // docs を自分の子(work)の上へは移せない
    dispatch_click(window, row_point(0));
    drag_mouse(window, row_point(0), row_point(1));
    CHECK(docs->parent_item.get() == nullptr && work->parent_item.get() == docs);
    // work を一番上の終わり(最後の行より下)へ
    tree->move_item(work, nullptr, -1);
    CHECK(work->parent_item.get() == nullptr && tree->items.get().size() == 2 && tree->items.get()[1] == work);

    // ほかのウィジェットから: モデルの can_drop・drop_data(既定は受け取らない)
    tokotachi::DragData data;
    data.text = "x";
    CHECK(!list->can_drop(data, tokotachi::ModelIndex(), 0));
}

TEST(list_view_follows_model_changes)
{
    tokotachi::Window window("t", 300, 300);
    window.visible = true;
    tokotachi::ListModel* list = new tokotachi::ListModel(&window);
    tokotachi::Item* b = list->add_item("b");
    tokotachi::Item* c = list->add_item("c");
    tokotachi::Item* a = list->add_item("a");
    tokotachi::ListView* view = new tokotachi::ListView(&window);
    view->bounds = tokotachi::Rect(0, 0, 200, 250);
    view->model = list;
    view->selection_mode = tokotachi::SelectionMode::MULTI;
    view->select(list->index_of(c));
    view->current = list->index_of(c);
    int changes = 0;
    view->on_selection_change = [&](tokotachi::Object*) { ++changes; };

    // 前に加えると番号がずれるが、同じ項目を選んだまま。
    list->insert_item(0, "z");
    CHECK(view->is_selected(list->index_of(c)) && view->current == list->index_of(c) && list->index_of(c).row() == 2);
    // 並べ替えても同じ項目。
    list->sort(0, tokotachi::SortOrder::ASCENDING);
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
    view->on_selection_change = tokotachi::NotifyHandler();
}

TEST(list_view_columns_sort_and_resize)
{
    tokotachi::Window window("t", 400, 300);
    window.visible = true;
    tokotachi::ListModel* list = new tokotachi::ListModel(&window);
    const char* const NAMES[] = {"b", "c", "a"};
    const char* const SIZES[] = {"2", "1", "3"};
    for (int i = 0; i < 3; ++i)
    {
        list->add_item(NAMES[i])->column_texts = std::vector<std::string>(1, SIZES[i]);
    }
    tokotachi::ListView* view = new tokotachi::ListView(&window);
    view->bounds = tokotachi::Rect(0, 0, 300, 250);
    tokotachi::Column* name = view->add_column("名前", 100);
    view->add_column("大きさ", 60);
    view->model = list;
    window.apply_layout();
    CHECK(list->text(list->index(0), 1) == "2");
    const float header = view_row_height();

    // 見出しを押すと並べ替える(sorting_enabled のときだけ)。もう一度押すと逆順。
    dispatch_click(window, tokotachi::Point(50, 1 + header / 2));
    CHECK(view->sort_column == -1);
    view->sorting_enabled = true;
    dispatch_click(window, tokotachi::Point(130, 1 + header / 2));
    CHECK(view->sort_column == 1 && view->sort_order == tokotachi::SortOrder::ASCENDING);
    CHECK(list->items.get()[0]->text.get() == "c");
    dispatch_click(window, tokotachi::Point(130, 1 + header / 2));
    CHECK(view->sort_order == tokotachi::SortOrder::DESCENDING && list->items.get()[0]->text.get() == "a");
    view->sort_column = 0;  // 代入でも並べ替える
    CHECK(list->items.get()[0]->text.get() == "c");
    // 見出しの下の行を押す。
    dispatch_click(window, row_point(0, 100, header));
    CHECK(view->current == list->index(0));

    // 境目をドラッグして幅を変える(最小 24)。
    dispatch_mouse(window, SDL_EVENT_MOUSE_BUTTON_DOWN, tokotachi::Point(1 + 100, 1 + header / 2));
    dispatch_mouse(window, SDL_EVENT_MOUSE_MOTION, tokotachi::Point(1 + 150, 1 + header / 2));
    dispatch_mouse(window, SDL_EVENT_MOUSE_BUTTON_UP, tokotachi::Point(1 + 150, 1 + header / 2));
    CHECK(near(name->width, 150));
    name->width = 5;
    CHECK(near(name->width, 24));
    view->remove_column(name);
    CHECK(view->columns.get().size() == 1 && view->sort_column == -1);
}

TEST(tree_view_expand_and_keys)
{
    tokotachi::Window window("t", 300, 400);
    window.visible = true;
    tokotachi::TreeModel* tree = new tokotachi::TreeModel(&window);
    tokotachi::TreeItem* docs = tree->add_item("文書");
    tokotachi::TreeItem* work = docs->add_child("仕事");
    tokotachi::TreeItem* report = work->add_child("報告書");
    tokotachi::TreeItem* readme = docs->add_child("README");
    tokotachi::TreeItem* pics = tree->add_item("画像");
    CHECK(report->parent_item == work && docs->children.get().size() == 2 && tree->items.get().size() == 2);
    CHECK(tree->parent_of(tree->index_of(report)) == tree->index_of(work));
    CHECK(tree->row_count(tree->index_of(docs)) == 2 && tree->index(1, tree->index_of(docs)) == tree->index_of(readme));

    tokotachi::TreeView* view = new tokotachi::TreeView(&window);
    view->bounds = tokotachi::Rect(0, 0, 250, 350);
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
    tokotachi::TreeItem* added = docs->insert_child(0, "新しい");
    CHECK(view->index_at(row_point(1)) == tree->index_of(added));

    view->expand_all();
    view->collapse_all();
    CHECK(view->index_at(row_point(1)) == tree->index_of(pics));
    tree->clear();
    CHECK(!view->index_at(row_point(0)).is_valid());
}


// 項目のオブジェクトを作らないモデル: 100 万行でも、描くときに問い合わせるのは見えている行だけ。
struct CountingModel : tokotachi::ItemModel
{
    mutable int queries;
    CountingModel() : queries(0) {}
    int row_count(const tokotachi::ModelIndex& parent) const override { return parent.is_valid() ? 0 : 1000000; }
    std::string text(const tokotachi::ModelIndex& index, int) const override
    {
        ++queries;
        return std::to_string(index.row());
    }
};

TEST(item_view_queries_only_visible_rows)
{
    tokotachi::Window window("t", 300, 300);
    window.visible = true;
    CountingModel* model = new CountingModel();
    tokotachi::ListView* view = new tokotachi::ListView(&window);
    view->bounds = tokotachi::Rect(0, 0, 200, 250);
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
    const tokotachi::Shortcut save("Ctrl+S");
    CHECK(save.key == tokotachi::Key::S && save.modifiers == tokotachi::MODIFIER_CONTROL);
    CHECK(tokotachi::Shortcut("ctrl+shift+z").to_string() == "Ctrl+Shift+Z");
    CHECK(tokotachi::Shortcut("F5").key == tokotachi::Key::F5 && tokotachi::Shortcut("Delete").key == tokotachi::Key::DEL);
    CHECK(tokotachi::Shortcut("Alt+PageDown").to_string() == "Alt+PageDown");
    CHECK(tokotachi::Shortcut("Ctrl+").is_empty() && tokotachi::Shortcut("Hyper+A").is_empty() && tokotachi::Shortcut("F13").is_empty());
    CHECK(tokotachi::Shortcut().to_string().empty());
    tokotachi::KeyEvent e(tokotachi::Key::S, tokotachi::MODIFIER_CONTROL, false, 0);
    CHECK(save.matches(e));
    e.modifiers = tokotachi::MODIFIER_CONTROL | tokotachi::MODIFIER_SHIFT;
    CHECK(!save.matches(e));  // 修飾キーは過不足なく
}

TEST(action_trigger_and_menu_entries)
{
    tokotachi::Window window("t", 300, 200);
    tokotachi::Action* action = new tokotachi::Action(&window, "太字(&B)");
    int triggers = 0;
    action->on_trigger = [&](tokotachi::Object* sender)
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
    tokotachi::Menu* a = new tokotachi::Menu(&window);
    tokotachi::Menu* b = new tokotachi::Menu(&window);
    a->add_action(action);
    b->add_action(action);
    tokotachi::Action* owned = a->add_item("所有");
    CHECK(a->actions.get().size() == 2 && b->actions.get().size() == 1);
    delete action;
    CHECK(a->actions.get().size() == 1 && b->actions.get().empty());
    a->remove_action(owned);  // メニューが作った Action は破棄される
    CHECK(a->actions.get().empty());
}

TEST(menu_bar_layout_and_shortcuts)
{
    tokotachi::Window window("t", 400, 300);
    tokotachi::BoxLayout* box = new tokotachi::BoxLayout(&window);
    box->margins = tokotachi::Margins(0);
    tokotachi::LineEdit* edit = new tokotachi::LineEdit(&window);
    tokotachi::ListView* list = new tokotachi::ListView(&window);
    box->add(edit);
    box->add(list, 1);
    tokotachi::MenuBar* bar = new tokotachi::MenuBar();
    tokotachi::Menu* file = bar->add_menu("ファイル(&F)");
    tokotachi::Menu* edit_menu = bar->add_menu("編集(&E)");
    std::string done;
    file->add_item("保存(&S)", [&](tokotachi::Object*) { done += "save "; })->shortcut = "Ctrl+S";
    tokotachi::Menu* recent = file->add_submenu("最近(&R)");
    recent->add_item("一つ目(&1)", [&](tokotachi::Object*) { done += "recent "; })->shortcut = "Ctrl+1";
    tokotachi::Action* select_all = edit_menu->add_item("すべて選択(&A)", [&](tokotachi::Object*) { done += "all "; });
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
    tokotachi::Action* local_save = new tokotachi::Action(&window);
    local_save->shortcut = "Ctrl+S";
    local_save->on_trigger = [&](tokotachi::Object*) { done += "local "; };
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
    tokotachi::Window window("t", 400, 300);
    tokotachi::MenuBar* bar = new tokotachi::MenuBar();
    tokotachi::Menu* file = bar->add_menu("ファイル(&F)");
    tokotachi::Menu* edit = bar->add_menu("編集(&E)");
    std::string done;
    file->add_item("開く(&O)", [&](tokotachi::Object*) { done += "open "; });
    file->add_item("保存(&S)", [&](tokotachi::Object*) { done += "save "; });
    edit->add_item("元に戻す(&U)", [&](tokotachi::Object*) { done += "undo "; });
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
    tokotachi::Window window("t", 100, 100);
    tokotachi::Widget* w = new tokotachi::Widget(&window);
    int calls = 0;
    w->on_resize = [&](tokotachi::Object* sender)
    {
        ++calls;
        sender->delete_later();
        sender->delete_later();  // 2 度頼んでも 1 度だけ破棄する
    };
    w->bounds = tokotachi::Rect(0, 0, 10, 10);
    CHECK(calls == 1 && window.children.get().size() == 1);  // まだ残っている
    // post した関数は、イベントの処理の中で呼ばれる。その処理の後に、頼まれたオブジェクトを破棄する。
    bool posted = false;
    tokotachi::detail::ApplicationImpl::post([&]() { posted = true; });
    // SDL3 は 1 回りの終わりの印をキューに入れるので、前に残った印で止まらないよう 2 回り読む。
    SDL_Event e;
    for (int round = 0; round < 2 && !posted; ++round)
    {
        while (SDL_PollEvent(&e))
        {
            tokotachi::Application::instance()->impl()->dispatch(e);
        }
    }
    CHECK(posted);
    CHECK(window.children.get().empty());
    // 頼んだ後に破棄されたら、何もしない。
    tokotachi::Widget* x = new tokotachi::Widget(&window);
    x->delete_later();
    delete x;
    tokotachi::Application::instance()->impl()->flush_doomed();
    CHECK(tokotachi::Application::instance()->impl()->doomed.empty());
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
    tokotachi::Window window("t", 400, 300);
    tokotachi::Button* behind = new tokotachi::Button(&window, "後ろ");
    behind->bounds = tokotachi::Rect(10, 10, 100, 30);
    int behind_clicks = 0;
    behind->on_click = [&](tokotachi::Object*) { ++behind_clicks; };
    window.visible = true;

    tokotachi::Dialog* dialog = new tokotachi::Dialog(&window, "名前");
    tokotachi::BoxLayout* box = new tokotachi::BoxLayout(dialog);
    tokotachi::LineEdit* field = new tokotachi::LineEdit(dialog);
    box->add(field);
    tokotachi::Button* ok = dialog->add_button(tokotachi::DialogButton::OK);
    tokotachi::Button* cancel = dialog->add_button(tokotachi::DialogButton::CANCEL);
    CHECK(ok->style == tokotachi::ButtonStyle::PRIMARY && cancel->style == tokotachi::ButtonStyle::SECONDARY);
    std::vector<tokotachi::DialogButton> results;
    dialog->on_finish = [&](tokotachi::Object* sender)
    {
        CHECK(sender == dialog);
        results.push_back(dialog->result);
    };
    dialog->open();
    CHECK(dialog->visible && dialog->owner == &window);
    CHECK(dialog->width >= 320 && dialog->result == tokotachi::DialogButton::NONE);
    // ボタンは下端に、OS の慣習の順に並ぶ。中身はその上。
    CHECK((ok->x < cancel->x) == accept_button_first());
    CHECK(near(ok->y, cancel->y) && field->y.get() + field->height.get() <= ok->y);

    // 開いている間は、親への入力を捨てる。
    dispatch_click(window, tokotachi::Point(50, 25));
    dispatch_key(window, SDLK_SPACE);
    CHECK(behind_clicks == 0);

    // 入力欄で Enter: 入力欄は受け取らず(on_submit が無い)、決定のボタンで閉じる。
    field->focused = true;
    dispatch_key(*dialog, SDLK_RETURN);
    CHECK(results.size() == 1 && results[0] == tokotachi::DialogButton::OK && !dialog->visible);
    CHECK(dialog->result == tokotachi::DialogButton::OK);
    dispatch_click(window, tokotachi::Point(50, 25));
    CHECK(behind_clicks == 1);  // 閉じたら親へ届く

    // Esc・close は取り消しのボタンの結果。
    dialog->open();
    dispatch_key(*dialog, SDLK_ESCAPE);
    dialog->open();
    dialog->close();
    CHECK(results.size() == 3 && results[1] == tokotachi::DialogButton::CANCEL && results[2] == tokotachi::DialogButton::CANCEL);
    // ボタンを押す。
    dialog->open();
    dispatch_click(*dialog, tokotachi::Point(ok->x + 10, ok->y + 10));
    CHECK(results.size() == 4 && results[3] == tokotachi::DialogButton::OK);
    dialog->on_finish = tokotachi::NotifyHandler();
}

TEST(message_dialog_show_finishes_and_deletes)
{
    tokotachi::Window window("t", 400, 300);
    window.visible = true;
    const std::size_t windows_before = tokotachi::Application::instance()->impl()->windows.size();
    tokotachi::DialogButton answer = tokotachi::DialogButton::NONE;
    tokotachi::MessageDialog* d = tokotachi::MessageDialog::show(
        &window, "確認", "保存しますか？ 長い文はダイアログの幅で折り返す。長い文はダイアログの幅で折り返す。",
        std::vector<tokotachi::DialogButton>{tokotachi::DialogButton::SAVE, tokotachi::DialogButton::DISCARD, tokotachi::DialogButton::CANCEL},
        [&](tokotachi::Object* sender) { answer = static_cast<tokotachi::Dialog*>(sender)->result; }, tokotachi::MessageType::QUESTION);
    CHECK(d->visible && d->text.get().find("保存") == 0 && d->type == tokotachi::MessageType::QUESTION);
    CHECK(d->width <= 560 && tokotachi::Application::instance()->impl()->windows.size() == windows_before + 1);
    if (const char* dir = std::getenv("TOKOTACHI_TEST_SHOTS"))
    {
        d->save_png(std::string(dir) + "/message_dialog.png");
    }
    dispatch_key(*d, SDLK_ESCAPE);  // ダイアログは dispatch の終わりに破棄される
    CHECK(answer == tokotachi::DialogButton::CANCEL);
    CHECK(tokotachi::Application::instance()->impl()->windows.size() == windows_before);

    // OK だけのお知らせは、Esc でも閉じる。
    answer = tokotachi::DialogButton::NONE;
    tokotachi::MessageDialog* info = tokotachi::MessageDialog::show(&window, "お知らせ", "終わった",
                                                        std::vector<tokotachi::DialogButton>(),
                                                        [&](tokotachi::Object* sender)
                                                        { answer = static_cast<tokotachi::Dialog*>(sender)->result; });
    dispatch_key(*info, SDLK_ESCAPE);
    CHECK(answer == tokotachi::DialogButton::OK);
}

// 溜まっているイベントをすべて処理する(SDL3 の 1 回りの終わりの印で止まらないよう 2 回り)。
void pump_events()
{
    SDL_Event e;
    for (int round = 0; round < 2; ++round)
    {
        while (SDL_PollEvent(&e))
        {
            tokotachi::Application::instance()->impl()->dispatch(e);
        }
    }
}

TEST(post_from_worker_threads)
{
    tokotachi::Application* app = tokotachi::Application::instance();
    CHECK(tokotachi::is_main_thread());
    tokotachi::Window window("t", 100, 100);  // 表示しているウィンドウが無いと exec はすぐに終わる
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
                if (tokotachi::is_main_thread())
                {
                    ++worker_on_main;
                }
                // メインスレッドが exec で眠ってから頼む(頼むと起きることを確かめる)。
                std::this_thread::sleep_for(std::chrono::milliseconds(30));
                for (int i = 0; i < PER_THREAD; ++i)
                {
                    tokotachi::post(
                        [&, t, i]()
                        {
                            on_main = on_main && tokotachi::is_main_thread();
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
    tokotachi::Timer guard;  // 届かなかったときに終えるため
    guard.interval = 5000;
    guard.on_timer = [&](tokotachi::Object*) { app->quit(); };
    guard.enabled = true;
    app->exec();
    for (std::size_t i = 0; i < workers.size(); ++i)
    {
        workers[i].join();
    }
    CHECK(received == THREADS * PER_THREAD);
    CHECK(in_order && on_main && worker_on_main == 0);
    CHECK(!tokotachi::post(std::function<void()>()));  // 空の関数は頼まない
}

TEST(post_with_weak_ref)
{
    tokotachi::Window window("t", 100, 100);
    tokotachi::Label* label = new tokotachi::Label(&window, "前");
    tokotachi::Label* doomed = new tokotachi::Label(&window, "消える");
    const tokotachi::WeakRef<tokotachi::Label> live(label);
    const tokotachi::WeakRef<tokotachi::Label> dead(doomed);
    CHECK(live.get() == label && dead.get() == doomed && tokotachi::WeakRef<tokotachi::Label>().expired());
    int calls = 0;
    std::thread worker(
        [&]()
        {
            tokotachi::post(live, [&](tokotachi::Label* l) { l->text = "後"; ++calls; });
            tokotachi::post(dead, [&](tokotachi::Label*) { ++calls; });
        });
    worker.join();
    delete doomed;  // 頼んだ処理が呼ばれる前に破棄する
    CHECK(dead.expired() && !live.expired());
    pump_events();
    CHECK(calls == 1 && label->text.get() == "後");
    // 同じ Object から作った WeakRef は目印を共有する。
    const tokotachi::WeakRef<tokotachi::Label> again(label);
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
            tokotachi::Application::instance()->impl()->dispatch(e);
        }
    }
    return done();
}

TEST(task_result_and_progress)
{
    tokotachi::Window window("t", 100, 100);
    tokotachi::Task<long>* task = new tokotachi::Task<long>(&window);
    int progress_calls = 0, finishes = 0;
    double last_progress = -1;
    bool increasing = true;
    task->on_progress = [&](tokotachi::Object* sender)
    {
        CHECK(sender == task && tokotachi::is_main_thread());
        ++progress_calls;
        increasing = increasing && task->progress > last_progress;
        last_progress = task->progress;
    };
    task->on_finish = [&](tokotachi::Object*)
    {
        ++finishes;
        CHECK(!task->running && !task->cancelled && task->error.get().empty());
    };
    const bool started = task->start(
        [](tokotachi::TaskContext& ctx) -> long
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
    CHECK(!task->start([](tokotachi::TaskContext&) -> long { return 0; }));  // 動いている間は始めない
    CHECK(run_until([&]() { return finishes > 0; }));
    CHECK(finishes == 1 && task->result == 2001000L);
    CHECK(task->progress == 2000.0 && increasing);
    CHECK(progress_calls >= 0 && progress_calls < 2000);  // 間引いて届く

    // 終わった後は、もう一度始められる(進み具合などは戻る)。
    CHECK(task->start([](tokotachi::TaskContext&) -> long { return 7; }) && task->progress == 0.0);
    CHECK(run_until([&]() { return finishes > 1; }));
    CHECK(task->result == 7L);
}

TEST(task_cancel_and_error)
{
    tokotachi::Window window("t", 100, 100);
    tokotachi::Task<void>* waiter = new tokotachi::Task<void>(&window);
    bool finished = false;
    waiter->on_finish = [&](tokotachi::Object*) { finished = true; };
    waiter->start(
        [](tokotachi::TaskContext& ctx)
        {
            while (!ctx.cancel_requested())  // 取り消されるまで待つ
            {
                std::this_thread::sleep_for(std::chrono::milliseconds(1));
            }
        });
    waiter->cancel();
    CHECK(run_until([&]() { return finished; }));
    CHECK(waiter->cancelled && !waiter->running && waiter->error.get().empty());

    tokotachi::Task<std::string>* failing = new tokotachi::Task<std::string>(&window);
    bool failed = false;
    failing->on_finish = [&](tokotachi::Object*) { failed = true; };
    failing->start([](tokotachi::TaskContext&) -> std::string { throw std::runtime_error("読めなかった"); });
    CHECK(run_until([&]() { return failed; }));
    CHECK(failing->error.get() == "読めなかった" && failing->result.get().empty() && !failing->cancelled);
}

TEST(task_destroyed_while_running)
{
    tokotachi::Window window("t", 100, 100);
    tokotachi::Task<int>* task = new tokotachi::Task<int>(&window);
    bool called = false;
    task->on_finish = [&](tokotachi::Object*) { called = true; };
    task->on_progress = [&](tokotachi::Object*) { called = true; };
    std::atomic<bool> gate(false), saw_cancel(false), returned(false);
    task->start(
        [&](tokotachi::TaskContext& ctx) -> int
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
    tokotachi::Window window("t", 500, 300);
    tokotachi::BoxLayout* box = new tokotachi::BoxLayout(&window);
    box->margins = tokotachi::Margins(0);
    tokotachi::LineEdit* edit = new tokotachi::LineEdit(&window);
    box->add(edit);
    tokotachi::MenuBar* bar = new tokotachi::MenuBar();
    bar->add_menu("ファイル(&F)")->add_item("開く");
    tokotachi::ToolBar* tools = new tokotachi::ToolBar();
    std::string done;
    tokotachi::Action* save = new tokotachi::Action(&window, "保存(&S)...");
    save->shortcut = "Ctrl+S";
    save->on_trigger = [&](tokotachi::Object*) { done += "save "; };
    tools->add_action(save);
    tokotachi::Action* bold = tools->add_item("太字");
    bold->checkable = true;
    tools->add_separator();
    tokotachi::StatusBar* status = new tokotachi::StatusBar();
    window.menu_bar = bar;
    window.tool_bar = tools;
    window.status_bar = status;
    window.visible = true;
    window.apply_layout();

    // 上からメニューバー・ツールバー・レイアウト、下端にステータスバー。
    CHECK(near(tools->y, bar->height) && near(edit->y, bar->height.get() + tools->height.get()));
    CHECK(near(status->y.get() + status->height.get(), window.height) && status->height > 0);
    // ボタンはツールバーの子。押すと Action を実行する。ツールチップは文字(メニュー向けの表記を省く)とショートカット。
    std::vector<tokotachi::Widget*> buttons;
    const std::vector<tokotachi::Widget*> children = tools->children;
    for (std::size_t i = 0; i < children.size(); ++i)
    {
        if (children[i]->showing)
        {
            buttons.push_back(children[i]);
        }
    }
    CHECK(buttons.size() == 2 && buttons[0]->tooltip.get() == "保存 (Ctrl+S)");
    const tokotachi::Point at = buttons[0]->map_to_window(tokotachi::Point(5, 5));
    dispatch_click(window, at);
    CHECK(done == "save ");
    dispatch_click(window, buttons[1]->map_to_window(tokotachi::Point(5, 5)));
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
    tokotachi::Window window("t", 200, 200);
    tokotachi::ToolBar* tools = new tokotachi::ToolBar();
    std::string done;
    for (int i = 0; i < 8; ++i)
    {
        tools->add_item("項目" + std::to_string(i), [&, i](tokotachi::Object*) { done = std::to_string(i); });
    }
    window.tool_bar = tools;
    window.visible = true;
    window.apply_layout();
    // 収まらない項目は隠れ、右端の » が出る。
    int shown = 0;
    tokotachi::Widget* chevron = nullptr;
    const std::vector<tokotachi::Widget*> children = tools->children;
    for (std::size_t i = 0; i < children.size(); ++i)
    {
        if (children[i]->showing)
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
    dispatch_click(window, chevron->map_to_window(tokotachi::Point(5, 5)));
    dispatch_key(window, SDLK_UP);  // 最後の項目
    dispatch_key(window, SDLK_RETURN);
    CHECK(done == "7");
}

TEST(tool_bar_keeps_user_visibility)
{
    // 幅に収まらずに隠すときは visible を変えない。利用者が隠した部品は場所を取らない(docs/adr/0041)。
    tokotachi::Window window("t", 400, 200);
    tokotachi::ToolBar* tools = new tokotachi::ToolBar();
    tokotachi::LineEdit* edit = new tokotachi::LineEdit();
    edit->minimum_size = tokotachi::Size(120, 0);
    tools->add_widget(edit);
    tools->add_separator();
    tools->add_item("後ろ");
    window.tool_bar = tools;
    window.visible = true;
    window.apply_layout();
    tokotachi::Widget* after = nullptr;
    const std::vector<tokotachi::Widget*> children = tools->children;
    for (std::size_t i = 0; i < children.size(); ++i)
    {
        if (children[i] != edit && children[i]->showing && (!after || children[i]->x < after->x))
        {
            after = children[i];
        }
    }
    CHECK(after && edit->showing && after->x > edit->x.get() + 100);
    edit->visible = false;
    window.apply_layout();
    CHECK(!edit->showing && near(after->x, 4));  // 場所を空けない(先頭に来た区切りも置かない)
    edit->visible = true;
    window.apply_layout();
    CHECK(edit->showing && after->x > edit->x.get() + 100);

    // 狭くして収まらなくなっても、visible は true のまま(showing だけが false)。
    window.width = 60;
    window.apply_layout();
    CHECK(edit->visible && !edit->showing);
    window.width = 400;
    window.apply_layout();
    CHECK(edit->showing);
    // 隠したまま狭くし、広げても隠れたまま。
    edit->visible = false;
    window.width = 60;
    window.apply_layout();
    window.width = 400;
    window.apply_layout();
    CHECK(!edit->visible && !edit->showing);
}

TEST(status_bar_message_and_widgets)
{
    tokotachi::Window window("t", 400, 200);
    tokotachi::StatusBar* status = new tokotachi::StatusBar();
    status->text = "準備完了";
    tokotachi::Label* position = new tokotachi::Label(nullptr, "行 1");
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
    tokotachi::Window window("t", 600, 200);
    tokotachi::Splitter* split = new tokotachi::Splitter(&window);
    split->bounds = tokotachi::Rect(0, 0, 400, 200);
    tokotachi::Widget* left = new tokotachi::Widget();
    tokotachi::Widget* right = new tokotachi::Widget();
    left->minimum_size = tokotachi::Size(50, 0);
    right->minimum_size = tokotachi::Size(50, 0);
    tokotachi::SplitterPane* left_pane = split->add_widget(left);
    tokotachi::SplitterPane* right_pane = split->add_widget(right);
    CHECK(left->parent == split && split->panes.get().size() == 2 && left_pane->widget == left);
    window.visible = true;
    window.apply_layout();
    // 望む大きさ(50)から始め、余りを stretch(1 と 1)で配る。境目は 6dp。
    std::vector<float> s = split->sizes;
    CHECK(near(s[0], 197) && near(s[1], 197) && near(right->x, 203));

    // 左の stretch を 0 にすると、大きくしても左は幅を保つ。
    left_pane->stretch = 0;
    split->bounds = tokotachi::Rect(0, 0, 500, 200);
    window.apply_layout();
    s = split->sizes;
    CHECK(near(s[0], 197) && near(s[1], 297));

    // 境目をドラッグする。最小より小さくはならず、最小の半分より小さくするとたたむ。
    int changes = 0;
    split->on_change = [&](tokotachi::Object*) { ++changes; };
    dispatch_mouse(window, SDL_EVENT_MOUSE_BUTTON_DOWN, tokotachi::Point(200, 100));
    dispatch_mouse(window, SDL_EVENT_MOUSE_MOTION, tokotachi::Point(103, 100));
    window.apply_layout();
    CHECK(near(split->sizes.get()[0], 100) && near(split->sizes.get()[1], 394) && changes == 1);
    dispatch_mouse(window, SDL_EVENT_MOUSE_MOTION, tokotachi::Point(33, 100));
    CHECK(near(split->sizes.get()[0], 50));  // 最小(30 は最小の半分より大きい)
    dispatch_mouse(window, SDL_EVENT_MOUSE_MOTION, tokotachi::Point(13, 100));
    window.apply_layout();
    CHECK(near(split->sizes.get()[0], 0) && left_pane->collapsed && !left->showing && left->visible);  // visible は変えない
    dispatch_mouse(window, SDL_EVENT_MOUSE_BUTTON_UP, tokotachi::Point(13, 100));
    // たたんだ区画は、境目(左端)をドラッグして戻す。
    dispatch_mouse(window, SDL_EVENT_MOUSE_BUTTON_DOWN, tokotachi::Point(3, 100));
    dispatch_mouse(window, SDL_EVENT_MOUSE_MOTION, tokotachi::Point(153, 100));
    dispatch_mouse(window, SDL_EVENT_MOUSE_BUTTON_UP, tokotachi::Point(153, 100));
    window.apply_layout();
    CHECK(near(split->sizes.get()[0], 150) && !left_pane->collapsed && left->showing);
    // たためない区画はたたまない。
    right_pane->collapsible = false;
    dispatch_mouse(window, SDL_EVENT_MOUSE_BUTTON_DOWN, tokotachi::Point(153, 100));
    dispatch_mouse(window, SDL_EVENT_MOUSE_MOTION, tokotachi::Point(498, 100));
    dispatch_mouse(window, SDL_EVENT_MOUSE_BUTTON_UP, tokotachi::Point(498, 100));
    CHECK(near(split->sizes.get()[1], 50));

    // sizes の代入(合計が合わない分は stretch で合わせる)。
    split->sizes = std::vector<float>{120, 100};
    window.apply_layout();
    CHECK(near(split->sizes.get()[0], 120) && near(split->sizes.get()[1], 374));
    // 区画の部品を破棄すると、その区画は消える。
    delete left;
    window.apply_layout();
    CHECK(split->panes.get().size() == 1 && near(right->x, 0) && near(right->width, 500));
    split->on_change = tokotachi::NotifyHandler();
}

TEST(splitter_hidden_pane_takes_no_space)
{
    // 区画の部品を visible = false にすると、場所も境目も取らない。戻すと元の大きさで出る(docs/adr/0041)。
    tokotachi::Window window("t", 600, 200);
    tokotachi::Splitter* split = new tokotachi::Splitter(&window);
    split->bounds = tokotachi::Rect(0, 0, 406, 200);
    tokotachi::Widget* a = new tokotachi::Widget();
    tokotachi::Widget* b = new tokotachi::Widget();
    tokotachi::Widget* c = new tokotachi::Widget();
    split->add_widget(a, 0);
    split->add_widget(b, 0);
    split->add_widget(c, 1);
    split->sizes = std::vector<float>{100, 100, 194};
    window.visible = true;
    window.apply_layout();
    CHECK(near(b->x, 106) && near(c->x, 212) && near(c->width, 194));
    const float preferred = split->preferred_size.get().width;

    b->visible = false;
    window.apply_layout();
    CHECK(!b->showing && near(c->x, 106) && near(c->width, 300));
    CHECK(split->preferred_size.get().width < preferred);
    CHECK(near(split->sizes.get()[1], 100));  // 戻したときの大きさを覚えている
    // 境目は a と c の間の 1 本だけ(b の境目をつかもうとしても、c の中)。
    dispatch_mouse(window, SDL_EVENT_MOUSE_BUTTON_DOWN, tokotachi::Point(103, 100));
    dispatch_mouse(window, SDL_EVENT_MOUSE_MOTION, tokotachi::Point(153, 100));
    dispatch_mouse(window, SDL_EVENT_MOUSE_BUTTON_UP, tokotachi::Point(153, 100));
    window.apply_layout();
    CHECK(near(a->width, 150) && near(c->x, 156) && near(c->width, 250));

    b->visible = true;
    window.apply_layout();
    CHECK(b->showing && near(b->x, 156) && near(b->width, 100) && near(c->x, 262) && near(c->width, 144));
}

TEST(mnemonics_move_focus_and_click)
{
    tokotachi::Window window("t", 400, 400);
    tokotachi::BoxLayout* box = new tokotachi::BoxLayout(&window);
    new tokotachi::Label(&window, "名前(&N)");
    tokotachi::LineEdit* line_edit_name = new tokotachi::LineEdit(&window);
    int saves = 0;
    tokotachi::Button* button_save = new tokotachi::Button(&window, "保存(&S)");
    button_save->on_click = [&](tokotachi::Object*) { ++saves; };
    tokotachi::CheckBox* check_box_over = new tokotachi::CheckBox(&window, "上書き(&O)");
    tokotachi::GroupBox* group_box_out = new tokotachi::GroupBox(&window, "出力(&U)");
    tokotachi::BoxLayout* box_out = new tokotachi::BoxLayout(group_box_out);
    tokotachi::LineEdit* line_edit_inner = new tokotachi::LineEdit(group_box_out);
    box_out->add(line_edit_inner);
    tokotachi::Label* label_memo = new tokotachi::Label(&window, "メモ(&M)");
    label_memo->buddy = line_edit_inner;
    tokotachi::Label* label_amp = new tokotachi::Label(&window, "A && B");
    // 同じキーが 2 つ: 押すたびに順に移る
    new tokotachi::Label(&window, "一(&D)");
    tokotachi::LineEdit* line_edit_d1 = new tokotachi::LineEdit(&window);
    new tokotachi::Label(&window, "二(&D)");
    tokotachi::LineEdit* line_edit_d2 = new tokotachi::LineEdit(&window);
    const std::vector<tokotachi::Widget*> children = window.children;
    for (std::size_t i = 0; i < children.size(); ++i)
    {
        box->add(children[i]);
    }
    window.visible = true;
    window.apply_layout();

    // Alt を押している間は下線を出す
    dispatch_key(window, SDLK_LALT, true, SDL_KMOD_LALT);
    CHECK(window.impl()->cues);
    dispatch_key(window, SDLK_N, true, SDL_KMOD_LALT);
    CHECK(line_edit_name->focused);  // ラベルの次の部品
    dispatch_key(window, SDLK_LALT, false);
    CHECK(!window.impl()->cues);
    dispatch_key(window, SDLK_S, true, SDL_KMOD_LALT);
    CHECK(saves == 1 && button_save->focused);
    dispatch_key(window, SDLK_O, true, SDL_KMOD_LALT);
    CHECK(check_box_over->checked);
    dispatch_key(window, SDLK_U, true, SDL_KMOD_LALT);
    CHECK(line_edit_inner->focused);  // 枠の中の最初の部品
    line_edit_name->focused = true;
    dispatch_key(window, SDLK_M, true, SDL_KMOD_LALT);
    CHECK(line_edit_inner->focused);  // buddy
    dispatch_key(window, SDLK_D, true, SDL_KMOD_LALT);
    CHECK(line_edit_d1->focused);
    dispatch_key(window, SDLK_D, true, SDL_KMOD_LALT);
    CHECK(line_edit_d2->focused);
    dispatch_key(window, SDLK_D, true, SDL_KMOD_LALT);
    CHECK(line_edit_d1->focused);
    // Ctrl と一緒・無効な部品・当てはまらないキーは何もしない
    dispatch_key(window, SDLK_S, true, static_cast<SDL_Keymod>(SDL_KMOD_LALT | SDL_KMOD_LCTRL));
    button_save->enabled = false;
    dispatch_key(window, SDLK_S, true, SDL_KMOD_LALT);
    CHECK(saves == 1);
    // 「&&」は「&」を出す(ニーモニックではない)
    CHECK(label_amp->text == "A && B");
    const tokotachi::TextLayout amp("A & B", tokotachi::Font(), label_amp->display_scale);
    CHECK(label_amp->preferred_size.get().width == std::ceil(amp.width()));
}

TEST(group_box_content)
{
    tokotachi::Window window("t", 300, 300);
    tokotachi::GroupBox* group_box_out = new tokotachi::GroupBox(&window, "出力");
    tokotachi::BoxLayout* box_out = new tokotachi::BoxLayout(group_box_out);
    tokotachi::Button* button_go = new tokotachi::Button(group_box_out, "実行");
    box_out->add(button_go);
    const tokotachi::Size inner = button_go->preferred_size;
    const tokotachi::Size outer = group_box_out->preferred_size;
    CHECK(outer.width >= inner.width + 24 && outer.height > inner.height + 24);
    group_box_out->bounds = tokotachi::Rect(10, 10, 200, 120);
    window.visible = true;
    window.apply_layout();
    // 中の部品は、枠と見出しの内側に並ぶ
    const tokotachi::Rect content = group_box_out->content_rect;
    CHECK(content.y > 12 && near(content.x, 12) && near(content.right(), 188) && near(content.bottom(), 108));
    CHECK(near(button_go->x, content.x) && near(button_go->y, content.y) && near(button_go->width, content.width));
    // 見出しが無ければ、上も同じ余白
    group_box_out->title = "";
    window.apply_layout();
    CHECK(near(group_box_out->content_rect.get().y, 12));
}

TEST(progress_bar_indeterminate)
{
    tokotachi::Window window("t", 300, 100);
    tokotachi::ProgressBar* progress_bar_busy = new tokotachi::ProgressBar(&window);
    progress_bar_busy->text_visible = true;
    const float with_text = progress_bar_busy->preferred_size.get().width;
    progress_bar_busy->indeterminate = true;
    CHECK(progress_bar_busy->preferred_size.get().width < with_text);  // 割合の文字は出さない
    progress_bar_busy->bounds = tokotachi::Rect(0, 0, 200, 20);
    window.visible = true;
    // 動く部分(棒の 3 割)は、動き始めに棒の左寄りにある
    tokotachi::Bitmap b = window.capture();
    const float s = static_cast<float>(b.width()) / 300;
    const tokotachi::Color on = b.pixel(static_cast<int>(60 * s), static_cast<int>(10 * s));
    const tokotachi::Color off = b.pixel(static_cast<int>(150 * s), static_cast<int>(10 * s));
    CHECK(on != off);
    progress_bar_busy->indeterminate = false;
    CHECK(progress_bar_busy->preferred_size.get().width == with_text);
}

TEST(spin_box_prefix_suffix_and_slider_ticks)
{
    tokotachi::Window window("t", 300, 100);
    window.visible = true;
    tokotachi::SpinBox* spin_box_weight = new tokotachi::SpinBox(&window);
    spin_box_weight->bounds = tokotachi::Rect(10, 10, 140, 32);
    spin_box_weight->maximum = 1000000;  // 幅が数値で決まるように(短い数値なら最小の幅)
    const float plain = spin_box_weight->preferred_size.get().width;
    spin_box_weight->suffix = " kg";
    spin_box_weight->value = 5;
    CHECK(spin_box_weight->text == "5 kg" && spin_box_weight->preferred_size.get().width > plain);
    // 単位は付けても付けなくてもよい
    const char* inputs[] = {"7", "8kg", " 9 kg ", "x kg"};
    const double expected[] = {7, 8, 9, 9};
    const char* shown[] = {"7 kg", "8 kg", "9 kg", "9 kg"};
    for (int i = 0; i < 4; ++i)
    {
        dispatch_click(window, tokotachi::Point(30, 26));
        dispatch_key(window, SDLK_A, true, SDL_KMOD_LCTRL);
        dispatch_text(window, inputs[i]);
        dispatch_key(window, SDLK_RETURN);
        CHECK(spin_box_weight->value == expected[i] && spin_box_weight->text == shown[i]);
    }
    spin_box_weight->prefix = "約";
    CHECK(spin_box_weight->text == "約9 kg");

    tokotachi::Slider* slider_level = new tokotachi::Slider(&window);
    const float thin = slider_level->preferred_size.get().height;
    slider_level->tick_interval = 10;
    CHECK(slider_level->preferred_size.get().height > thin);
    slider_level->tick_interval = -1;  // 0 以下は目盛り無し
    CHECK(slider_level->tick_interval == 0 && slider_level->preferred_size.get().height == thin);
}

TEST(splitter_double_click_and_keys)
{
    tokotachi::Window window("t", 600, 200);
    tokotachi::Splitter* split = new tokotachi::Splitter(&window);
    split->bounds = tokotachi::Rect(0, 0, 400, 200);
    tokotachi::Widget* left = new tokotachi::Widget();
    tokotachi::Widget* right = new tokotachi::Widget();
    left->minimum_size = tokotachi::Size(50, 0);
    right->minimum_size = tokotachi::Size(50, 0);
    tokotachi::SplitterPane* left_pane = split->add_widget(left, 0);
    split->add_widget(right, 1);
    window.visible = true;
    window.apply_layout();
    split->sizes = std::vector<float>{150, 244};
    window.apply_layout();
    int changes = 0;
    split->on_change = [&](tokotachi::Object*) { ++changes; };
    // ダブルクリック: stretch の小さい左をたたみ、もう一度で元の大きさに戻す
    click_with(window, tokotachi::Point(153, 100), SDL_KMOD_NONE, 1);
    click_with(window, tokotachi::Point(153, 100), SDL_KMOD_NONE, 2);
    window.apply_layout();
    CHECK(left_pane->collapsed && near(split->sizes.get()[1], 394) && changes == 1);
    click_with(window, tokotachi::Point(3, 100), SDL_KMOD_NONE, 2);
    window.apply_layout();
    CHECK(!left_pane->collapsed && near(split->sizes.get()[0], 150) && near(split->sizes.get()[1], 244) && changes == 2);
    // 境目を押した後は、矢印キーで動かし、Enter でたたむ
    CHECK(split->focused);
    dispatch_key(window, SDLK_RIGHT);
    window.apply_layout();
    CHECK(near(split->sizes.get()[0], 160) && changes == 3);
    dispatch_key(window, SDLK_RETURN);
    window.apply_layout();
    CHECK(left_pane->collapsed);
    // プログラムからも戻せる(on_change は呼ばない)
    left_pane->collapsed = false;
    window.apply_layout();
    CHECK(!left_pane->collapsed && near(split->sizes.get()[0], 160) && changes == 4);
}

TEST(splitter_vertical_and_nested)
{
    tokotachi::Window window("t", 300, 400);
    tokotachi::Splitter* outer = new tokotachi::Splitter(&window, tokotachi::Orientation::VERTICAL);
    outer->bounds = tokotachi::Rect(0, 0, 300, 400);
    tokotachi::Widget* top = new tokotachi::Widget();
    tokotachi::Splitter* inner = new tokotachi::Splitter();
    inner->add_widget(new tokotachi::Widget());
    inner->add_widget(new tokotachi::Widget());
    inner->add_widget(new tokotachi::Widget());
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

TEST(canvas_transforms_clip_and_dashes)
{
    tokotachi::Window window("t", 200, 120);
    tokotachi::Widget* w = new tokotachi::Widget(&window);
    w->bounds = tokotachi::Rect(0, 0, 200, 120);
    const tokotachi::Color red = tokotachi::Color::rgb(0xff0000), blue = tokotachi::Color::rgb(0x0000ff), green = tokotachi::Color::rgb(0x00ff00);
    tokotachi::Rect seen_bounds;
    w->on_paint = [&](tokotachi::Object*, tokotachi::Canvas& c)
    {
        // 回転: 横長の矩形を 90 度回すと縦長になる。
        c.save();
        c.translate(100, 60);
        c.rotate(1.5707963f);
        c.fill_rect(tokotachi::Rect(-40, -5, 80, 10), red);
        seen_bounds = c.clip_bounds();
        c.restore();
        // パスでクリップ: 円の外は塗らない。
        c.save();
        tokotachi::Path circle;
        circle.add_ellipse(tokotachi::Point(40, 40), 20, 20);
        c.clip_path(circle);
        c.fill_rect(tokotachi::Rect(0, 0, 80, 80), blue);
        c.draw_text("A", tokotachi::Point(34, 30), red);  // クリップの中の文字は輪郭で描く
        c.restore();
        // 破線。
        tokotachi::StrokeStyle dashed(4);
        dashed.dashes = std::vector<float>{10, 10};
        c.draw_line(tokotachi::Point(120, 100), tokotachi::Point(200, 100), dashed, green);
        // 拡大。
        c.save();
        c.translate(150, 10);
        c.scale_by(2, 2);
        c.fill_rect(tokotachi::Rect(0, 0, 10, 10), red);
        c.restore();
    };
    window.visible = true;
    window.impl()->render(false);
    const int stride = window.impl()->pixel_width;
    const std::uint32_t* px = &window.impl()->buffer[0];
    const auto at = [&](int x, int y) { return px[y * stride + x]; };
    CHECK(at(100, 30) == packed(red) && at(100, 90) == packed(red));  // 縦に伸びた
    CHECK(at(70, 60) != packed(red) && at(130, 60) != packed(red));
    CHECK(seen_bounds.width > 0 && seen_bounds.height > 0);           // 回転した座標系でのクリップの範囲
    CHECK(at(40, 25) == packed(blue) && at(22, 22) != packed(blue));  // 円の中だけ
    CHECK(at(75, 75) != packed(blue));
    CHECK(at(125, 100) == packed(green) && at(135, 100) != packed(green) && at(145, 100) == packed(green));
    CHECK(at(165, 25) == packed(red) && at(172, 32) != packed(red));  // 10dp を 2 倍した 20dp の正方形
    if (const char* dir = std::getenv("TOKOTACHI_TEST_SHOTS"))
    {
        window.save_png(std::string(dir) + "/canvas.png");
    }
}

TEST(path_arc_and_quad)
{
    tokotachi::Path p;
    p.add_arc(tokotachi::Point(0, 0), 10, 10, 0, 3.1415926f);  // 右から時計回りに半周(下を通って左へ)
    const std::vector<tokotachi::Path::Element>& els = p.elements();
    CHECK(els.size() == 3 && els[0].op == tokotachi::Path::Op::MOVE_TO);  // move と 90 度ずつの 2 つのベジェ
    CHECK(near(els[0].pts[0], 10) && near(els[0].pts[1], 0));
    CHECK(near(els[1].pts[4], 0) && near(els[1].pts[5], 10));      // 90 度で真下
    CHECK(near(els[2].pts[4], -10) && near(els[2].pts[5], 0));
    tokotachi::Path q;
    q.move_to(0, 0);
    q.quad_to(10, 10, 20, 0);
    CHECK(q.elements().size() == 2 && q.elements()[1].op == tokotachi::Path::Op::CUBIC_TO);
    CHECK(near(q.elements()[1].pts[0], 20.0f / 3) && near(q.elements()[1].pts[4], 20));
}

TEST(canvas_layer_group_opacity)
{
    tokotachi::Window window("t", 200, 100);
    tokotachi::Widget* w = new tokotachi::Widget(&window);
    w->bounds = tokotachi::Rect(0, 0, 200, 100);
    const tokotachi::Color red = tokotachi::Color::rgb(0xff0000);
    w->on_paint = [&](tokotachi::Object*, tokotachi::Canvas& c)
    {
        c.fill_rect(tokotachi::Rect(0, 0, 200, 100), tokotachi::Color::rgb(0xffffff));
        // 図形ごとに半透明: 重なった所は濃くなる。
        c.fill_rect(tokotachi::Rect(10, 10, 40, 40), red.with_alpha(128));
        c.fill_rect(tokotachi::Rect(30, 10, 40, 40), red.with_alpha(128));
        // グループの透過: 重なった所も同じ濃さ。
        c.begin_layer(0.5f);
        c.fill_rect(tokotachi::Rect(110, 10, 40, 40), red);
        c.fill_rect(tokotachi::Rect(130, 10, 40, 40), red);
        c.end_layer();
        // クリップした範囲のレイヤー(一時的な画像の左上が原点でない)の中の文字。
        c.save();
        c.clip_rect(tokotachi::Rect(100, 60, 100, 40));
        c.begin_layer(1.0f);
        c.draw_text("ABC", tokotachi::Point(110, 65), tokotachi::Color::rgb(0x000000));
        c.end_layer();
        c.restore();
    };
    window.visible = true;
    window.impl()->render(false);
    const int stride = window.impl()->pixel_width;
    const std::uint32_t* px = &window.impl()->buffer[0];
    const auto at = [&](int x, int y) { return px[y * stride + x]; };
    CHECK(at(20, 20) != at(40, 20));    // 図形ごと: 重なりが濃い
    CHECK(at(120, 20) == at(140, 20));  // グループ: 同じ濃さ
    CHECK(at(140, 20) != 0xffffffffu && at(140, 20) != packed(red));
    int ink = 0;
    for (int y = 60; y < 100; ++y)
    {
        for (int x = 100; x < 200; ++x)
        {
            ink += (at(x, y) & 0xff) < 0x80 ? 1 : 0;
        }
    }
    CHECK(ink > 20);  // 文字がレイヤーの中の正しい位置に描かれ、重ねられた
    if (const char* dir = std::getenv("TOKOTACHI_TEST_SHOTS"))
    {
        window.save_png(std::string(dir) + "/layer.png");
    }
}

TEST(draw_image_scales_smoothly)
{
    // 白黒の市松 4×4 を 2×2 に縮めると灰色(面積の平均)。最も近い画素を取ると白か黒になる
    std::vector<tokotachi::Color> pixels;
    for (int i = 0; i < 16; ++i)
    {
        pixels.push_back(((i % 4) + (i / 4)) % 2 ? tokotachi::Color(255, 255, 255) : tokotachi::Color(0, 0, 0));
    }
    const tokotachi::Image checker = tokotachi::Image::from_pixels(4, 4, pixels);
    tokotachi::Bitmap small(2, 2);
    small.canvas().draw_image(checker, tokotachi::Rect(0, 0, 2, 2));
    const tokotachi::Color c = small.pixel(1, 1);
    CHECK(c.r > 100 && c.r < 155 && c.a == 255);
    // 黒・白の 2×1 を 8×1 に広げると、間は灰色の段階になる(端は元の色)
    std::vector<tokotachi::Color> two;
    two.push_back(tokotachi::Color(0, 0, 0));
    two.push_back(tokotachi::Color(255, 255, 255));
    tokotachi::Bitmap wide(8, 1);
    wide.canvas().draw_image(tokotachi::Image::from_pixels(2, 1, two), tokotachi::Rect(0, 0, 8, 1));
    CHECK(wide.pixel(0, 0).r == 0 && wide.pixel(7, 0).r == 255);
    CHECK(wide.pixel(3, 0).r > 60 && wide.pixel(3, 0).r < 128 && wide.pixel(4, 0).r > 128 && wide.pixel(4, 0).r < 200);
    // 同じ大きさなら写さずにそのまま
    tokotachi::Bitmap same(4, 4);
    same.canvas().draw_image(checker, tokotachi::Rect(0, 0, 4, 4));
    CHECK(same.pixel(0, 0).r == 0 && same.pixel(1, 0).r == 255);
}

TEST(bitmap_canvas_pixels_and_image)
{
    tokotachi::Bitmap b(20, 10, 2);  // 倍率 2: 10 × 5 dp
    CHECK(!b.is_null() && b.width() == 20 && near(b.size().width, 10) && b.pixel(0, 0).a == 0);
    b.canvas().fill_rect(tokotachi::Rect(0, 0, 5, 5), tokotachi::Color::rgb(0x2563eb));  // 10 × 10 ピクセル
    CHECK(b.pixel(9, 9) == tokotachi::Color::rgb(0x2563eb) && b.pixel(10, 9).a == 0);
    b.set_pixel(19, 0, tokotachi::Color(255, 0, 0, 128));
    CHECK(b.pixel(19, 0).a == 128 && b.pixel(19, 0).r >= 254);
    CHECK(b.pixels()[0] == 0xff2563ebu);  // アルファ乗算済みの ARGB
    const tokotachi::Image image = b.to_image();
    b.clear(tokotachi::Color::rgb(0x00ff00));
    CHECK(image.pixel(0, 0) == tokotachi::Color::rgb(0x2563eb));  // 後で変えても、写した画像は変わらない
    CHECK(b.pixel(0, 0) == tokotachi::Color::rgb(0x00ff00));
    const tokotachi::Bitmap copy = tokotachi::Bitmap::from_image(image);
    CHECK(copy.width() == 20 && copy.pixel(5, 5) == tokotachi::Color::rgb(0x2563eb));
    tokotachi::Bitmap moved(std::move(b));
    CHECK(b.is_null() && moved.pixel(0, 0) == tokotachi::Color::rgb(0x00ff00));
    CHECK(tokotachi::Bitmap(0, 5).is_null());
    b.canvas().fill_rect(tokotachi::Rect(0, 0, 1, 1), tokotachi::Color());  // 空でも落ちない
}

TEST(paint_box_cached)
{
    tokotachi::Window window("t", 120, 80);
    tokotachi::PaintBox* box = new tokotachi::PaintBox(&window);
    box->bounds = tokotachi::Rect(0, 0, 100, 60);
    int calls = 0;
    box->on_paint = [&](tokotachi::Object* sender, tokotachi::Canvas& c)
    {
        CHECK(sender == box);
        ++calls;
        c.fill_rect(tokotachi::Rect(0, 0, 50, 60), tokotachi::Color::rgb(0x059669));
    };
    window.visible = true;
    window.impl()->render(false);
    box->invalidate();
    window.impl()->render(false);
    CHECK(calls == 2);  // 覚えないとき: 描き直すたびに呼ぶ
    box->cached = true;
    window.impl()->render(false);
    box->invalidate();
    window.impl()->render(false);
    CHECK(calls == 3);  // 覚えるとき: 1 回だけ
    const int stride = window.impl()->pixel_width;
    CHECK(window.impl()->buffer[10 * stride + 10] == packed(tokotachi::Color::rgb(0x059669)));  // 覚えた絵を貼る
    CHECK(window.impl()->buffer[10 * stride + 70] != packed(tokotachi::Color::rgb(0x059669)));
    box->refresh();
    window.impl()->render(false);
    CHECK(calls == 4);
    box->bounds = tokotachi::Rect(0, 0, 110, 60);  // 大きさが変わると描き直す
    window.impl()->render(false);
    CHECK(calls == 5);
}

// 文字と図形を Bitmap に描き、画素の並びを返す(どのスレッドからでも呼べる)。
std::vector<std::uint32_t> render_text_bitmap(float size, int round)
{
    tokotachi::Bitmap b(160, 60);
    tokotachi::Canvas& c = b.canvas();
    c.fill_rect(tokotachi::Rect(0, 0, 160, 60), tokotachi::Color::rgb(0xffffff));
    const std::string text = "スレッド " + std::to_string(round) + " Ag漢字";
    c.draw_text(tokotachi::TextLayout(text, tokotachi::Font(size), 1), tokotachi::Point(4, 4), tokotachi::Color::rgb(0x111827));
    c.save();
    c.translate(80, 40);
    c.rotate(0.3f);
    c.draw_text("回転", tokotachi::Point(0, 0), tokotachi::Color::rgb(0x2563eb));  // 輪郭で描く
    c.restore();
    return std::vector<std::uint32_t>(b.pixels(), b.pixels() + 160 * 60);
}

TEST(text_rendering_on_worker_threads)
{
    // 各スレッドで、別々の大きさ(フォントの実体を同時に開く)の文字を何度も描く。
    const int THREADS = 4, ROUNDS = 20;
    std::vector<std::vector<std::uint32_t> > results(THREADS);
    std::vector<std::thread> workers;
    for (int t = 0; t < THREADS; ++t)
    {
        workers.push_back(std::thread(
            [&results, t]()
            {
                for (int r = 0; r < ROUNDS; ++r)
                {
                    results[t] = render_text_bitmap(11.0f + t * 3 + (r % 5), r);
                }
            }));
    }
    for (std::size_t i = 0; i < workers.size(); ++i)
    {
        workers[i].join();
    }
    // メインスレッドで同じものを描くと、同じ画素になる。
    bool same = true;
    for (int t = 0; t < THREADS; ++t)
    {
        same = same && results[t] == render_text_bitmap(11.0f + t * 3 + ((ROUNDS - 1) % 5), ROUNDS - 1);
    }
    CHECK(same);
    int ink = 0;
    for (std::size_t i = 0; i < results[0].size(); ++i)
    {
        ink += results[0][i] != 0xffffffffu ? 1 : 0;
    }
    CHECK(ink > 100);
}

TEST(task_renders_image_in_background)
{
    tokotachi::Window window("t", 100, 100);
    tokotachi::Task<tokotachi::Image>* task = new tokotachi::Task<tokotachi::Image>(&window);
    bool done = false;
    task->on_finish = [&](tokotachi::Object*) { done = true; };
    task->start(
        [](tokotachi::TaskContext&) -> tokotachi::Image
        {
            tokotachi::Bitmap b(80, 40);
            b.canvas().fill_rect(tokotachi::Rect(0, 0, 80, 40), tokotachi::Color::rgb(0x059669));
            b.canvas().draw_text("裏", tokotachi::Point(4, 4), tokotachi::Color::rgb(0xffffff));
            return b.to_image();
        });
    CHECK(run_until([&]() { return done; }));
    const tokotachi::Image image = task->result;
    CHECK(image.width() == 80 && image.pixel(70, 30) == tokotachi::Color::rgb(0x059669));
}

}  // namespace

int main(int argc, char** argv)
{
    tokotachi::Application app(argc, argv);
    for (std::size_t i = 0; i < registry().size(); ++i)
    {
        const int before = g_failures;
        registry()[i].fn();
        std::printf("%s %s\n", g_failures == before ? "ok  " : "FAIL", registry()[i].name);
    }
    std::printf("%d checks, %d failures\n", g_checks, g_failures);
    return g_failures == 0 ? 0 : 1;
}
