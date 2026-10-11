/// @file
/// ウィジェットの基底(docs/adr/0010、0012、0013)。
///
/// 所有: 親が子を所有する(親が Object の所有者を兼ねる。docs/adr/0015)。new Widget(parent) で作り、親が破棄されるときに
/// 子も delete される。子を先に delete してもよい(親の一覧から外れる)。親を持つウィジェットをスタックに置いてはいけない。
/// ウィジェットはメインスレッドだけで扱う。
///
/// イベント: 利用者は on_xxx にハンドラを代入する。派生クラスは protected の仮想関数(paint・mouse_down 等)を上書きし、
/// ハンドラを呼ぶために基底の実装を呼ぶ。
#ifndef TOKOTACHI_WIDGETS_WIDGET_HPP
#define TOKOTACHI_WIDGETS_WIDGET_HPP

#include <string>
#include <vector>

#include "../core/drag.hpp"
#include "../core/event.hpp"
#include "../core/geometry.hpp"
#include "../core/property.hpp"
#include "../graphics/theme.hpp"

namespace tokotachi
{

class Canvas;
class Layout;
class Action;
class Window;

namespace detail
{
struct WidgetAccess;
class ThemeScope;
}

enum class FocusPolicy
{
    NONE,
    TAB,           ///< Tab キーでだけフォーカスを得る
    CLICK,         ///< クリックでだけフォーカスを得る
    TAB_AND_CLICK  ///< Tab キーとクリックの両方
};

enum class MouseCursor
{
    ARROW,
    IBEAM,
    POINTING_HAND,
    WAIT,
    CROSSHAIR,
    SIZE_HORIZONTAL,
    SIZE_VERTICAL
};

class Widget : public Object
{
public:
    explicit Widget(Widget* parent = nullptr);
    virtual ~Widget();

    // --- ツリー ---
    /// 代入すると親を付け替える(所有者も移る。docs/adr/0018)。ウィンドウは付け替えられない。
    /// 自分の子孫を親にすることはできない(無視する)。nullptr にすると親も所有者も無くなる(利用者が破棄する)。
    Property<Widget*> parent;
    ReadOnlyProperty<std::vector<Widget*> > children;
    /// このウィジェットが属するウィンドウ(ウィンドウ自身なら自分)。どのウィンドウにも属していなければ nullptr。
    ReadOnlyProperty<Window*> window;

    // --- 位置と大きさ(親の左上からの相対、dp) ---
    Property<float> x;
    Property<float> y;
    Property<float> width;
    Property<float> height;
    Property<Rect> bounds;
    /// 自分の座標系での矩形(左上が原点)。
    ReadOnlyProperty<Rect> local_rect;
    // --- レイアウト(docs/adr/0017) ---
    /// 望む大きさ(minimum_hint 以上 maximum_size 以下)。
    ReadOnlyProperty<Size> preferred_size;
    /// 利用者が決める最小・最大の大きさ(既定は 0 と無制限)。
    Property<Size> minimum_size;
    Property<Size> maximum_size;
    /// レイアウトが縮めてよい最小の大きさ(minimum_size と、ウィジェットが決める最小の大きさの大きい方)。
    ReadOnlyProperty<Size> minimum_hint;
    /// 子を配置するレイアウト(無ければ自由配置)。レイアウトを作ると付き、delete すると外れる。
    ReadOnlyProperty<Layout*> layout;

    // --- 状態 ---
    /// 表示するか(利用者が決める)。ToolBar・Splitter・TabWidget が自分の都合(幅に収まらない・たたんだ・選んでいないタブ)で
    /// 子を隠すときは、この値を変えない(showing が false になる。docs/adr/0041)。
    Property<bool> visible;
    /// 実際に表示されるか: 自分と祖先がすべて visible で、どれも親に隠されていない。
    ReadOnlyProperty<bool> showing;
    Property<bool> enabled;
    /// 自分と祖先がすべて enabled か。
    ReadOnlyProperty<bool> effectively_enabled;
    Property<FocusPolicy> focus_policy;
    /// true を代入するとフォーカスを得る(focus_policy が NONE なら何もしない)。false を代入するとフォーカスを外す。
    Property<bool> focused;
    Property<MouseCursor> cursor;
    /// 文字の入力(IME を含む)を受け付けるか。フォーカスを得たときに IME が有効になる。
    Property<bool> accepts_text_input;
    /// dp からピクセルへの倍率(ウィンドウに属していなければ 1)。
    ReadOnlyProperty<float> display_scale;
    /// ポインタを少し止めたときに出す説明(空なら出さない。docs/adr/0022)。
    Property<std::string> tooltip;
    /// テーマの一部の上書き(docs/adr/0042)。自分と子孫に効く(ポップアップは開いた元のウィジェットのものを継ぐ)。
    Property<ThemeOverride> theme_override;
    /// 自分に効いているテーマ(アプリケーションのテーマに、祖先と自分の theme_override を重ねたもの)。
    /// 変わったことは、theme_override を変えたウィジェットと、アプリケーションのテーマが変わったときに、子孫にも知らせる。
    ReadOnlyProperty<Theme> theme;

    // --- ショートカット(docs/adr/0027) ---
    /// Action を加える。そのショートカットは、フォーカスがこのウィジェットの中にあるときに効く(ウィンドウに加えればウィンドウ全体)。
    /// 所有はしない(Action が破棄されると外れる)。
    void add_action(Action* action);
    void remove_action(Action* action);
    ReadOnlyProperty<std::vector<Action*> > actions;

    // --- ドラッグ&ドロップ(docs/adr/0047) ---
    /// ドロップを受け取るか(既定 false)。true なら、ポインタの下にドラッグが来たときに drag_move・drag_leave・drop を送る。
    Property<bool> accepts_drops;
    /// ドラッグを始める。マウスのボタンを押している間(mouse_down・mouse_move の中)に呼ぶ。label はドラッグ中にポインタの横に出す文字。
    /// ボタンを離すとポインタの下の受け取るウィジェットへ drop を送り、終わったら drag_finished・on_drag_finish を呼ぶ。Esc でやめる。
    /// ボタンを押していない・ウィンドウに属していない・ほかのドラッグの途中なら何もせず false。
    bool start_drag(const DragData& data, const std::string& label = std::string());
    /// ドラッグがポインタの下に来た・動いた。受け取らないなら accepted を false にする(親へ送る)。操作は action で変えられる。
    /// ハンドラが無ければ、on_drop があるときだけ受け取る。
    Property<DragHandler> on_drag_move;
    /// ドラッグが離れた(受け取ると答えた後に、ほかへ移った・やめた)。
    Property<NotifyHandler> on_drag_leave;
    /// 落とした。受け取らなかったら accepted を false にする。
    Property<DragHandler> on_drop;
    /// 自分が始めたドラッグが終わった(受け取った側の操作。受け取られなければ NONE。MOVE なら元のデータを消す)。
    Property<DragFinishHandler> on_drag_finish;

    // --- イベントのハンドラ ---
    /// 自分の描画の後に呼ぶ(子の描画の前)。
    Property<PaintHandler> on_paint;
    Property<NotifyHandler> on_resize;
    Property<MouseHandler> on_mouse_down;
    Property<MouseHandler> on_mouse_up;
    Property<MouseHandler> on_mouse_move;
    Property<NotifyHandler> on_mouse_enter;
    Property<NotifyHandler> on_mouse_leave;
    /// ホイール。受け取らなければ(ハンドラが無いか accepted を false にすると)親へ送る。
    Property<WheelHandler> on_mouse_wheel;
    Property<KeyHandler> on_key_down;
    Property<KeyHandler> on_key_up;
    Property<NotifyHandler> on_focus_in;
    Property<NotifyHandler> on_focus_out;

    // --- 操作 ---
    /// 描き直しを依頼する(次のイベントの処理の後にまとめて描く)。
    void invalidate();
    void invalidate(const Rect& r);

    /// 配置のやり直しを依頼する(望む大きさが変わったとき。次の描画の前にまとめて配置する)。
    void request_layout();
    /// 依頼されている配置を、すぐに行う(配置の結果をすぐに読みたいとき)。
    void apply_layout();

    Point map_to_window(const Point& p) const;
    Point map_from_window(const Point& p) const;
    /// p(自分の座標系)にある、表示されている最も深い子孫。無ければ nullptr。
    Widget* child_at(const Point& p) const;

protected:
    virtual void paint(Canvas& canvas);
    virtual void resized();
    virtual void mouse_down(MouseEvent& e);
    virtual void mouse_up(MouseEvent& e);
    virtual void mouse_move(MouseEvent& e);
    virtual void mouse_enter();
    virtual void mouse_leave();
    virtual void mouse_wheel(WheelEvent& e);
    virtual void key_down(KeyEvent& e);
    virtual void key_up(KeyEvent& e);
    virtual void text_input(TextInputEvent& e);
    virtual void text_editing(TextEditingEvent& e);
    virtual void focus_in();
    virtual void focus_out();
    /// ドラッグ&ドロップ(accepts_drops のとき。docs/adr/0047)。既定の実装はハンドラを呼ぶ。
    virtual void drag_move(DragEvent& e);
    virtual void drag_leave();
    virtual void drop(DragEvent& e);
    /// 自分が始めたドラッグが終わった。既定の実装は on_drag_finish を呼ぶ。
    virtual void drag_finished(DropAction action);
    /// 倍率が変わった(テキストのレイアウトなど、ピクセルに依存するキャッシュを捨てる)。
    virtual void scale_changed();
    /// アプリケーションのテーマが変わった(色・寸法から作ったキャッシュを捨てる)。この後、描き直しと配置のやり直しが行われる。
    virtual void theme_changed();
    /// IME の候補窓を置く位置(キャレットの矩形。自分の座標系)。
    virtual Rect input_method_rect() const;
    /// 子を配置する(配置のやり直しの印が付いているときに、描画の前に呼ばれる)。既定の実装はレイアウトに任せる。
    /// 子の配置を自分で決めるウィジェット(ScrollArea 等)が上書きする(docs/adr/0021)。
    virtual void arrange_children();
    /// 子が外れた(破棄された・別の親へ移った)。子を覚えているウィジェット(TabWidget 等)が忘れるために使う。
    virtual void child_removed(Widget* child);
    /// add_action で Action が加わった・remove_action(Action の破棄を含む)で外れた・Action の表示(文字・画像・有効など)が
    /// 変わった。Action をボタンとして見せるウィジェット(ToolBar 等)が使う。
    virtual void action_added(Action* action);
    virtual void action_removed(Action* action);
    virtual void action_changed(Action* action);
    /// 自分の配置だけをやり直す印を付ける(子の配置が変わるが、自分の望む大きさは変わらないとき)。
    void mark_layout_dirty();

    /// 望む大きさ・ウィジェットが決める最小の大きさ。既定の実装は、レイアウトがあればその大きさ、無ければ 0。
    virtual Size compute_preferred_size() const;
    virtual Size compute_minimum_size() const;

    /// input_method_rect() が変わったことをウィンドウに知らせる。
    void update_input_method();

private:
    friend struct detail::WidgetAccess;
    friend class detail::ThemeScope;

    Widget* get_parent() const { return parent_; }
    void set_parent(Widget* const& p);
    /// w と子孫に scale_changed を送り、配置のやり直しの印を付ける(別のウィンドウへ移ったとき)。
    static void scale_changed_tree(Widget* w);
    std::vector<Widget*> get_children() const { return children_; }
    Window* get_window() const;
    float get_x() const { return bounds_.x; }
    void set_x(const float& v);
    float get_y() const { return bounds_.y; }
    void set_y(const float& v);
    float get_width() const { return bounds_.width; }
    void set_width(const float& v);
    float get_height() const { return bounds_.height; }
    void set_height(const float& v);
    Rect get_bounds() const { return bounds_; }
    void set_bounds(const Rect& r);
    /// old から変わった位置と大きさのプロパティを知らせる。
    void notify_bounds(const Rect& old);
    Rect get_local_rect() const { return Rect(0, 0, bounds_.width, bounds_.height); }
    Size get_preferred_size() const;
    Size get_minimum_size() const { return minimum_size_; }
    void set_minimum_size(const Size& v);
    Size get_maximum_size() const { return maximum_size_; }
    void set_maximum_size(const Size& v);
    Size get_minimum_hint() const;
    Layout* get_layout() const { return layout_; }

    bool get_visible() const { return visible_; }
    void set_visible(const bool& v);
    bool get_showing() const;
    /// 自分とウィンドウまでの祖先がすべて表示されるか(ウィンドウ自身は問わない)。
    bool shown_in_window() const;
    /// visible で、親に隠されていない(描く・配置する・クリックを受ける)。
    bool displayed() const { return visible_ && !hidden_by_parent_; }
    /// 親(ToolBar・Splitter・TabWidget)が自分の都合で隠す・戻す(visible は変えない。detail::WidgetAccess から使う)。
    void set_hidden_by_parent(bool hide);
    bool get_enabled() const { return enabled_; }
    void set_enabled(const bool& v);
    bool get_effectively_enabled() const;
    FocusPolicy get_focus_policy() const { return focus_policy_; }
    void set_focus_policy(const FocusPolicy& v) { focus_policy_ = v; }
    bool get_focused() const;
    void set_focused(const bool& v);
    MouseCursor get_cursor() const { return cursor_; }
    void set_cursor(const MouseCursor& v);
    bool get_accepts_text_input() const { return accepts_text_input_; }
    void set_accepts_text_input(const bool& v);
    bool get_accepts_drops() const { return accepts_drops_; }
    void set_accepts_drops(const bool& v) { accepts_drops_ = v; }
    float get_display_scale() const;
    void set_tooltip(const std::string& v) { tooltip_ = v; }
    ThemeOverride get_theme_override() const { return theme_override_; }
    void set_theme_override(const ThemeOverride& v);
    Theme get_theme() const;
    /// テーマを継ぐ元(親。ウィンドウは無し。ポップアップは開いた元のウィジェット)。
    const Widget* theme_parent() const;
    /// 自分と祖先の theme_override を out(アプリケーションのテーマ)に重ねる。上書きが 1 つも無ければ false(out は変えない)。
    bool compose_theme(Theme& out) const;
    std::vector<Action*> get_actions() const { return actions_; }

    Widget* parent_;
    std::vector<Widget*> children_;
    Rect bounds_;
    Size minimum_size_;
    Size maximum_size_;
    Layout* layout_;
    bool layout_dirty_;
    bool visible_;
    bool hidden_by_parent_;  ///< 親が自分の都合で隠している(visible_ とは別。displayed を見る)
    bool enabled_;
    bool accepts_text_input_;
    bool accepts_drops_;
    bool is_window_;
    FocusPolicy focus_policy_;
    MouseCursor cursor_;
    std::string tooltip_;
    ThemeOverride theme_override_;
    std::vector<Action*> actions_;
    PaintHandler on_paint_;
    NotifyHandler on_resize_;
    MouseHandler on_mouse_down_;
    MouseHandler on_mouse_up_;
    MouseHandler on_mouse_move_;
    NotifyHandler on_mouse_enter_;
    NotifyHandler on_mouse_leave_;
    WheelHandler on_mouse_wheel_;
    KeyHandler on_key_down_;
    KeyHandler on_key_up_;
    NotifyHandler on_focus_in_;
    NotifyHandler on_focus_out_;
    DragHandler on_drag_move_;
    NotifyHandler on_drag_leave_;
    DragHandler on_drop_;
    DragFinishHandler on_drag_finish_;
};

}  // namespace tokotachi

#endif
