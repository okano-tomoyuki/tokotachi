/// @file
/// tokotachi::Widget の宣言。
#ifndef TOKOTACHI_WIDGETS_WIDGET_HPP
#define TOKOTACHI_WIDGETS_WIDGET_HPP

#include <string>
#include <vector>

#include "../core/drag.hpp"
#include "../core/event.hpp"
#include "../core/export.hpp"
#include "../core/geometry.hpp"
#include "../core/property.hpp"
#include "../graphics/theme.hpp"

namespace tokotachi
{

class Canvas;
class Layout;
class Action;
class Window;

/// フォーカスの得方。
enum class FocusPolicy
{
    NONE,
    TAB,           ///< Tab キーでだけフォーカスを得る
    CLICK,         ///< クリックでだけフォーカスを得る
    TAB_AND_CLICK  ///< Tab キーとクリックの両方
};

/// マウスのカーソルの形。
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

/// ウィジェットの基底(docs/adr/0010、0012、0013)。
///
/// 所有: 親が子を所有する(親が Object の所有者を兼ねる。docs/adr/0015)。new Widget(parent) で作り、親が破棄されるときに
/// 子も delete される。子を先に delete してもよい(親の一覧から外れる)。親を持つウィジェットをスタックに置いてはいけない。
/// ウィジェットはメインスレッドだけで扱う。
///
/// イベント: 利用者は on_xxx にハンドラを代入する。派生クラスは protected の仮想関数(paint・mouse_down 等)を上書きし、
/// ハンドラを呼ぶために基底の実装を呼ぶ。
///
/// @ingroup widgets_base
class TOKOTACHI_API Widget : public Object
{
public:
    /// parent の子として作る(parent が所有する)。nullptr なら、レイアウトに add する・タブに加えるなどで子になる。
    explicit Widget(Widget* parent = nullptr);
    virtual ~Widget();

    // --- ツリー ---
    /// 代入すると親を付け替える(所有者も移る。docs/adr/0018)。ウィンドウは付け替えられない。
    /// 自分の子孫を親にすることはできない(無視する)。nullptr にすると親も所有者も無くなる(利用者が破棄する)。
    Property<Widget*> parent;
    /// 子(作った順。タブの順もこの順)。
    ReadOnlyProperty<std::vector<Widget*> > children;
    /// このウィジェットが属するウィンドウ(ウィンドウ自身なら自分)。どのウィンドウにも属していなければ nullptr。
    ReadOnlyProperty<Window*> window;

    // --- 位置と大きさ(親の左上からの相対、dp) ---
    /// 左端の位置(親の左上からの相対、dp)。
    Property<float> x;
    /// 上端の位置(親の左上からの相対、dp)。
    Property<float> y;
    /// 幅(dp)。
    Property<float> width;
    /// 高さ(dp)。
    Property<float> height;
    /// 位置と大きさ(親の左上からの相対、dp)。レイアウトが管理する子では、次の配置で上書きされる。
    Property<Rect> bounds;
    /// 自分の座標系での矩形(左上が原点)。
    ReadOnlyProperty<Rect> local_rect;
    // --- レイアウト(docs/adr/0017) ---
    /// 望む大きさ(minimum_hint 以上 maximum_size 以下)。
    ReadOnlyProperty<Size> preferred_size;
    /// 利用者が決める最小・最大の大きさ(既定は 0 と無制限)。
    Property<Size> minimum_size;
    /// 利用者が決める最大の大きさ(既定は無制限)。
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
    /// 操作できるか(既定 true)。false なら薄く描き、入力を受けない(子孫も)。
    Property<bool> enabled;
    /// 自分と祖先がすべて enabled か。
    ReadOnlyProperty<bool> effectively_enabled;
    /// フォーカスの得方(Widget の既定は NONE。入力欄・ボタンなどは TAB_AND_CLICK)。
    Property<FocusPolicy> focus_policy;
    /// true を代入するとフォーカスを得る(focus_policy が NONE なら何もしない)。false を代入するとフォーカスを外す。
    Property<bool> focused;
    /// ポインタが乗ったときのカーソルの形(既定 ARROW)。
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
    /// Action を外す。
    void remove_action(Action* action);
    /// 加えた Action(加えた順)。
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
    /// 大きさが変わった。
    Property<NotifyHandler> on_resize;
    /// マウスのボタンを押した。受け取らなければ(accepted を false にすると)親へ送る。
    Property<MouseHandler> on_mouse_down;
    /// マウスのボタンを離した(押したウィジェットに届く)。
    Property<MouseHandler> on_mouse_up;
    /// マウスが動いた(ボタンを押している間は、押したウィジェットに届く)。
    Property<MouseHandler> on_mouse_move;
    /// ポインタが乗った。
    Property<NotifyHandler> on_mouse_enter;
    /// ポインタが離れた。
    Property<NotifyHandler> on_mouse_leave;
    /// ホイール。受け取らなければ(ハンドラが無いか accepted を false にすると)親へ送る。
    Property<WheelHandler> on_mouse_wheel;
    /// キーを押した(フォーカスのあるウィジェットに届く)。受け取らなければ親へ送る。
    Property<KeyHandler> on_key_down;
    /// キーを離した。
    Property<KeyHandler> on_key_up;
    /// フォーカスを得た。
    Property<NotifyHandler> on_focus_in;
    /// フォーカスを失った。
    Property<NotifyHandler> on_focus_out;

    // --- 操作 ---
    /// 描き直しを依頼する(次のイベントの処理の後にまとめて描く)。
    void invalidate();
    /// r(自分の座標系)の描き直しを依頼する。
    void invalidate(const Rect& r);

    /// 配置のやり直しを依頼する(望む大きさが変わったとき。次の描画の前にまとめて配置する)。
    void request_layout();
    /// 依頼されている配置を、すぐに行う(配置の結果をすぐに読みたいとき)。
    void apply_layout();

    /// 自分の座標系の点を、ウィンドウの座標系にする。
    Point map_to_window(const Point& p) const;
    /// ウィンドウの座標系の点を、自分の座標系にする。
    Point map_from_window(const Point& p) const;
    /// p(自分の座標系)にある、表示されている最も深い子孫。無ければ nullptr。
    Widget* child_at(const Point& p) const;
    /// フォーカスを、タブの順で自分の後ろにある最初の部品へ移す(自分の子孫は除く)。移せたら true。
    bool focus_next_after();
    /// フォーカスを、自分か子孫のうちタブの順で最初の部品へ移す。移せたら true。
    bool focus_first_within();

protected:
    /// 自分を描く(座標は自分の左上が原点。既定の実装は on_paint を呼ぶ)。子はこの後に描かれる。
    virtual void paint(Canvas& canvas);
    /// 大きさが変わった(既定の実装は on_resize を呼ぶ)。
    virtual void resized();
    /// マウスのボタンを押した(既定の実装は on_mouse_down を呼ぶ。ハンドラが無ければ accepted を false にする)。
    virtual void mouse_down(MouseEvent& e);
    /// マウスのボタンを離した(既定の実装は on_mouse_up を呼ぶ。ハンドラが無ければ親へ送る)。
    virtual void mouse_up(MouseEvent& e);
    /// マウスが動いた(既定の実装は on_mouse_move を呼ぶ)。
    virtual void mouse_move(MouseEvent& e);
    /// ポインタが乗った(既定の実装は on_mouse_enter を呼ぶ)。
    virtual void mouse_enter();
    /// ポインタが離れた(既定の実装は on_mouse_leave を呼ぶ)。
    virtual void mouse_leave();
    /// ホイールを回した(既定の実装は on_mouse_wheel を呼ぶ。ハンドラが無ければ親へ送る)。
    virtual void mouse_wheel(WheelEvent& e);
    /// キーを押した(既定の実装は on_key_down を呼ぶ。ハンドラが無ければ親へ送る)。
    virtual void key_down(KeyEvent& e);
    /// キーを離した(既定の実装は on_key_up を呼ぶ。ハンドラが無ければ親へ送る)。
    virtual void key_up(KeyEvent& e);
    /// 文字が入力された(accepts_text_input のとき)。
    virtual void text_input(TextInputEvent& e);
    /// IME の変換中の文字が変わった(accepts_text_input のとき)。
    virtual void text_editing(TextEditingEvent& e);
    /// フォーカスを得た(既定の実装は on_focus_in を呼ぶ)。
    virtual void focus_in();
    /// フォーカスを失った(既定の実装は on_focus_out を呼ぶ)。
    virtual void focus_out();
    /// ドラッグ&ドロップ(accepts_drops のとき。docs/adr/0047)。既定の実装はハンドラを呼ぶ。
    virtual void drag_move(DragEvent& e);
    /// ドラッグが離れた(既定の実装は on_drag_leave を呼ぶ)。
    virtual void drag_leave();
    /// 落とした(既定の実装は on_drop を呼ぶ)。
    virtual void drop(DragEvent& e);
    /// 自分が始めたドラッグが終わった。既定の実装は on_drag_finish を呼ぶ。
    virtual void drag_finished(DropAction action);
    /// 倍率が変わった(テキストのレイアウトなど、ピクセルに依存するキャッシュを捨てる)。
    virtual void scale_changed();
    /// アプリケーションのテーマが変わった(色・寸法から作ったキャッシュを捨てる)。この後、描き直しと配置のやり直しが行われる。
    virtual void theme_changed();
    /// 子を配置する(配置のやり直しの印が付いているときに、描画の前に呼ばれる)。既定の実装はレイアウトに任せる。
    /// 子の配置を自分で決めるウィジェット(ScrollArea 等)が上書きする(docs/adr/0021)。
    virtual void arrange_children();

    /// 望む大きさ・ウィジェットが決める最小の大きさ。既定の実装は、レイアウトがあればその大きさ、無ければ 0。
    virtual Size compute_preferred_size() const;
    /// ウィジェットが決める最小の大きさ(既定の実装は、レイアウトがあればその最小の大きさ、無ければ 0)。
    virtual Size compute_minimum_size() const;

private:
    template <typename>
    friend struct detail::ImplAccess;
    class Impl;
    Impl* d_;
};

}  // namespace tokotachi

#endif
