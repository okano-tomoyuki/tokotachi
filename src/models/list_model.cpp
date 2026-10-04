#include <gui/list_model.hpp>

#include <algorithm>

#include "../core/property_impl.hpp"

namespace gui
{

Item::Item(detail::ItemHost* host, const std::string& initial_text)
    : Object(nullptr),
      GUI_PROPERTY_MEMBER(Item, std::string, text),
      GUI_PROPERTY_MEMBER(Item, std::vector<std::string>, column_texts),
      GUI_PROPERTY_MEMBER(Item, Image, icon),
      GUI_PROPERTY_MEMBER(Item, bool, enabled),
      GUI_PROPERTY_MEMBER(Item, bool, checkable),
      GUI_PROPERTY_MEMBER(Item, bool, checked),
      host_(host),
      row_(0),
      text_(initial_text),
      enabled_(true),
      checkable_(false),
      checked_(false)
{
}

Item::~Item() {}

std::string Item::column_text(int column) const
{
    if (column == 0)
    {
        return text_;
    }
    const std::size_t i = static_cast<std::size_t>(column - 1);
    return column > 0 && i < column_texts_.size() ? column_texts_[i] : std::string();
}

void Item::changed()
{
    if (host_)
    {
        host_->item_changed(this);
    }
}

void Item::set_text(const std::string& v)
{
    if (v != text_)
    {
        text_ = v;
        changed();
    }
}

void Item::set_column_texts(const std::vector<std::string>& v)
{
    if (v != column_texts_)
    {
        column_texts_ = v;
        changed();
    }
}

void Item::set_icon(const Image& v)
{
    if (v != icon_)
    {
        icon_ = v;
        changed();
    }
}

void Item::set_enabled(const bool& v)
{
    if (v != enabled_)
    {
        enabled_ = v;
        changed();
    }
}

void Item::set_checkable(const bool& v)
{
    if (v != checkable_)
    {
        checkable_ = v;
        changed();
    }
}

void Item::set_checked(const bool& v)
{
    if (v != checked_)
    {
        checked_ = v;
        changed();
    }
}

ListModel::ListModel(Object* owner_object)
    : ItemModel(owner_object), GUI_READONLY_PROPERTY(ListModel, std::vector<Item*>, items)
{
}

ListModel::~ListModel()
{
    for (std::size_t i = 0; i < items_.size(); ++i)
    {
        items_[i]->host_ = nullptr;
        delete items_[i];
    }
    items_.clear();
}

Item* ListModel::add_item(const std::string& text_value)
{
    return insert_item(static_cast<int>(items_.size()), text_value);
}

Item* ListModel::insert_item(int index_value, const std::string& text_value)
{
    const int n = static_cast<int>(items_.size());
    const int at = index_value >= 0 && index_value <= n ? index_value : n;
    Item* item = new Item(this, text_value);
    items_.insert(items_.begin() + at, item);
    renumber(static_cast<std::size_t>(at));
    notify_changed(items);
    notify_rows_inserted(ModelIndex(), at, 1);
    return item;
}

void ListModel::remove_item(Item* item)
{
    const ModelIndex i = index_of(item);
    if (!i.is_valid())
    {
        return;
    }
    notify_rows_removing(ModelIndex(), i.row(), 1);
    items_.erase(items_.begin() + i.row());
    renumber(static_cast<std::size_t>(i.row()));
    item->host_ = nullptr;
    delete item;
    notify_changed(items);
}

void ListModel::clear()
{
    if (items_.empty())
    {
        return;
    }
    notify_rows_removing(ModelIndex(), 0, static_cast<int>(items_.size()));
    std::vector<Item*> old;
    old.swap(items_);
    for (std::size_t i = 0; i < old.size(); ++i)
    {
        old[i]->host_ = nullptr;
        delete old[i];
    }
    notify_changed(items);
}

void ListModel::renumber(std::size_t from)
{
    for (std::size_t i = from; i < items_.size(); ++i)
    {
        items_[i]->row_ = static_cast<int>(i);
    }
}

ModelIndex ListModel::index_of(const Item* item) const
{
    if (!item || item->host_ != static_cast<const detail::ItemHost*>(this))
    {
        return ModelIndex();
    }
    return make_index(item->row_, const_cast<Item*>(item));
}

Item* ListModel::item_at(const ModelIndex& i) const
{
    // 行と識別子が合っているときだけ(古い ModelIndex で別の項目を指さないように)。
    if (!i.is_valid() || i.row() >= static_cast<int>(items_.size()) || items_[i.row()] != i.id())
    {
        return nullptr;
    }
    return items_[i.row()];
}

int ListModel::row_count(const ModelIndex& parent) const
{
    return parent.is_valid() ? 0 : static_cast<int>(items_.size());
}

ModelIndex ListModel::index(int row, const ModelIndex& parent) const
{
    if (parent.is_valid() || row < 0 || row >= static_cast<int>(items_.size()))
    {
        return ModelIndex();
    }
    return make_index(row, items_[row]);
}

std::string ListModel::text(const ModelIndex& i, int column) const
{
    const Item* item = item_at(i);
    return item ? item->column_text(column) : std::string();
}

Image ListModel::icon(const ModelIndex& i) const
{
    const Item* item = item_at(i);
    return item ? item->icon_ : Image();
}

bool ListModel::enabled(const ModelIndex& i) const
{
    const Item* item = item_at(i);
    return item && item->enabled_;
}

bool ListModel::checkable(const ModelIndex& i) const
{
    const Item* item = item_at(i);
    return item && item->checkable_;
}

bool ListModel::checked(const ModelIndex& i) const
{
    const Item* item = item_at(i);
    return item && item->checked_;
}

void ListModel::set_checked(const ModelIndex& i, bool value)
{
    if (Item* item = item_at(i))
    {
        item->checked = value;
    }
}

namespace
{

struct TextLess
{
    int column;
    bool descending;

    bool operator()(const Item* a, const Item* b) const
    {
        return descending ? b->column_text(column) < a->column_text(column)
                          : a->column_text(column) < b->column_text(column);
    }
};

}  // namespace

void ListModel::sort(int column, SortOrder order)
{
    const TextLess less = {column, order == SortOrder::DESCENDING};
    std::stable_sort(items_.begin(), items_.end(), less);
    renumber(0);
    notify_changed(items);
    notify_layout_changed();
}

ModelIndex ListModel::locate(const ModelIndex& i) const
{
    // ビューが持っている ModelIndex の項目は残っている(除くときに知らせを受けて捨てる)ので、識別子から引ける。
    const Item* item = static_cast<const Item*>(i.id());
    return item ? index_of(item) : ModelIndex();
}

void ListModel::item_changed(Item* item) { notify_data_changed(make_index(item->row_, item)); }

}  // namespace gui
