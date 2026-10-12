#include <tokotachi/views/list_model.hpp>

#include <algorithm>

#include "item_impl.hpp"
#include "../core/impl.hpp"

namespace tokotachi
{

Item::Item(detail::ItemHost* host, const std::string& initial_text)
    : Object(nullptr),
      TOKOTACHI_IMPL_PROPERTY_MEMBER(Impl, std::string, text),
      TOKOTACHI_IMPL_PROPERTY_MEMBER(Impl, std::vector<std::string>, column_texts),
      TOKOTACHI_IMPL_PROPERTY_MEMBER(Impl, Image, icon),
      TOKOTACHI_IMPL_PROPERTY_MEMBER(Impl, bool, enabled),
      TOKOTACHI_IMPL_PROPERTY_MEMBER(Impl, bool, checkable),
      TOKOTACHI_IMPL_PROPERTY_MEMBER(Impl, bool, checked),
      TOKOTACHI_IMPL_PROPERTY_MEMBER(Impl, ThemeColor, text_color),
      TOKOTACHI_IMPL_PROPERTY_MEMBER(Impl, Font, font),
      TOKOTACHI_IMPL_PROPERTY_MEMBER(Impl, bool, editable),
      d_(new Impl(this))
{
    d_->host_ = host;
    d_->row_ = 0;
    d_->text_ = initial_text;
    d_->enabled_ = true;
    d_->checkable_ = false;
    d_->checked_ = false;
    d_->editable_ = false;
}

Item::~Item() {
    delete d_;
}

std::string Item::column_text(int column) const
{
    if (column == 0)
    {
        return d_->text_;
    }
    const std::size_t i = static_cast<std::size_t>(column - 1);
    return column > 0 && i < d_->column_texts_.size() ? d_->column_texts_[i] : std::string();
}

void Item::set_column_text(int column, const std::string& value)
{
    if (column <= 0)
    {
        d_->set_text(value);
        return;
    }
    std::vector<std::string> texts = d_->column_texts_;
    if (texts.size() < static_cast<std::size_t>(column))
    {
        texts.resize(static_cast<std::size_t>(column));
    }
    texts[static_cast<std::size_t>(column - 1)] = value;
    d_->set_column_texts(texts);
}

void Item::Impl::changed()
{
    if (host_)
    {
        host_->item_changed(q);
    }
}

void Item::Impl::set_text(const std::string& v)
{
    if (v != text_)
    {
        text_ = v;
        changed();
    }
}

void Item::Impl::set_column_texts(const std::vector<std::string>& v)
{
    if (v != column_texts_)
    {
        column_texts_ = v;
        changed();
    }
}

void Item::Impl::set_icon(const Image& v)
{
    if (v != icon_)
    {
        icon_ = v;
        changed();
    }
}

void Item::Impl::set_enabled(const bool& v)
{
    if (v != enabled_)
    {
        enabled_ = v;
        changed();
    }
}

void Item::Impl::set_checkable(const bool& v)
{
    if (v != checkable_)
    {
        checkable_ = v;
        changed();
    }
}

void Item::Impl::set_checked(const bool& v)
{
    if (v != checked_)
    {
        checked_ = v;
        changed();
    }
}

void Item::Impl::set_text_color(const ThemeColor& v)
{
    if (v != text_color_)
    {
        text_color_ = v;
        changed();
    }
}

void Item::Impl::set_font(const Font& v)
{
    if (v != font_)
    {
        font_ = v;
        changed();
    }
}

void Item::Impl::set_editable(const bool& v)
{
    if (v != editable_)
    {
        editable_ = v;
        changed();
    }
}

class ListModel::Impl : public detail::ItemHost
{
    TOKOTACHI_IMPL(ListModel);

    explicit Impl(ListModel* object) : q(object) {}

    void item_changed(Item* item) override;

    std::vector<Item*> get_items() const { return items_; }
    void renumber(std::size_t from);

    std::vector<Item*> items_;
};

ListModel::ListModel(Object* owner_object)
    : ItemModel(owner_object), TOKOTACHI_IMPL_READONLY_PROPERTY(Impl, std::vector<Item*>, items),
      d_(new Impl(this))
{
}

ListModel::~ListModel()
{
    for (std::size_t i = 0; i < d_->items_.size(); ++i)
    {
        d_->items_[i]->d_->host_ = nullptr;
        delete d_->items_[i];
    }
    d_->items_.clear();
    delete d_;
}

Item* ListModel::add_item(const std::string& text_value)
{
    return insert_item(static_cast<int>(d_->items_.size()), text_value);
}

Item* ListModel::insert_item(int index_value, const std::string& text_value)
{
    const int n = static_cast<int>(d_->items_.size());
    const int at = index_value >= 0 && index_value <= n ? index_value : n;
    Item* item = new Item(d_, text_value);
    d_->items_.insert(d_->items_.begin() + at, item);
    d_->renumber(static_cast<std::size_t>(at));
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
    d_->items_.erase(d_->items_.begin() + i.row());
    d_->renumber(static_cast<std::size_t>(i.row()));
    item->d_->host_ = nullptr;
    delete item;
    notify_changed(items);
}

void ListModel::move_item(Item* item, int index)
{
    const ModelIndex i = index_of(item);
    if (!i.is_valid())
    {
        return;
    }
    std::vector<ModelIndex> one(1, i);
    // 移した後の位置にするため、元より後ろなら 1 つ後ろの前に入れる
    const int n = static_cast<int>(d_->items_.size());
    const int at = index < 0 || index >= n ? n : index >= i.row() ? index + 1 : index;
    move_rows(one, ModelIndex(), at);
}

bool ListModel::can_move_rows(const std::vector<ModelIndex>& indexes, const ModelIndex& parent, int) const
{
    if (parent.is_valid() || indexes.empty())
    {
        return false;
    }
    for (std::size_t k = 0; k < indexes.size(); ++k)
    {
        if (!item_at(indexes[k]))
        {
            return false;
        }
    }
    return true;
}

bool ListModel::move_rows(const std::vector<ModelIndex>& indexes, const ModelIndex& parent, int row)
{
    if (!can_move_rows(indexes, parent, row))
    {
        return false;
    }
    // 移す項目(今の並びの順)を除き、row より前にあったものの数だけ入れる位置をずらす
    const int n = static_cast<int>(d_->items_.size());
    const int at = row < 0 || row > n ? n : row;
    std::vector<bool> moving(d_->items_.size(), false);
    for (std::size_t k = 0; k < indexes.size(); ++k)
    {
        moving[static_cast<std::size_t>(item_at(indexes[k])->d_->row_)] = true;
    }
    std::vector<Item*> moved, rest;
    int before = 0;
    for (int r = 0; r < n; ++r)
    {
        if (moving[static_cast<std::size_t>(r)])
        {
            moved.push_back(d_->items_[static_cast<std::size_t>(r)]);
            if (r < at)
            {
                ++before;
            }
        }
        else
        {
            rest.push_back(d_->items_[static_cast<std::size_t>(r)]);
        }
    }
    const std::vector<Item*>::iterator pos = rest.begin() + (at - before);
    rest.insert(pos, moved.begin(), moved.end());
    if (rest == d_->items_)
    {
        return true;
    }
    d_->items_.swap(rest);
    d_->renumber(0);
    notify_changed(items);
    notify_layout_changed();
    return true;
}

void ListModel::clear()
{
    if (d_->items_.empty())
    {
        return;
    }
    notify_rows_removing(ModelIndex(), 0, static_cast<int>(d_->items_.size()));
    std::vector<Item*> old;
    old.swap(d_->items_);
    for (std::size_t i = 0; i < old.size(); ++i)
    {
        old[i]->d_->host_ = nullptr;
        delete old[i];
    }
    notify_changed(items);
}

void ListModel::Impl::renumber(std::size_t from)
{
    for (std::size_t i = from; i < items_.size(); ++i)
    {
        items_[i]->d_->row_ = static_cast<int>(i);
    }
}

ModelIndex ListModel::index_of(const Item* item) const
{
    if (!item || item->d_->host_ != static_cast<const detail::ItemHost*>(d_))
    {
        return ModelIndex();
    }
    return make_index(item->d_->row_, const_cast<Item*>(item));
}

Item* ListModel::item_at(const ModelIndex& i) const
{
    // 行と識別子が合っているときだけ(古い ModelIndex で別の項目を指さないように)。
    if (!i.is_valid() || i.row() >= static_cast<int>(d_->items_.size()) || d_->items_[i.row()] != i.id())
    {
        return nullptr;
    }
    return d_->items_[i.row()];
}

int ListModel::row_count(const ModelIndex& parent) const
{
    return parent.is_valid() ? 0 : static_cast<int>(d_->items_.size());
}

ModelIndex ListModel::index(int row, const ModelIndex& parent) const
{
    if (parent.is_valid() || row < 0 || row >= static_cast<int>(d_->items_.size()))
    {
        return ModelIndex();
    }
    return make_index(row, d_->items_[row]);
}

std::string ListModel::text(const ModelIndex& i, int column) const
{
    const Item* item = item_at(i);
    return item ? item->column_text(column) : std::string();
}

ThemeColor ListModel::text_color(const ModelIndex& i, int) const
{
    const Item* item = item_at(i);
    return item ? item->d_->text_color_ : ThemeColor();
}

Font ListModel::font(const ModelIndex& i, int) const
{
    const Item* item = item_at(i);
    return item ? item->d_->font_ : Font();
}

bool ListModel::editable(const ModelIndex& i, int) const
{
    const Item* item = item_at(i);
    return item && item->d_->editable_;
}

void ListModel::set_text(const ModelIndex& i, int column, const std::string& value)
{
    Item* item = item_at(i);
    if (item)
    {
        item->set_column_text(column, value);
    }
}

Image ListModel::icon(const ModelIndex& i) const
{
    const Item* item = item_at(i);
    return item ? item->d_->icon_ : Image();
}

bool ListModel::enabled(const ModelIndex& i) const
{
    const Item* item = item_at(i);
    return item && item->d_->enabled_;
}

bool ListModel::checkable(const ModelIndex& i) const
{
    const Item* item = item_at(i);
    return item && item->d_->checkable_;
}

bool ListModel::checked(const ModelIndex& i) const
{
    const Item* item = item_at(i);
    return item && item->d_->checked_;
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
    std::stable_sort(d_->items_.begin(), d_->items_.end(), less);
    d_->renumber(0);
    notify_changed(items);
    notify_layout_changed();
}

ModelIndex ListModel::locate(const ModelIndex& i) const
{
    // ビューが持っている ModelIndex の項目は残っている(除くときに知らせを受けて捨てる)ので、識別子から引ける。
    const Item* item = static_cast<const Item*>(i.id());
    return item ? index_of(item) : ModelIndex();
}

void ListModel::Impl::item_changed(Item* item) { q->notify_data_changed(q->make_index(item->d_->row_, item)); }

}  // namespace tokotachi
