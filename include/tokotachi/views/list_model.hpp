/// @file
/// tokotachi::ListModel の宣言。
#ifndef TOKOTACHI_VIEWS_LIST_MODEL_HPP
#define TOKOTACHI_VIEWS_LIST_MODEL_HPP

#include "../core/export.hpp"
#include "item.hpp"
#include "item_model.hpp"

namespace tokotachi
{

/// 項目をオブジェクト(Item)で持つ一覧のモデル(docs/adr/0026)。
///
/// @code
/// tokotachi::ListModel* list = new tokotachi::ListModel(window_main);
/// list->add_item("りんご");
/// tokotachi::Item* item = list->add_item("みかん");
/// item->checkable = true;
/// list_view_fruits->model = list;
/// @endcode
///
/// @ingroup views
class TOKOTACHI_API ListModel : public ItemModel
{
public:
    /// 所有者を渡す(所有者と一緒に破棄される)。
    explicit ListModel(Object* owner = nullptr);
    ~ListModel();

    /// 最後に加える。
    Item* add_item(const std::string& text);
    /// index の位置に加える(範囲の外なら最後)。
    Item* insert_item(int index, const std::string& text);
    /// 項目を除いて破棄する。
    void remove_item(Item* item);
    /// 項目を index の位置へ移す(移した後の位置。範囲の外なら最後)。
    void move_item(Item* item, int index);
    /// すべての項目を除いて破棄する。
    void clear();

    /// 項目(並んだ順)。
    ReadOnlyProperty<std::vector<Item*> > items;

    /// 項目の位置(このモデルの項目でなければ無効な ModelIndex)・位置の項目。
    ModelIndex index_of(const Item* item) const;
    /// index の項目(無効なら nullptr)。
    Item* item_at(const ModelIndex& index) const;

    int row_count(const ModelIndex& parent = ModelIndex()) const override;
    ModelIndex index(int row, const ModelIndex& parent = ModelIndex()) const override;
    std::string text(const ModelIndex& index, int column) const override;
    Image icon(const ModelIndex& index) const override;
    bool enabled(const ModelIndex& index) const override;
    bool checkable(const ModelIndex& index) const override;
    bool checked(const ModelIndex& index) const override;
    void set_checked(const ModelIndex& index, bool checked) override;
    ThemeColor text_color(const ModelIndex& index, int column) const override;
    Font font(const ModelIndex& index, int column) const override;
    bool editable(const ModelIndex& index, int column) const override;
    void set_text(const ModelIndex& index, int column, const std::string& text) override;
    /// 文字のバイト列の順(UTF-8 なので、コードポイントの順)で、同じ値の項目の順は保つ。
    void sort(int column, SortOrder order) override;
    /// 一覧の中の並べ替え(ドラッグ&ドロップ)。
    bool can_move_rows(const std::vector<ModelIndex>& indexes, const ModelIndex& parent, int row) const override;
    bool move_rows(const std::vector<ModelIndex>& indexes, const ModelIndex& parent, int row) override;
    ModelIndex locate(const ModelIndex& index) const override;

private:
    class Impl;
    Impl* d_;
};

}  // namespace tokotachi

#endif
