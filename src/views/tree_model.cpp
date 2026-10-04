#include <gui/views/tree_model.hpp>

#include <algorithm>

#include "../core/property_impl.hpp"

namespace gui
{

TreeItem::TreeItem(TreeModel* model, TreeItem* parent, const std::string& initial_text)
    : Item(model, initial_text),
      GUI_READONLY_PROPERTY(TreeItem, TreeItem*, parent_item),
      GUI_READONLY_PROPERTY(TreeItem, std::vector<TreeItem*>, children),
      model_(model),
      parent_(parent)
{
}

TreeItem::~TreeItem() {}

TreeItem* TreeItem::add_child(const std::string& text_value)
{
    return insert_child(static_cast<int>(children_.size()), text_value);
}

TreeItem* TreeItem::insert_child(int index, const std::string& text_value)
{
    return model_ ? model_->insert(this, index, text_value) : nullptr;
}

TreeModel::TreeModel(Object* owner_object)
    : ItemModel(owner_object), GUI_READONLY_PROPERTY(TreeModel, std::vector<TreeItem*>, items)
{
}

TreeModel::~TreeModel()
{
    for (std::size_t i = 0; i < roots_.size(); ++i)
    {
        destroy(roots_[i]);
    }
    roots_.clear();
}

void TreeModel::destroy(TreeItem* item)
{
    for (std::size_t i = 0; i < item->children_.size(); ++i)
    {
        destroy(item->children_[i]);
    }
    item->host_ = nullptr;
    item->model_ = nullptr;
    delete item;
}

TreeItem* TreeModel::add_item(const std::string& text_value)
{
    return insert(nullptr, static_cast<int>(roots_.size()), text_value);
}

TreeItem* TreeModel::insert_item(int index_value, const std::string& text_value)
{
    return insert(nullptr, index_value, text_value);
}

TreeItem* TreeModel::insert(TreeItem* parent, int index_value, const std::string& text_value)
{
    std::vector<TreeItem*>& list = siblings(parent);
    const int n = static_cast<int>(list.size());
    const int at = index_value >= 0 && index_value <= n ? index_value : n;
    TreeItem* item = new TreeItem(this, parent, text_value);
    list.insert(list.begin() + at, item);
    renumber(list, static_cast<std::size_t>(at));
    if (parent)
    {
        notify_changed(parent->children);
    }
    else
    {
        notify_changed(items);
    }
    notify_rows_inserted(index_of(parent), at, 1);
    return item;
}

void TreeModel::remove_item(TreeItem* item)
{
    if (!item || item->model_ != this)
    {
        return;
    }
    TreeItem* parent = item->parent_;
    std::vector<TreeItem*>& list = siblings(parent);
    const int row = item->row_;
    notify_rows_removing(index_of(parent), row, 1);
    list.erase(list.begin() + row);
    renumber(list, static_cast<std::size_t>(row));
    destroy(item);
    if (parent)
    {
        notify_changed(parent->children);
    }
    else
    {
        notify_changed(items);
    }
}

void TreeModel::clear()
{
    if (roots_.empty())
    {
        return;
    }
    notify_rows_removing(ModelIndex(), 0, static_cast<int>(roots_.size()));
    std::vector<TreeItem*> old;
    old.swap(roots_);
    for (std::size_t i = 0; i < old.size(); ++i)
    {
        destroy(old[i]);
    }
    notify_changed(items);
}

void TreeModel::renumber(std::vector<TreeItem*>& list, std::size_t from)
{
    for (std::size_t i = from; i < list.size(); ++i)
    {
        list[i]->row_ = static_cast<int>(i);
    }
}

ModelIndex TreeModel::index_of(const TreeItem* item) const
{
    if (!item || item->model_ != this)
    {
        return ModelIndex();
    }
    return make_index(item->row_, const_cast<TreeItem*>(item));
}

TreeItem* TreeModel::item_at(const ModelIndex& i) const
{
    // ビューが持っている ModelIndex の項目は残っている(除くときに知らせを受けて捨てる)ので、識別子から引ける。
    TreeItem* item = static_cast<TreeItem*>(i.id());
    return i.is_valid() && item && item->model_ == this ? item : nullptr;
}

int TreeModel::row_count(const ModelIndex& parent) const
{
    if (!parent.is_valid())
    {
        return static_cast<int>(roots_.size());
    }
    const TreeItem* item = item_at(parent);
    return item ? static_cast<int>(item->children_.size()) : 0;
}

ModelIndex TreeModel::index(int row, const ModelIndex& parent) const
{
    const std::vector<TreeItem*>* list = &roots_;
    if (parent.is_valid())
    {
        const TreeItem* p = item_at(parent);
        if (!p)
        {
            return ModelIndex();
        }
        list = &p->children_;
    }
    if (row < 0 || row >= static_cast<int>(list->size()))
    {
        return ModelIndex();
    }
    return make_index(row, (*list)[row]);
}

ModelIndex TreeModel::parent_of(const ModelIndex& i) const
{
    const TreeItem* item = item_at(i);
    return item ? index_of(item->parent_) : ModelIndex();
}

std::string TreeModel::text(const ModelIndex& i, int column) const
{
    const TreeItem* item = item_at(i);
    return item ? item->column_text(column) : std::string();
}

Image TreeModel::icon(const ModelIndex& i) const
{
    const TreeItem* item = item_at(i);
    return item ? item->icon_ : Image();
}

bool TreeModel::enabled(const ModelIndex& i) const
{
    const TreeItem* item = item_at(i);
    return item && item->enabled_;
}

bool TreeModel::checkable(const ModelIndex& i) const
{
    const TreeItem* item = item_at(i);
    return item && item->checkable_;
}

bool TreeModel::checked(const ModelIndex& i) const
{
    const TreeItem* item = item_at(i);
    return item && item->checked_;
}

void TreeModel::set_checked(const ModelIndex& i, bool value)
{
    if (TreeItem* item = item_at(i))
    {
        item->checked = value;
    }
}

namespace
{

struct TreeTextLess
{
    int column;
    bool descending;

    bool operator()(const TreeItem* a, const TreeItem* b) const
    {
        return descending ? b->column_text(column) < a->column_text(column)
                          : a->column_text(column) < b->column_text(column);
    }
};

}  // namespace

void TreeModel::sort_list(std::vector<TreeItem*>& list, int column, SortOrder order)
{
    const TreeTextLess less = {column, order == SortOrder::DESCENDING};
    std::stable_sort(list.begin(), list.end(), less);
    renumber(list, 0);
    for (std::size_t i = 0; i < list.size(); ++i)
    {
        if (!list[i]->children_.empty())
        {
            sort_list(list[i]->children_, column, order);
            notify_changed(list[i]->children);
        }
    }
}

void TreeModel::sort(int column, SortOrder order)
{
    sort_list(roots_, column, order);
    notify_changed(items);
    notify_layout_changed();
}

ModelIndex TreeModel::locate(const ModelIndex& i) const { return index_of(item_at(i)); }

void TreeModel::item_changed(Item* item)
{
    notify_data_changed(index_of(static_cast<TreeItem*>(item)));
}

}  // namespace gui
