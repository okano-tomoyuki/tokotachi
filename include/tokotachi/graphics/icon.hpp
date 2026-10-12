/// @file
/// よく使うアイコン(docs/adr/0054)。Lucide(https://lucide.dev/)の線画を、24 × 24 の格子のパスとして持つ。
///
/// @code
/// action_save->icon = tokotachi::Image::icon(tokotachi::Icon::SAVE);
/// @endcode
///
/// Image::icon で画像にする。描くときは、部品の文字の色で塗り、描く大きさに合わせて線を引く(ぼやけない)。
/// 名前は Lucide の名前を大文字にし、- を _ にしたもの(folder-open → FOLDER_OPEN)。
// tools/icons/generate.mjs が lucide-static 1.48.0(ISC。一部は Feather の MIT。THIRD_PARTY_LICENSES.txt)から生成した。手で直さない。
#ifndef TOKOTACHI_GRAPHICS_ICON_HPP
#define TOKOTACHI_GRAPHICS_ICON_HPP

namespace tokotachi
{

enum class Icon
{
    // ファイル
    FILE,  ///< file
    FILE_PLUS,  ///< file-plus
    FILE_TEXT,  ///< file-text
    FOLDER,  ///< folder
    FOLDER_OPEN,  ///< folder-open
    SAVE,  ///< save
    PRINTER,  ///< printer
    DOWNLOAD,  ///< download
    UPLOAD,  ///< upload
    // 編集
    UNDO_2,  ///< undo-2
    REDO_2,  ///< redo-2
    SCISSORS,  ///< scissors
    COPY,  ///< copy
    CLIPBOARD_PASTE,  ///< clipboard-paste
    TRASH,  ///< trash
    PENCIL,  ///< pencil
    PLUS,  ///< plus
    MINUS,  ///< minus
    X,  ///< x
    CHECK,  ///< check
    // 移動
    ARROW_LEFT,  ///< arrow-left
    ARROW_RIGHT,  ///< arrow-right
    ARROW_UP,  ///< arrow-up
    ARROW_DOWN,  ///< arrow-down
    CHEVRON_LEFT,  ///< chevron-left
    CHEVRON_RIGHT,  ///< chevron-right
    CHEVRON_UP,  ///< chevron-up
    CHEVRON_DOWN,  ///< chevron-down
    HOUSE,  ///< house
    MENU,  ///< menu
    ELLIPSIS,  ///< ellipsis
    REFRESH_CW,  ///< refresh-cw
    EXTERNAL_LINK,  ///< external-link
    // 表示
    SEARCH,  ///< search
    ZOOM_IN,  ///< zoom-in
    ZOOM_OUT,  ///< zoom-out
    EYE,  ///< eye
    EYE_OFF,  ///< eye-off
    FUNNEL,  ///< funnel
    SETTINGS,  ///< settings
    SLIDERS_HORIZONTAL,  ///< sliders-horizontal
    LIST,  ///< list
    LAYOUT_GRID,  ///< layout-grid
    // 状態
    INFO,  ///< info
    TRIANGLE_ALERT,  ///< triangle-alert
    CIRCLE_X,  ///< circle-x
    CIRCLE_CHECK,  ///< circle-check
    CIRCLE_QUESTION_MARK,  ///< circle-question-mark
    BELL,  ///< bell
    // 再生
    PLAY,  ///< play
    PAUSE,  ///< pause
    SQUARE,  ///< square
    SKIP_BACK,  ///< skip-back
    SKIP_FORWARD,  ///< skip-forward
    VOLUME_2,  ///< volume-2
    VOLUME_X,  ///< volume-x
    // その他
    USER,  ///< user
    USERS,  ///< users
    MAIL,  ///< mail
    CALENDAR,  ///< calendar
    CLOCK,  ///< clock
    LOCK,  ///< lock
    LOCK_OPEN,  ///< lock-open
    STAR,  ///< star
    HEART,  ///< heart
    LINK,  ///< link
    IMAGE,  ///< image
    LOG_IN,  ///< log-in
    LOG_OUT,  ///< log-out
    SUN,  ///< sun
    MOON,  ///< moon
};

}  // namespace tokotachi

#endif
