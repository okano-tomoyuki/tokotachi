#include <tokotachi/views/item_model.hpp>

#include <algorithm>

#include <tokotachi/views/item_view.hpp>

namespace tokotachi
{

ItemModel::ItemModel(Object* owner_object) : Object(owner_object) {}

ItemModel::~ItemModel()
{
    // ビューは model を nullptr にする(views_ から自分を除く)。
    while (!views_.empty())
    {
        views_.back()->model_destroyed();
    }
}

ModelIndex ItemModel::index(int row, const ModelIndex& parent) const
{
    if (parent.is_valid() || row < 0 || row >= row_count())
    {
        return ModelIndex();
    }
    return make_index(row, nullptr);
}

ModelIndex ItemModel::parent_of(const ModelIndex&) const { return ModelIndex(); }

Image ItemModel::icon(const ModelIndex&) const { return Image(); }

bool ItemModel::enabled(const ModelIndex&) const { return true; }

bool ItemModel::checkable(const ModelIndex&) const { return false; }

bool ItemModel::checked(const ModelIndex&) const { return false; }

void ItemModel::set_checked(const ModelIndex&, bool) {}

void ItemModel::sort(int, SortOrder) {}

ModelIndex ItemModel::locate(const ModelIndex&) const { return ModelIndex(); }

namespace
{

// 知らせの間にビューが外れても(ハンドラがビューを破棄しても)よいように、写しを回し、まだ居るかを確かめる。
template <typename F>
void for_each_view(const std::vector<ItemView*>& views, F call)
{
    const std::vector<ItemView*> copy = views;
    for (std::size_t i = 0; i < copy.size(); ++i)
    {
        if (std::find(views.begin(), views.end(), copy[i]) != views.end())
        {
            call(copy[i]);
        }
    }
}

}  // namespace

void ItemModel::notify_rows_inserted(const ModelIndex& parent, int first, int count)
{
    for_each_view(views_, [&](ItemView* v) { v->model_rows_inserted(parent, first, count); });
}

void ItemModel::notify_rows_removing(const ModelIndex& parent, int first, int count)
{
    for_each_view(views_, [&](ItemView* v) { v->model_rows_removing(parent, first, count); });
}

void ItemModel::notify_data_changed(const ModelIndex& index)
{
    for_each_view(views_, [&](ItemView* v) { v->model_data_changed(index); });
}

void ItemModel::notify_layout_changed()
{
    for_each_view(views_, [](ItemView* v) { v->model_layout_changed(); });
}

void ItemModel::notify_reset()
{
    for_each_view(views_, [](ItemView* v) { v->model_reset(); });
}

}  // namespace tokotachi
