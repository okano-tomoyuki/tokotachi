// OS のファイル選択のダイアログ(開く・保存・フォルダー。docs/adr/0028)。SDL3 を通して、Windows では標準のダイアログ、
// Linux では xdg-desktop-portal(無ければ zenity など)を使う。
//
//   gui::FileDialog* chooser = new gui::FileDialog(&window, gui::FileDialogMode::OPEN);   // window が所有する
//   chooser->filters = std::vector<gui::FileFilter>{gui::FileFilter("画像", "png;jpg"), gui::FileFilter("すべて", "*")};
//   chooser->on_finish = [=](gui::Object*)
//   {
//       const std::vector<std::string> paths = chooser->paths;   // 取り消したら空
//       ...
//   };
//   chooser->open();   // すぐに戻る
//
// on_finish はメインスレッドで、イベントの処理の中で呼ぶ。開いている間に FileDialog を破棄したら、結果は捨てる。
#ifndef GUI_FILE_DIALOG_HPP
#define GUI_FILE_DIALOG_HPP

#include <string>
#include <vector>

#include "event.hpp"
#include "property.hpp"

namespace gui
{

class Window;

enum class FileDialogMode
{
    OPEN,   // 既にあるファイルを選ぶ
    SAVE,   // 保存するファイルの名前を決める
    FOLDER  // フォルダーを選ぶ
};

// ファイルの種類の絞り込み。pattern は拡張子を ; でつなぐ("png;jpg")。"*" はすべて。
struct FileFilter
{
    std::string name;
    std::string pattern;

    FileFilter() {}
    FileFilter(const std::string& name_, const std::string& pattern_) : name(name_), pattern(pattern_) {}

    bool operator==(const FileFilter& o) const { return name == o.name && pattern == o.pattern; }
    bool operator!=(const FileFilter& o) const { return !(*this == o); }
};

class FileDialog : public Object
{
public:
    // parent の上に出す(parent が所有する。nullptr なら親を持たない)。
    explicit FileDialog(Window* parent = nullptr, FileDialogMode mode = FileDialogMode::OPEN);
    ~FileDialog();

    Property<FileDialogMode> mode;
    // ダイアログの題名(空なら OS の既定)。
    Property<std::string> title;
    // 絞り込み(FOLDER では使わない)。
    Property<std::vector<FileFilter> > filters;
    // 最初に開く場所(フォルダー。SAVE ではファイルの名前を含めてよい)。
    Property<std::string> location;
    // 複数を選べるか(OPEN・FOLDER)。
    Property<bool> allow_many;

    // 選んだパス(取り消した・失敗したら空)。
    ReadOnlyProperty<std::vector<std::string> > paths;
    // 失敗したときの理由(成功した・取り消したなら空)。Linux で xdg-desktop-portal も zenity も無いときなど。
    ReadOnlyProperty<std::string> error;
    // 選んだ絞り込みの番号(分からなければ -1)。
    ReadOnlyProperty<int> selected_filter;
    // 開いているか。
    ReadOnlyProperty<bool> running;

    // 閉じた(選んだ・取り消した・失敗した)。sender はこの FileDialog。
    Property<NotifyHandler> on_finish;

    // 開く(すぐに戻る)。開いている間は何もしない。
    void open();

private:
    std::vector<std::string> get_paths() const { return paths_; }
    int get_selected_filter() const { return selected_filter_; }
    std::string get_error() const { return error_; }
    bool get_running() const { return request_ != 0; }
    void set_mode(const FileDialogMode& v) { mode_ = v; }
    void set_title(const std::string& v) { title_ = v; }
    void set_filters(const std::vector<FileFilter>& v) { filters_ = v; }
    void set_location(const std::string& v) { location_ = v; }
    void set_allow_many(const bool& v) { allow_many_ = v; }
    // 結果が届いた(メインスレッド)。
    void finished(const std::vector<std::string>& result, int filter, const std::string& failure);

    friend struct FileDialogAccess;

    Window* parent_;
    FileDialogMode mode_;
    std::string title_;
    std::vector<FileFilter> filters_;
    std::string location_;
    bool allow_many_;
    std::vector<std::string> paths_;
    int selected_filter_;
    std::string error_;
    NotifyHandler on_finish_;
    int request_;  // 開いている要求の番号(0 なら開いていない)
};

}  // namespace gui

#endif
