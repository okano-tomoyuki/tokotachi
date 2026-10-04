// 項目の一覧を表示するビュー(docs/adr/0026)。モデルの一番上の行だけを表示する。
//
//   gui::ListModel* fruits = new gui::ListModel(&window);
//   fruits->add_item("りんご");
//   fruits->add_item("みかん");
//   gui::ListView* view = new gui::ListView(&window);
//   view->model = fruits;
//   view->on_selection_change = [=](gui::Object*) { ... view->current ... };
//
// 選び方・列・見出し・キーの操作は ItemView(item_view.hpp)を参照。
#ifndef GUI_LIST_VIEW_HPP
#define GUI_LIST_VIEW_HPP

#include "item_view.hpp"

namespace gui
{

class ListView : public ItemView
{
public:
    explicit ListView(Widget* parent = nullptr) : ItemView(parent, false) {}
};

}  // namespace gui

#endif
