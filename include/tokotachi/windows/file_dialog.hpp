/// @file
/// tokotachi::FileDialog の宣言。
#ifndef TOKOTACHI_WINDOWS_FILE_DIALOG_HPP
#define TOKOTACHI_WINDOWS_FILE_DIALOG_HPP

#include <string>
#include <vector>

#include "../core/event.hpp"
#include "../core/export.hpp"
#include "../core/property.hpp"

namespace tokotachi
{

class Window;

enum class FileDialogMode
{
    OPEN,   ///< 既にあるファイルを選ぶ
    SAVE,   ///< 保存するファイルの名前を決める
    FOLDER  ///< フォルダーを選ぶ
};

/// ファイルの種類の絞り込み。pattern は拡張子を ; でつなぐ("png;jpg")。"*" はすべて。
/// @ingroup windows
struct TOKOTACHI_API FileFilter
{
    std::string name;
    std::string pattern;

    FileFilter() {}
    FileFilter(const std::string& name_, const std::string& pattern_) : name(name_), pattern(pattern_) {}

    bool operator==(const FileFilter& o) const { return name == o.name && pattern == o.pattern; }
    bool operator!=(const FileFilter& o) const { return !(*this == o); }
};

/// OS のファイル選択のダイアログ(開く・保存・フォルダー。docs/adr/0028)。SDL3 を通して、Windows では標準のダイアログ、
/// Linux では xdg-desktop-portal(無ければ zenity など)を使う。
///
/// @code
/// tokotachi::FileDialog* file_dialog_image = new tokotachi::FileDialog(&window, tokotachi::FileDialogMode::OPEN);   // window が所有する
/// file_dialog_image->filters = std::vector<tokotachi::FileFilter>{tokotachi::FileFilter("画像", "png;jpg"), tokotachi::FileFilter("すべて", "*")};
/// file_dialog_image->on_finish = [=](tokotachi::Object*)
/// {
///     const std::vector<std::string> paths = file_dialog_image->paths;   // 取り消したら空
///     ...
/// };
/// file_dialog_image->open();   // すぐに戻る
/// @endcode
///
/// on_finish はメインスレッドで、イベントの処理の中で呼ぶ。開いている間に FileDialog を破棄したら、結果は捨てる。
///
/// @ingroup windows
class TOKOTACHI_API FileDialog : public Object
{
public:
    /// parent の上に出す(parent が所有する。nullptr なら親を持たない)。
    explicit FileDialog(Window* parent = nullptr, FileDialogMode mode = FileDialogMode::OPEN);
    ~FileDialog();

    Property<FileDialogMode> mode;
    /// ダイアログの題名(空なら OS の既定)。
    Property<std::string> title;
    /// 絞り込み(FOLDER では使わない)。
    Property<std::vector<FileFilter> > filters;
    /// 最初に開く場所(フォルダー。SAVE ではファイルの名前を含めてよい)。
    Property<std::string> location;
    /// 複数を選べるか(OPEN・FOLDER)。
    Property<bool> multi_enabled;

    /// 選んだパス(取り消した・失敗したら空)。
    ReadOnlyProperty<std::vector<std::string> > paths;
    /// 失敗したときの理由(成功した・取り消したなら空)。Linux で xdg-desktop-portal も zenity も無いときなど。
    ReadOnlyProperty<std::string> error;
    /// 選んだ絞り込みの番号(分からなければ -1)。
    ReadOnlyProperty<int> selected_filter;
    /// 開いているか。
    ReadOnlyProperty<bool> running;

    /// 閉じた(選んだ・取り消した・失敗した)。sender はこの FileDialog。
    Property<NotifyHandler> on_finish;

    /// 開く(すぐに戻る)。開いている間は何もしない。
    void open();

private:
    template <typename>
    friend struct detail::ImplAccess;
    class Impl;
    Impl* d_;
};

}  // namespace tokotachi

#endif
