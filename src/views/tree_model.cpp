#include <tokotachi/views/tree_model.hpp>

#include <algorithm>

#include "item_impl.hpp"
#include "../core/impl.hpp"

namespace tokotachi
{

class TreeItem::Impl
{
    TOKOTACHI_IMPL(TreeItem);

    explicit Impl(TreeItem* object) : q(object) {}

    TreeItem* get_parent_item() const { return parent_; }
    std::vector<TreeItem*> get_children() const { return children_; }

    TreeModel* model_;
    TreeItem* parent_;
    std::vector<TreeItem*> children_;
};

class TreeModel::Impl : public detail::ItemHost
{
    TOKOTACHI_IMPL(TreeModel);

    explicit Impl(TreeModel* object) : q(object) {}

    void item_changed(Item* item) override;

    std::vector<TreeItem*> get_items() const { return roots_; }
    TreeItem* insert(TreeItem* parent, int index, const std::string& text);
    std::vector<TreeItem*>& siblings(TreeItem* parent) { return parent ? parent->d_->children_ : roots_; }
    static void renumber(std::vector<TreeItem*>& list, std::size_t from);
    static void destroy(TreeItem* item);
    void sort_list(std::vector<TreeItem*>& list, int column, SortOrder order);

    std::vector<TreeItem*> roots_;
};

TreeItem::TreeItem(TreeModel* model, TreeItem* parent, const std::string& initial_text)
    : Item(model->d_, initial_text),
      TOKOTACHI_IMPL_READONLY_PROPERTY(Impl, TreeItem*, parent_item),
      TOKOTACHI_IMPL_READONLY_PROPERTY(Impl, std::vector<TreeItem*>, children),
      d_(new Impl(this))
{
    d_->model_ = model;
    d_->parent_ = parent;
}

TreeItem::~TreeItem() {
    delete d_;
}

TreeItem* TreeItem::add_child(const std::string& text_value)
{
    return insert_child(static_cast<int>(d_->children_.size()), text_value);
}

TreeItem* TreeItem::insert_child(int index, const std::string& text_value)
{
    return d_->model_ ? d_->model_->d_->insert(this, index, text_value) : nullptr;
}

TreeModel::TreeModel(Object* owner_object)
    : ItemModel(owner_object), TOKOTACHI_IMPL_READONLY_PROPERTY(Impl, std::vector<TreeItem*>, items),
      d_(new Impl(this))
{
}

TreeModel::~TreeModel()
{
    for (std::size_t i = 0; i < d_->roots_.size(); ++i)
    {
        d_->destroy(d_->roots_[i]);
    }
    d_->roots_.clear();
    delete d_;
}

void TreeModel::Impl::destroy(TreeItem* item)
{
    for (std::size_t i = 0; i < item->d_->children_.size(); ++i)
    {
        destroy(item->d_->children_[i]);
    }
    item->Item::d_->host_ = nullptr;
    item->d_->model_ = nullptr;
    delete item;
}

TreeItem* TreeModel::add_item(const std::string& text_value)
{
    return d_->insert(nullptr, static_cast<int>(d_->roots_.size()), text_value);
}

TreeItem* TreeModel::insert_item(int index_value, const std::string& text_value)
{
    return d_->insert(nullptr, index_value, text_value);
}

TreeItem* TreeModel::Impl::insert(TreeItem* parent, int index_value, const std::string& text_value)
{
    std::vector<TreeItem*>& list = siblings(parent);
    const int n = static_cast<int>(list.size());
    const int at = index_value >= 0 && index_value <= n ? index_value : n;
    TreeItem* item = new TreeItem(q, parent, text_value);
    list.insert(list.begin() + at, item);
    renumber(list, static_cast<std::size_t>(at));
    if (parent)
    {
        q->notify_changed(parent->children);
    }
    else
    {
        q->notify_changed(q->items);
    }
    q->notify_rows_inserted(q->index_of(parent), at, 1);
    return item;
}

void TreeModel::remove_item(TreeItem* item)
{
    if (!item || item->d_->model_ != this)
    {
        return;
    }
    TreeItem* parent = item->d_->parent_;
    std::vector<TreeItem*>& list = d_->siblings(parent);
    const int row = item->Item::d_->row_;
    notify_rows_removing(index_of(parent), row, 1);
    list.erase(list.begin() + row);
    d_->renumber(list, static_cast<std::size_t>(row));
    d_->destroy(item);
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
    if (d_->roots_.empty())
    {
        return;
    }
    notify_rows_removing(ModelIndex(), 0, static_cast<int>(d_->roots_.size()));
    std::vector<TreeItem*> old;
    old.swap(d_->roots_);
    for (std::size_t i = 0; i < old.size(); ++i)
    {
        d_->destroy(old[i]);
    }
    notify_changed(items);
}

void TreeModel::move_item(TreeItem* item, TreeItem* parent, int index)
{
    const ModelIndex i = index_of(item);
    if (!i.is_valid())
    {
        return;
    }
    std::vector<ModelIndex> one(1, i);
    // 移した後の位置にするため、同じ親で元より後ろなら 1 つ後ろの前に入れる
    const int n = static_cast<int>(d_->siblings(parent).size());
    const int at = index < 0 || index >= n ? -1 : parent == item->d_->parent_ && index >= i.row() ? index + 1 : index;
    move_rows(one, index_of(parent), at);
}

bool TreeModel::can_move_rows(const std::vector<ModelIndex>& indexes, const ModelIndex& parent, int) const
{
    if (indexes.empty() || (parent.is_valid() && !item_at(parent)))
    {
        return false;
    }
    const TreeItem* target = item_at(parent);
    for (std::size_t k = 0; k < indexes.size(); ++k)
    {
        const TreeItem* item = item_at(indexes[k]);
        if (!item)
        {
            return false;
        }
        // 自分・子孫の下へは移せない
        for (const TreeItem* p = target; p; p = p->d_->parent_)
        {
            if (p == item)
            {
                return false;
            }
        }
    }
    return true;
}

bool TreeModel::move_rows(const std::vector<ModelIndex>& indexes, const ModelIndex& parent, int row)
{
    if (!can_move_rows(indexes, parent, row))
    {
        return false;
    }
    TreeItem* target = item_at(parent);
    std::vector<TreeItem*>& dest = d_->siblings(target);
    // 入れる位置の前にある項目(移さないもの)を覚え、その後ろに入れる(移す項目を除くと位置がずれるため)
    const int n = static_cast<int>(dest.size());
    const int at = row < 0 || row > n ? n : row;
    // 移す項目のうち、ほかの移す項目の子孫は、親ごと移るので除く
    std::vector<TreeItem*> moving;
    for (std::size_t k = 0; k < indexes.size(); ++k)
    {
        TreeItem* item = item_at(indexes[k]);
        bool inside = false;
        for (std::size_t m = 0; m < indexes.size() && !inside; ++m)
        {
            const TreeItem* other = item_at(indexes[m]);
            for (const TreeItem* p = item->d_->parent_; p; p = p->d_->parent_)
            {
                inside = inside || p == other;
            }
        }
        if (!inside && std::find(moving.begin(), moving.end(), item) == moving.end())
        {
            moving.push_back(item);
        }
    }
    TreeItem* anchor = nullptr;  // この項目の後ろに入れる(nullptr なら先頭)
    for (int r = at - 1; r >= 0; --r)
    {
        if (std::find(moving.begin(), moving.end(), dest[static_cast<std::size_t>(r)]) == moving.end())
        {
            anchor = dest[static_cast<std::size_t>(r)];
            break;
        }
    }
    for (std::size_t k = 0; k < moving.size(); ++k)
    {
        TreeItem* item = moving[k];
        std::vector<TreeItem*>& from = d_->siblings(item->d_->parent_);
        from.erase(from.begin() + item->Item::d_->row_);
        d_->renumber(from, static_cast<std::size_t>(item->Item::d_->row_));
        if (item->d_->parent_)
        {
            notify_changed(item->d_->parent_->children);
        }
        item->d_->parent_ = target;
    }
    std::size_t pos = 0;
    if (anchor)
    {
        pos = static_cast<std::size_t>(std::find(dest.begin(), dest.end(), anchor) - dest.begin()) + 1;
    }
    dest.insert(dest.begin() + static_cast<std::ptrdiff_t>(pos), moving.begin(), moving.end());
    d_->renumber(dest, 0);
    if (target)
    {
        notify_changed(target->children);
    }
    notify_changed(items);
    notify_layout_changed();
    return true;
}

void TreeModel::Impl::renumber(std::vector<TreeItem*>& list, std::size_t from)
{
    for (std::size_t i = from; i < list.size(); ++i)
    {
        list[i]->Item::d_->row_ = static_cast<int>(i);
    }
}

ModelIndex TreeModel::index_of(const TreeItem* item) const
{
    if (!item || item->d_->model_ != this)
    {
        return ModelIndex();
    }
    return make_index(item->Item::d_->row_, const_cast<TreeItem*>(item));
}

TreeItem* TreeModel::item_at(const ModelIndex& i) const
{
    // ビューが持っている ModelIndex の項目は残っている(除くときに知らせを受けて捨てる)ので、識別子から引ける。
    TreeItem* item = static_cast<TreeItem*>(i.id());
    return i.is_valid() && item && item->d_->model_ == this ? item : nullptr;
}

int TreeModel::row_count(const ModelIndex& parent) const
{
    if (!parent.is_valid())
    {
        return static_cast<int>(d_->roots_.size());
    }
    const TreeItem* item = item_at(parent);
    return item ? static_cast<int>(item->d_->children_.size()) : 0;
}

ModelIndex TreeModel::index(int row, const ModelIndex& parent) const
{
    const std::vector<TreeItem*>* list = &d_->roots_;
    if (parent.is_valid())
    {
        const TreeItem* p = item_at(parent);
        if (!p)
        {
            return ModelIndex();
        }
        list = &p->d_->children_;
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
    return item ? index_of(item->d_->parent_) : ModelIndex();
}

std::string TreeModel::text(const ModelIndex& i, int column) const
{
    const TreeItem* item = item_at(i);
    return item ? item->column_text(column) : std::string();
}

ThemeColor TreeModel::text_color(const ModelIndex& i, int) const
{
    const TreeItem* item = item_at(i);
    return item ? item->Item::d_->text_color_ : ThemeColor();
}

Font TreeModel::font(const ModelIndex& i, int) const
{
    const TreeItem* item = item_at(i);
    return item ? item->Item::d_->font_ : Font();
}

bool TreeModel::editable(const ModelIndex& i, int) const
{
    const TreeItem* item = item_at(i);
    return item && item->Item::d_->editable_;
}

void TreeModel::set_text(const ModelIndex& i, int column, const std::string& value)
{
    TreeItem* item = item_at(i);
    if (item)
    {
        item->set_column_text(column, value);
    }
}

Image TreeModel::icon(const ModelIndex& i) const
{
    const TreeItem* item = item_at(i);
    return item ? item->Item::d_->icon_ : Image();
}

bool TreeModel::enabled(const ModelIndex& i) const
{
    const TreeItem* item = item_at(i);
    return item && item->Item::d_->enabled_;
}

bool TreeModel::checkable(const ModelIndex& i) const
{
    const TreeItem* item = item_at(i);
    return item && item->Item::d_->checkable_;
}

bool TreeModel::checked(const ModelIndex& i) const
{
    const TreeItem* item = item_at(i);
    return item && item->Item::d_->checked_;
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

void TreeModel::Impl::sort_list(std::vector<TreeItem*>& list, int column, SortOrder order)
{
    const TreeTextLess less = {column, order == SortOrder::DESCENDING};
    std::stable_sort(list.begin(), list.end(), less);
    renumber(list, 0);
    for (std::size_t i = 0; i < list.size(); ++i)
    {
        if (!list[i]->d_->children_.empty())
        {
            sort_list(list[i]->d_->children_, column, order);
            q->notify_changed(list[i]->children);
        }
    }
}

void TreeModel::sort(int column, SortOrder order)
{
    d_->sort_list(d_->roots_, column, order);
    notify_changed(items);
    notify_layout_changed();
}

ModelIndex TreeModel::locate(const ModelIndex& i) const { return index_of(item_at(i)); }

void TreeModel::Impl::item_changed(Item* item)
{
    q->notify_data_changed(q->index_of(static_cast<TreeItem*>(item)));
}

}  // namespace tokotachi
