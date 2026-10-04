// 文を見せて、ボタンで答えてもらうダイアログ(確認・警告。docs/adr/0028)。
//
//   gui::MessageDialog::show(&window, "確認", "変更を保存しますか？",
//                            {gui::DialogButton::SAVE, gui::DialogButton::DISCARD, gui::DialogButton::CANCEL},
//                            [](gui::Object* sender)
//                            {
//                                gui::MessageDialog* d = static_cast<gui::MessageDialog*>(sender);
//                                if (d->result == gui::DialogButton::SAVE) { ... }
//                            },
//                            gui::MessageType::QUESTION);
//
// show は開いたダイアログを返し、閉じたら on_finish を呼んでから破棄する。
// 自分で作るとき(new MessageDialog)は、add_button でボタンを加えて open する(破棄は持ち主の親のウィンドウに任せる)。
#ifndef GUI_MESSAGE_DIALOG_HPP
#define GUI_MESSAGE_DIALOG_HPP

#include <string>
#include <vector>

#include "dialog.hpp"

namespace gui
{

class Label;

namespace detail
{
class MessageIcon;
}

// 左に出す印。
enum class MessageType
{
    INFORMATION,
    QUESTION,
    WARNING,
    CRITICAL
};

class MessageDialog : public Dialog
{
public:
    MessageDialog(Window* parent, const std::string& title, const std::string& text,
                  MessageType type = MessageType::INFORMATION);

    Property<std::string> text;
    Property<MessageType> type;

    // 作って buttons を加えて開き、閉じたら on_finish を呼んで破棄する。buttons が空なら OK だけ。
    static MessageDialog* show(Window* parent, const std::string& title, const std::string& text,
                               const std::vector<DialogButton>& buttons = std::vector<DialogButton>(),
                               const NotifyHandler& on_finish = NotifyHandler(),
                               MessageType type = MessageType::INFORMATION);

private:
    std::string get_text() const;
    void set_text(const std::string& v);
    MessageType get_type() const { return type_; }
    void set_type(const MessageType& v);

    MessageType type_;
    detail::MessageIcon* icon_;
    Label* label_;
};

}  // namespace gui

#endif
