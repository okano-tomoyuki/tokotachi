/// @file
/// tokotachi::ItemModel の宣言。
#ifndef TOKOTACHI_VIEWS_ITEM_MODEL_HPP
#define TOKOTACHI_VIEWS_ITEM_MODEL_HPP

#include <string>
#include <vector>

#include "../core/drag.hpp"
#include "../graphics/image.hpp"
#include "../graphics/text_layout.hpp"
#include "../graphics/theme.hpp"
#include "../core/property.hpp"

namespace tokotachi
{

class ItemModel;
class ItemView;

/// モデルの中の項目の位置: 親の中での行の番号と、モデルが決める識別子(TreeModel・ListModel では項目のアドレス)。
/// 項目を加える・除くと行の番号はずれるので、長く持たない(ビューは知らせを受けて直す)。
/// @ingroup views
class ModelIndex
{
public:
    ModelIndex() : row_(-1), id_(nullptr) {}

    bool is_valid() const { return row_ >= 0; }
    int row() const { return row_; }
    void* id() const { return id_; }

private:
    friend class ItemModel;
    friend class ItemView;

    ModelIndex(int row, void* id) : row_(row), id_(id) {}

    int row_;
    void* id_;
};

/// 比べる(メンバにしないのは、view->current == index のように Property<ModelIndex> とも比べられるようにするため)。
inline bool operator==(const ModelIndex& a, const ModelIndex& b) { return a.row() == b.row() && a.id() == b.id(); }
inline bool operator!=(const ModelIndex& a, const ModelIndex& b) { return !(a == b); }
inline bool operator<(const ModelIndex& a, const ModelIndex& b)
{
    return a.id() != b.id() ? a.id() < b.id() : a.row() < b.row();
}

enum class SortOrder
{
    ASCENDING,
    DESCENDING
};

/// 項目の一覧・木のデータ(モデル)。ListView・TreeView が表示する(docs/adr/0026)。
///
/// 項目の数だけオブジェクトを作りたくないとき(数十万行・データベースの結果など)は ItemModel を派生し、
/// row_count と text を実装する(表示するのは見えている行だけで、そのときに問い合わせる):
///
/// @code
/// class LogModel : public tokotachi::ItemModel
/// {
/// public:
///     int row_count(const tokotachi::ModelIndex& parent) const override { return parent.is_valid() ? 0 : lines.size(); }
///     std::string text(const tokotachi::ModelIndex& index, int column) const override { return lines[index.row()]; }
///     ...
/// };
/// @endcode
///
/// 手軽に使うときは、項目をオブジェクトで持つ ListModel・TreeModel(list_model.hpp・tree_model.hpp)を使う。
///
/// @ingroup views
class ItemModel : public Object
{
public:
    ~ItemModel();

    /// parent の子の数(parent が無効なら一番上の数)。一覧のモデルは、有効な parent に 0 を返す。
    virtual int row_count(const ModelIndex& parent = ModelIndex()) const = 0;
    /// parent の row 番目の子。既定の実装は一番上の行だけを、識別子 nullptr で返す(一覧のモデル)。
    virtual ModelIndex index(int row, const ModelIndex& parent = ModelIndex()) const;
    /// 親(一番上の行なら無効な ModelIndex)。木のモデルは上書きする。
    virtual ModelIndex parent_of(const ModelIndex& index) const;

    /// column 列目の文字。
    virtual std::string text(const ModelIndex& index, int column) const = 0;
    /// 1 列目の文字の前に描く画像(既定は無し)。
    virtual Image icon(const ModelIndex& index) const;
    /// false の項目は薄く描き、選べない(既定は true)。
    virtual bool enabled(const ModelIndex& index) const;
    /// チェックボックスを付けるか・付いているか(既定は付けない)。
    virtual bool checkable(const ModelIndex& index) const;
    virtual bool checked(const ModelIndex& index) const;
    /// 利用者がチェックボックスを押した。値を変えたら notify_data_changed で知らせる。
    virtual void set_checked(const ModelIndex& index, bool checked);

    /// column 列目の文字の色(既定は ColorRole::TEXT。ビューの標準の色)。無効な項目と、フォーカスのあるビューで選んでいる行は、
    /// ビューの色で描く(選択の色の上で読めるように)。
    virtual ThemeColor text_color(const ModelIndex& index, int column) const;
    /// column 列目の文字のフォント(既定は Font())。行の高さは既定のフォントで決まるので、大きいフォントははみ出た分が切れる。
    virtual Font font(const ModelIndex& index, int column) const;

    /// column 列目の文字をビューで編集できるか(既定は false。docs/adr/0046)。
    virtual bool editable(const ModelIndex& index, int column) const;
    /// 利用者がビューで column 列目の文字を編集した。値を変えたら notify_data_changed で知らせる(既定は何もしない)。
    virtual void set_text(const ModelIndex& index, int column, const std::string& text);

    /// ドラッグ&ドロップ(docs/adr/0047)。row は parent の子の位置(その前に入れる。-1 なら終わり)。
    /// 同じモデルの項目 indexes(表示の順)を移せるか・移す(既定は移せない)。移したら notify_layout_changed などで知らせる。
    virtual bool can_move_rows(const std::vector<ModelIndex>& indexes, const ModelIndex& parent, int row) const;
    virtual bool move_rows(const std::vector<ModelIndex>& indexes, const ModelIndex& parent, int row);
    /// ほか(別のモデル・ウィジェット・OS)から落としたデータを受け取れるか・受け取る(既定は受け取らない)。
    /// OS からのデータは、落とすまで中身が分からない(DragData::from_outside)。
    virtual bool can_drop(const DragData& data, const ModelIndex& parent, int row) const;
    virtual bool drop_data(const DragData& data, DropAction action, const ModelIndex& parent, int row);

    /// column 列目の文字で並べ替える(既定は何もしない)。並べ替えたら notify_layout_changed で知らせる。
    virtual void sort(int column, SortOrder order);
    /// 並びが変わった後の、index の項目の今の位置(識別子で探す)。分からなければ無効な ModelIndex(既定)。
    virtual ModelIndex locate(const ModelIndex& index) const;

protected:
    explicit ItemModel(Object* owner = nullptr);

    ModelIndex make_index(int row, void* id) const { return ModelIndex(row, id); }

    /// 変更をビューに知らせる。
    /// 加えた後に呼ぶ: parent の子の first 番目から count 個を加えた。
    void notify_rows_inserted(const ModelIndex& parent, int first, int count);
    /// 除く前に呼ぶ: parent の子の first 番目から count 個(とその子孫)を除く。
    void notify_rows_removing(const ModelIndex& parent, int first, int count);
    /// 項目の中身(文字・画像・チェックなど)が変わった。
    void notify_data_changed(const ModelIndex& index);
    /// 項目は同じで並びが変わった(並べ替えた)。ビューは locate で選択などを引き直す。
    void notify_layout_changed();
    /// すべてが変わった。ビューは選択などを捨てる。
    void notify_reset();

private:
    friend class ItemView;

    std::vector<ItemView*> views_;
};

}  // namespace tokotachi

#endif
