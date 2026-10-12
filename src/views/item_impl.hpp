// Item の実装(内部。list_model.cpp・tree_model.cpp が使う。docs/adr/0057)。
#ifndef TOKOTACHI_SRC_VIEWS_ITEM_IMPL_HPP
#define TOKOTACHI_SRC_VIEWS_ITEM_IMPL_HPP

#include <string>
#include <vector>
#include <tokotachi/views/item.hpp>
#include "../core/impl.hpp"

namespace tokotachi
{

namespace detail
{

/// 項目を持つモデルの実装(ListModel・TreeModel の実装のクラス)。項目の値が変わったことを受け取る。
class ItemHost
{
public:
    virtual ~ItemHost() {}
    virtual void item_changed(Item* item) = 0;
};

}  // namespace detail

class Item::Impl
{
    TOKOTACHI_IMPL(Item);

    explicit Impl(Item* object) : q(object) {}

    void set_text(const std::string& v);
    void set_column_texts(const std::vector<std::string>& v);
    void set_icon(const Image& v);
    void set_enabled(const bool& v);
    void set_checkable(const bool& v);
    void set_checked(const bool& v);
    void set_text_color(const ThemeColor& v);
    void set_font(const Font& v);
    void set_editable(const bool& v);
    void changed();

    detail::ItemHost* host_;
    int row_;  ///< 親の中での位置(モデルが保つ)
    std::string text_;
    std::vector<std::string> column_texts_;
    Image icon_;
    bool enabled_;
    bool checkable_;
    bool checked_;
    ThemeColor text_color_;
    Font font_;
    bool editable_;
};

}  // namespace tokotachi

#endif
