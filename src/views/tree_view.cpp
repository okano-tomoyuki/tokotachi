#include <tokotachi/views/tree_view.hpp>

#include "../core/impl.hpp"
#include "item_view_impl.hpp"

namespace tokotachi
{

class TreeView::Impl
{
    TOKOTACHI_IMPL(TreeView);

    explicit Impl(TreeView* object) : q(object) {}

    float get_indentation() const { return detail::ImplAccess<ItemView>::of(q)->get_indentation(); }
    void set_indentation(const float& v) { detail::ImplAccess<ItemView>::of(q)->set_indentation(v); }
};

TreeView::TreeView(Widget* parent_widget)
    : ItemView(parent_widget, detail::internal_tag(), true), TOKOTACHI_IMPL_PROPERTY_GETTER(Impl, float, indentation),
      d_(new Impl(this))
{
}

TreeView::~TreeView() { delete d_; }

void TreeView::expand(const ModelIndex& index)
{
    detail::ImplAccess<ItemView>::of(this)->expand(index);
}

void TreeView::collapse(const ModelIndex& index)
{
    detail::ImplAccess<ItemView>::of(this)->collapse(index);
}

bool TreeView::is_expanded(const ModelIndex& index) const
{
    return detail::ImplAccess<ItemView>::of(this)->is_expanded(index);
}

void TreeView::expand_all()
{
    detail::ImplAccess<ItemView>::of(this)->expand_all();
}

void TreeView::collapse_all()
{
    detail::ImplAccess<ItemView>::of(this)->collapse_all();
}

}  // namespace tokotachi
