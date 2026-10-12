/// @file
/// tokotachi::Label の宣言。
#ifndef TOKOTACHI_WIDGETS_LABEL_HPP
#define TOKOTACHI_WIDGETS_LABEL_HPP

#include <string>

#include "../core/export.hpp"
#include "../layout/alignment.hpp"
#include "../core/thread.hpp"
#include "../graphics/image.hpp"
#include "../graphics/text_layout.hpp"
#include "../graphics/theme.hpp"
#include "widget.hpp"

namespace tokotachi
{

/// テキストを表示する。改行文字で行を分け、word_wrap なら幅で折り返す(docs/adr/0019)。
/// icon を与えると、文字の前(段落の向きの始め)に画像を出す(docs/adr/0048)。
///
/// @image html widget_label.png
///
/// text の「&」の次の文字はニーモニック(docs/adr/0049)。Alt と一緒に押すと、buddy(無ければタブの順でラベルの次の部品)へ
/// フォーカスを移す。「&」そのものを出すには「&&」と書く。
///
/// @code
/// tokotachi::Label* label_name = new tokotachi::Label(panel, "名前(&N)");
/// tokotachi::LineEdit* line_edit_name = new tokotachi::LineEdit(panel);  // Alt+N でここへ
/// @endcode
///
/// @ingroup widgets_display
class TOKOTACHI_API Label : public Widget
{
public:
    explicit Label(Widget* parent = nullptr, const std::string& text = std::string());
    ~Label() override;

    Property<std::string> text;
    Property<Font> font;
    /// 文字の色(既定はテーマの TEXT。無効なときは TEXT_DISABLED で描く)。
    Property<ThemeColor> color;
    /// 文字の横のそろえ方(既定 START。FILL は START と同じ)。複数行なら各行をそろえる。
    Property<Align> alignment;
    /// 幅で折り返すか(既定 false)。折り返すと、望む高さは今の幅で折り返したときの高さになり、最小の幅は最も長い単語の幅になる。
    Property<bool> word_wrap;
    /// 文字の前に出す画像(既定は空で、出さない)。
    Property<Image> icon;
    /// icon を描く大きさ(論理座標の幅・高さ。既定 16)。
    Property<float> icon_size;
    /// ニーモニックで移る先(既定 nullptr: タブの順でラベルの次の部品)。その部品か子孫の最初の部品へ移る。
    /// タブの順は作った順なので、入力欄をラベルより先に作ったときは、これを指定する。
    /// 破棄されると nullptr に戻る。
    Property<Widget*> buddy;

protected:
    void paint(Canvas& canvas) override;
    void scale_changed() override;
    Size compute_preferred_size() const override;
    /// 文字が切れないよう、望む大きさを最小とする。
    Size compute_minimum_size() const override;
    void resized() override;
    Key mnemonic_key() const override;
    void mnemonic_activated() override;

private:
    class Impl;
    Impl* d_;
};

}  // namespace tokotachi

#endif
