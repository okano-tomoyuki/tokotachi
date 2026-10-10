#include <tokotachi/views/tree_view.hpp>

#include "../core/property_impl.hpp"

namespace tokotachi
{

TreeView::TreeView(Widget* parent_widget)
    : ItemView(parent_widget, true), TOKOTACHI_PROPERTY_GETTER(TreeView, float, indentation)
{
}

}  // namespace tokotachi
