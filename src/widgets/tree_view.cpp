#include <gui/tree_view.hpp>

#include "../core/property_impl.hpp"

namespace gui
{

TreeView::TreeView(Widget* parent_widget)
    : ItemView(parent_widget, true), GUI_PROPERTY_GETTER(TreeView, float, indentation)
{
}

}  // namespace gui
