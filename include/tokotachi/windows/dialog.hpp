/// @file
/// モーダルのダイアログ(docs/adr/0028)。親のウィンドウの上に別のウィンドウとして開き、閉じるまで親への入力を止める。
///
/// @code
/// tokotachi::Dialog* dialog = new tokotachi::Dialog(&window, "名前の変更");    // 親のウィンドウが所有する
/// tokotachi::BoxLayout* box = new tokotachi::BoxLayout(dialog);                 // 中身はレイアウトで並べる
/// tokotachi::LineEdit* name = new tokotachi::LineEdit(dialog);
/// box->add(name);
/// dialog->add_button(tokotachi::DialogButton::OK);
/// dialog->add_button(tokotachi::DialogButton::CANCEL);
/// dialog->on_finish = [=](tokotachi::Object*)
/// {
///     if (dialog->result == tokotachi::DialogButton::OK) { ... name->text ... }
/// };
/// dialog->open();   // すぐに戻る。結果は on_finish で受け取る
/// @endcode
///
/// ボタンは下端に、OS の慣習の順に並べる(Windows・KDE は「OK キャンセル」、GNOME などは「キャンセル OK」)。
/// Enter は最初の決定のボタン(OK・はい・保存)を、Esc・ウィンドウの閉じるボタンは取り消しのボタン(キャンセル・閉じる。
/// ボタンが 1 つだけならそのボタン)を押す。
/// 大きさは開くときに中身に合わせる(幅は 320〜560dp。minimum_size で広げられる)。
#ifndef TOKOTACHI_WINDOWS_DIALOG_HPP
#define TOKOTACHI_WINDOWS_DIALOG_HPP

#include <string>
#include <utility>
#include <vector>

#include "../widgets/button.hpp"
#include "window.hpp"

namespace tokotachi
{

/// ダイアログのボタンの役割(結果にもなる)。
enum class DialogButton
{
    NONE,     ///< ボタンを押さずに閉じた(取り消しのボタンが無いときの閉じるボタンなど)
    OK,       ///< 決定
    YES,      ///< 決定
    SAVE,     ///< 決定
    NO,       ///< 否定
    DISCARD,  ///< 否定(保存しない)
    CANCEL,   ///< 取り消し
    CLOSE     ///< 取り消し
};

class Dialog : public Window
{
public:
    /// parent の上に出すダイアログ(parent が所有する)。
    Dialog(Window* parent, const std::string& title);
    ~Dialog();

    /// 役割 button のボタンを加える(text が空なら、言語に合わせた標準の文字)。押すと finish(button)。
    Button* add_button(DialogButton button, const std::string& text = std::string());

    /// モーダルで開く(すぐに戻る)。
    void open();
    /// result を決めて閉じ、on_finish を呼ぶ。
    void finish(DialogButton result);
    /// 開いていれば、取り消しのボタンの結果で閉じる(ウィンドウの閉じるボタンと同じ)。
    void close() override;

    /// 閉じたときの結果(開いている間・開く前は NONE)。
    ReadOnlyProperty<DialogButton> result;
    /// 閉じた(sender はこのダイアログ)。このハンドラの中で破棄するときは delete_later を使う。
    Property<NotifyHandler> on_finish;

protected:
    void key_down(KeyEvent& e) override;
    void arrange_children() override;
    void child_removed(Widget* child) override;
    Size compute_preferred_size() const override;
    Size compute_minimum_size() const override;

private:
    DialogButton get_result() const { return result_; }
    /// 並べる順に並べたボタン。
    std::vector<std::pair<DialogButton, Button*> > ordered_buttons() const;
    Button* button_for(DialogButton role) const;
    /// Esc・閉じるボタンのときの結果。
    DialogButton cancel_result() const;
    float button_row_height() const;
    float button_row_width() const;

    Window* parent_window_;
    std::vector<std::pair<DialogButton, Button*> > buttons_;
    DialogButton result_;
    NotifyHandler on_finish_;
    bool running_;
};

}  // namespace tokotachi

#endif
