// Column の実装(内部。item_view.cpp・table_view.cpp が使う。docs/adr/0057)。
#ifndef TOKOTACHI_SRC_VIEWS_COLUMN_IMPL_HPP
#define TOKOTACHI_SRC_VIEWS_COLUMN_IMPL_HPP

#include <string>
#include <tokotachi/graphics/text_layout.hpp>
#include <tokotachi/views/item_view.hpp>
#include "../core/impl.hpp"

namespace tokotachi
{

class Column::Impl
{
    TOKOTACHI_IMPL(Column);

    explicit Impl(Column* object) : q(object) {}

    void set_title(const std::string& v);
    void set_width(const float& v);
    void set_alignment(const Align& v);

    ItemView* view_;
    std::string title_;
    float width_;
    Align alignment_;
    mutable TextLayout layout_;
};

}  // namespace tokotachi

#endif
