// ポップアップ: 持ち主のウィジェットのウィンドウに相対的な位置に出る、枠の無いウィンドウ(docs/adr/0011、0022)。
// メニュー・ComboBox の一覧・ツールチップの土台。
//
//   tokotachi::Popup* popup = new tokotachi::Popup(button);              // button と一緒に破棄される
//   new tokotachi::Label(popup, "中身");                            // 中身は Popup を親にして作る(レイアウトも付けられる)
//   popup->open(button->local_rect, tokotachi::Size(200, 120));    // button の下に開く
//
// Wayland では、ポップアップは親のウィンドウからの相対の位置でしか置けない(docs/adr/0002)。画面からはみ出すときの
// 位置の調整は、OS(コンポジタ)・SDL3 が行う。メニューの種類のポップアップは、開くとキーボードのフォーカスを得て、
// 外側を押すと閉じる(押したことは、外側のウィンドウには送らない)。
#ifndef TOKOTACHI_WINDOWS_POPUP_HPP
#define TOKOTACHI_WINDOWS_POPUP_HPP

#include "window.hpp"

namespace tokotachi
{

enum class PopupKind
{
    MENU,    // メニュー・一覧(キーボードのフォーカスを得る。外側を押すと閉じる)
    TOOLTIP  // ツールチップ(入力を受けない)
};

enum class PopupPlacement
{
    BELOW,  // anchor の左下に、左上をそろえる
    RIGHT,  // anchor の右上に、左上をそろえる(サブメニュー)
    AT      // anchor の左上に、左上をそろえる(右クリックの位置など)
};

class Popup : public Window
{
public:
    // owner_widget(ウィンドウに属するウィジェット)のウィンドウの上に出す。owner_widget が破棄されると一緒に破棄される。
    // owner_object を渡すと、所有者はそちらになる(Menu のように、開く位置の基準と持ち主が違うとき)。
    // その場合も、owner_widget のウィンドウより先に破棄すること(ウィンドウが破棄されると、OS のポップアップも消えるため)。
    explicit Popup(Widget* owner_widget, PopupKind kind = PopupKind::MENU, Object* owner_object = nullptr);
    ~Popup();

    ReadOnlyProperty<Widget*> owner_widget;
    ReadOnlyProperty<PopupKind> kind;

    // owner_widget の座標系の矩形 anchor を基準に、size(dp)の大きさで開く。開いていれば位置と大きさを変える。
    void open(const Rect& anchor, const Size& size, PopupPlacement placement = PopupPlacement::BELOW);
    // 閉じる(このポップアップから開いたポップアップも閉じる)。
    void close() override;

    // 閉じた(close・外側のクリック・OS による終了のどれでも)。
    Property<NotifyHandler> on_closed;

protected:
    void paint(Canvas& canvas) override;

private:
    Widget* get_owner_widget() const { return owner_widget_; }
    PopupKind get_kind() const { return kind_; }

    Widget* owner_widget_;
    PopupKind kind_;
    NotifyHandler on_closed_;
};

}  // namespace tokotachi

#endif
