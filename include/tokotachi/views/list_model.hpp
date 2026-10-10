/// @file
/// 項目をオブジェクト(Item)で持つ一覧のモデル(docs/adr/0026)。
///
/// @code
/// tokotachi::ListModel* list = new tokotachi::ListModel(&window);
/// list->add_item("りんご");
/// tokotachi::Item* item = list->add_item("みかん");
/// item->checkable = true;
/// view->model = list;
/// @endcode
#ifndef TOKOTACHI_VIEWS_LIST_MODEL_HPP
#define TOKOTACHI_VIEWS_LIST_MODEL_HPP

#include "item.hpp"
#include "item_model.hpp"

namespace tokotachi
{

class ListModel : public ItemModel, private detail::ItemHost
{
public:
    explicit ListModel(Object* owner = nullptr);
    ~ListModel();

    /// 最後に加える。
    Item* add_item(const std::string& text);
    /// index の位置に加える(範囲の外なら最後)。
    Item* insert_item(int index, const std::string& text);
    /// 項目を除いて破棄する。
    void remove_item(Item* item);
    /// すべての項目を除いて破棄する。
    void clear();

    /// 項目(並んだ順)。
    ReadOnlyProperty<std::vector<Item*> > items;

    /// 項目の位置(このモデルの項目でなければ無効な ModelIndex)・位置の項目。
    ModelIndex index_of(const Item* item) const;
    Item* item_at(const ModelIndex& index) const;

    int row_count(const ModelIndex& parent = ModelIndex()) const override;
    ModelIndex index(int row, const ModelIndex& parent = ModelIndex()) const override;
    std::string text(const ModelIndex& index, int column) const override;
    Image icon(const ModelIndex& index) const override;
    bool enabled(const ModelIndex& index) const override;
    bool checkable(const ModelIndex& index) const override;
    bool checked(const ModelIndex& index) const override;
    void set_checked(const ModelIndex& index, bool checked) override;
    /// 文字のバイト列の順(UTF-8 なので、コードポイントの順)で、同じ値の項目の順は保つ。
    void sort(int column, SortOrder order) override;
    ModelIndex locate(const ModelIndex& index) const override;

private:
    std::vector<Item*> get_items() const { return items_; }
    void item_changed(Item* item) override;
    void renumber(std::size_t from);

    std::vector<Item*> items_;
};

}  // namespace tokotachi

#endif
