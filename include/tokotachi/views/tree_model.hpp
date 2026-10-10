/// @file
/// 項目をオブジェクト(TreeItem)で持つ木のモデル(docs/adr/0026)。
///
/// @code
/// tokotachi::TreeModel* tree = new tokotachi::TreeModel(&window);
/// tokotachi::TreeItem* docs = tree->add_item("文書");
/// docs->add_child("報告書.txt");
/// tree_view_docs->model = tree;
/// @endcode
#ifndef TOKOTACHI_VIEWS_TREE_MODEL_HPP
#define TOKOTACHI_VIEWS_TREE_MODEL_HPP

#include "item.hpp"
#include "item_model.hpp"

namespace tokotachi
{

class TreeModel;

class TreeItem : public Item
{
public:
    /// 親の項目(一番上なら nullptr)・子の項目(並んだ順)。
    ReadOnlyProperty<TreeItem*> parent_item;
    ReadOnlyProperty<std::vector<TreeItem*> > children;

    /// 最後の子として加える。
    TreeItem* add_child(const std::string& text);
    /// index の位置に子として加える(範囲の外なら最後)。
    TreeItem* insert_child(int index, const std::string& text);

private:
    friend class TreeModel;

    TreeItem(TreeModel* model, TreeItem* parent, const std::string& text);
    ~TreeItem();

    TreeItem* get_parent_item() const { return parent_; }
    std::vector<TreeItem*> get_children() const { return children_; }

    TreeModel* model_;
    TreeItem* parent_;
    std::vector<TreeItem*> children_;
};

class TreeModel : public ItemModel, private detail::ItemHost
{
public:
    explicit TreeModel(Object* owner = nullptr);
    ~TreeModel();

    /// 一番上に加える。
    TreeItem* add_item(const std::string& text);
    TreeItem* insert_item(int index, const std::string& text);
    /// 項目を(子孫ごと)除いて破棄する。どの深さの項目でもよい。
    void remove_item(TreeItem* item);
    void clear();

    /// 一番上の項目(並んだ順)。
    ReadOnlyProperty<std::vector<TreeItem*> > items;

    ModelIndex index_of(const TreeItem* item) const;
    TreeItem* item_at(const ModelIndex& index) const;

    int row_count(const ModelIndex& parent = ModelIndex()) const override;
    ModelIndex index(int row, const ModelIndex& parent = ModelIndex()) const override;
    ModelIndex parent_of(const ModelIndex& index) const override;
    std::string text(const ModelIndex& index, int column) const override;
    Image icon(const ModelIndex& index) const override;
    bool enabled(const ModelIndex& index) const override;
    bool checkable(const ModelIndex& index) const override;
    bool checked(const ModelIndex& index) const override;
    void set_checked(const ModelIndex& index, bool checked) override;
    /// 同じ親の子どうしを並べ替える(すべての深さで)。
    void sort(int column, SortOrder order) override;
    ModelIndex locate(const ModelIndex& index) const override;

private:
    friend class TreeItem;

    std::vector<TreeItem*> get_items() const { return roots_; }
    void item_changed(Item* item) override;
    TreeItem* insert(TreeItem* parent, int index, const std::string& text);
    std::vector<TreeItem*>& siblings(TreeItem* parent) { return parent ? parent->children_ : roots_; }
    static void renumber(std::vector<TreeItem*>& list, std::size_t from);
    static void destroy(TreeItem* item);
    void sort_list(std::vector<TreeItem*>& list, int column, SortOrder order);

    std::vector<TreeItem*> roots_;
};

}  // namespace tokotachi

#endif
