// ItemModel の実装(内部。item_model.cpp・item_view.cpp が使う。docs/adr/0057)。
#ifndef TOKOTACHI_SRC_VIEWS_ITEM_MODEL_IMPL_HPP
#define TOKOTACHI_SRC_VIEWS_ITEM_MODEL_IMPL_HPP

#include <vector>
#include <tokotachi/views/item_model.hpp>
#include "../core/impl.hpp"

namespace tokotachi
{

class ItemModel::Impl
{
    TOKOTACHI_IMPL(ItemModel);

    explicit Impl(ItemModel* object) : q(object) {}

    std::vector<ItemView*> views_;
};

}  // namespace tokotachi

#endif
