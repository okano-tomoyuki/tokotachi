// メッセージのダイアログ。open はすぐに戻り、結果は on_finish で受け取る。
#include "gallery.hpp"

int main(int argc, char** argv)
{
    tokotachi::Application app(argc, argv);
    tokotachi::Window window("ダイアログ", 420, 240);
    window.visible = true;

    tokotachi::MessageDialog* ask = tokotachi::MessageDialog::show(
        &window, "確認", "変更を保存しますか？ 保存しないと、編集した内容は失われる。",
        std::vector<tokotachi::DialogButton>{tokotachi::DialogButton::SAVE, tokotachi::DialogButton::DISCARD,
                                            tokotachi::DialogButton::CANCEL},
        [&](tokotachi::Object* sender)
        {
            if (static_cast<tokotachi::Dialog*>(sender)->result == tokotachi::DialogButton::SAVE)
            {
                // 保存する
            }
            window.close();
        },
        tokotachi::MessageType::QUESTION);
    return run_gallery(app, window, argc, argv, ask);
}
