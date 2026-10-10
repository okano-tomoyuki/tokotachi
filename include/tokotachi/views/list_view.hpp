/// @file
/// 項目の一覧を表示するビュー(docs/adr/0026)。モデルの一番上の行だけを表示する。
///
/// @code
/// tokotachi::ListModel* fruits = new tokotachi::ListModel(&window);
/// fruits->add_item("りんご");
/// fruits->add_item("みかん");
/// tokotachi::ListView* list_view_fruits = new tokotachi::ListView(&window);
/// list_view_fruits->model = fruits;
/// list_view_fruits->on_selection_change = [=](tokotachi::Object*) { ... list_view_fruits->current ... };
/// @endcode
///
/// 選び方・列・見出し・キーの操作は ItemView(item_view.hpp)を参照。
#ifndef TOKOTACHI_VIEWS_LIST_VIEW_HPP
#define TOKOTACHI_VIEWS_LIST_VIEW_HPP

#include "item_view.hpp"

namespace tokotachi
{

class ListView : public ItemView
{
public:
    explicit ListView(Widget* parent = nullptr) : ItemView(parent, false) {}
};

}  // namespace tokotachi

#endif
