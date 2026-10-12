/// @file
/// tokotachi::ListView の宣言。
#ifndef TOKOTACHI_VIEWS_LIST_VIEW_HPP
#define TOKOTACHI_VIEWS_LIST_VIEW_HPP

#include "../core/export.hpp"
#include "item_view.hpp"

namespace tokotachi
{

/// 項目の一覧を表示するビュー(docs/adr/0026)。モデルの一番上の行だけを表示する。
///
/// @image html widget_list_view.png
///
/// @code
/// tokotachi::ListModel* fruits = new tokotachi::ListModel(window_main);
/// fruits->add_item("りんご");
/// fruits->add_item("みかん");
/// tokotachi::ListView* list_view_fruits = new tokotachi::ListView(window_main);
/// list_view_fruits->model = fruits;
/// list_view_fruits->on_selection_change = [=](tokotachi::Object*) { ... list_view_fruits->current ... };
/// @endcode
///
/// 選び方・列・見出し・キーの操作は ItemView(item_view.hpp)を参照。
///
/// @ingroup views
class TOKOTACHI_API ListView : public ItemView
{
public:
    /// parent の子として作る(parent が所有する)。
    explicit ListView(Widget* parent = nullptr);
};

}  // namespace tokotachi

#endif
