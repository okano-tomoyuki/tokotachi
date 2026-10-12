#include <tokotachi/views/tree_view.hpp>

#include "../core/impl.hpp"

namespace tokotachi
{

class TreeView::Impl
{
    TOKOTACHI_IMPL(TreeView);

    explicit Impl(TreeView* object) : q(object) {}

    float get_indentation() const { return q->ItemView::get_indentation(); }
    void set_indentation(const float& v) { q->ItemView::set_indentation(v); }
};

TreeView::TreeView(Widget* parent_widget)
    : ItemView(parent_widget, true), TOKOTACHI_IMPL_PROPERTY_GETTER(Impl, float, indentation),
      d_(new Impl(this))
{
}

TreeView::~TreeView() { delete d_; }

}  // namespace tokotachi
