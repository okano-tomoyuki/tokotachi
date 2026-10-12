/// @file
/// ドラッグ&ドロップのデータ・操作・イベント(docs/adr/0047)。
///
/// 同じウィンドウの中のウィジェットの間と、OS(エクスプローラー・ファイルマネージャー)から落としたファイル・文字に対応する。
/// 受け取るウィジェットは accepts_drops を true にし、on_drag_move で受け取るかを決め、on_drop で受け取る:
///
/// @code
/// label_drop->accepts_drops = true;
/// label_drop->on_drag_move = [](tokotachi::Object*, tokotachi::DragEvent& e) {
///     e.accepted = e.data.from_outside() || !e.data.text.empty();  // OS からのものは、落とすまで中身が分からない
/// };
/// label_drop->on_drop = [=](tokotachi::Object*, tokotachi::DragEvent& e) {
///     label_drop->text = e.data.files.empty() ? e.data.text : e.data.files[0];
/// };
/// @endcode
///
/// 始めるウィジェットは、マウスのボタンを押している間に Widget::start_drag を呼ぶ。
#ifndef TOKOTACHI_CORE_DRAG_HPP
#define TOKOTACHI_CORE_DRAG_HPP

#include <functional>
#include <map>
#include <string>
#include <vector>

#include "event.hpp"
#include "export.hpp"

namespace tokotachi
{

class Widget;

/// ドロップしたときの操作。
enum class DropAction
{
    NONE,  ///< 受け取らない
    COPY,  ///< 写す(元は残る)
    MOVE   ///< 移す(元は始めた側が消す)
};

/// ドラッグするデータ(値)。
/// @ingroup core_events
class TOKOTACHI_API DragData
{
public:
    DragData() : source_(nullptr), outside_(false) {}

    /// 文字。
    std::string text;
    /// ファイルのパス(UTF-8)。
    std::vector<std::string> files;
    /// アプリが決める名前の付いたデータ(例: "app/card-id" → "42")。
    std::map<std::string, std::string> formats;

    /// ドラッグを始めたウィジェット(OS からなら nullptr)。
    Widget* source() const { return source_; }
    /// OS から落としたものか。重ねている間(drag_move)は、OS が中身を知らせないので files・text は空。落としたとき(drop)に入る。
    bool from_outside() const { return outside_; }

private:
    friend struct DragAccess;

    Widget* source_;
    bool outside_;
};

/// ドラッグの位置での知らせ(drag_move・drop)。位置は受け取るウィジェットの左上を原点とする dp。
/// @ingroup core_events
struct TOKOTACHI_API DragEvent
{
    /// 位置(受け取るウィジェットの左上を原点とする dp)。
    Point pos;
    /// ドラッグしているデータ。
    const DragData& data;
    /// 押している修飾キー。
    Modifiers modifiers;
    /// 提案された操作(ウィンドウの中は MOVE、Ctrl を押していれば COPY。OS からは COPY)。受け取る側が変えられる。
    DropAction action;
    /// drag_move: 受け取るなら true のままにする(false にすると親へ送る)。drop: 受け取ったなら true のまま。
    bool accepted;

    /// ライブラリが作る。
    DragEvent(const Point& pos_, const DragData& data_, Modifiers modifiers_, DropAction action_)
        : pos(pos_), data(data_), modifiers(modifiers_), action(action_), accepted(true)
    {
    }

    /// 修飾キー m(組み合わせなら全部)を押しているか。
    bool has_modifier(Modifiers m) const { return (modifiers & m) == m; }
};

/// ドラッグの位置での知らせ(on_drag_move・on_drop)のハンドラ。
typedef std::function<void(Object* sender, DragEvent& e)> DragHandler;
/// ドラッグが終わった(action は受け取った側が決めた操作。受け取られなければ NONE)。
typedef std::function<void(Object* sender, DropAction action)> DragFinishHandler;

}  // namespace tokotachi

#endif
