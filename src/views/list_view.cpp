#include <tokotachi/views/list_view.hpp>

#include "../core/impl.hpp"

namespace tokotachi
{

ListView::ListView(Widget* parent_widget) : ItemView(parent_widget, detail::internal_tag(), false) {}

}  // namespace tokotachi
