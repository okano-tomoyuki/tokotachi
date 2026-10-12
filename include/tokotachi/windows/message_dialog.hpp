/// @file
/// tokotachi::MessageDialog の宣言。
#ifndef TOKOTACHI_WINDOWS_MESSAGE_DIALOG_HPP
#define TOKOTACHI_WINDOWS_MESSAGE_DIALOG_HPP

#include <string>
#include <vector>

#include "../core/export.hpp"
#include "dialog.hpp"

namespace tokotachi
{

class Label;

/// 左に出す印。
enum class MessageType
{
    INFORMATION,
    QUESTION,
    WARNING,
    CRITICAL
};

/// 文を見せて、ボタンで答えてもらうダイアログ(確認・警告。docs/adr/0028)。
///
/// @image html widget_message_dialog.png
///
/// @code
/// tokotachi::MessageDialog::show(window_main, "確認", "変更を保存しますか？",
///                          {tokotachi::DialogButton::SAVE, tokotachi::DialogButton::DISCARD, tokotachi::DialogButton::CANCEL},
///                          [](tokotachi::Object* sender)
///                          {
///                              tokotachi::MessageDialog* message_dialog = static_cast<tokotachi::MessageDialog*>(sender);
///                              if (message_dialog->result == tokotachi::DialogButton::SAVE) { ... }
///                          },
///                          tokotachi::MessageType::QUESTION);
/// @endcode
///
/// show は開いたダイアログを返し、閉じたら on_finish を呼んでから破棄する。
/// 自分で作るとき(new MessageDialog)は、add_button でボタンを加えて open する(破棄は持ち主の親のウィンドウに任せる)。
///
/// @ingroup windows
class TOKOTACHI_API MessageDialog : public Dialog
{
public:
    /// parent の上に出すメッセージのダイアログ(parent が所有する)。
    MessageDialog(Window* parent, const std::string& title, const std::string& text,
                  MessageType type = MessageType::INFORMATION);
    ~MessageDialog() override;

    /// 知らせる文(幅で折り返す)。
    Property<std::string> text;
    /// 種類(左の印の形と色)。
    Property<MessageType> type;

    /// 作って buttons を加えて開き、閉じたら on_finish を呼んで破棄する。buttons が空なら OK だけ。
    static MessageDialog* show(Window* parent, const std::string& title, const std::string& text,
                               const std::vector<DialogButton>& buttons = std::vector<DialogButton>(),
                               const NotifyHandler& on_finish = NotifyHandler(),
                               MessageType type = MessageType::INFORMATION);

private:
    class Impl;
    Impl* d_;
};

}  // namespace tokotachi

#endif
