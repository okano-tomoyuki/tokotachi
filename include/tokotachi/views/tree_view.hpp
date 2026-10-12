/// @file
/// tokotachi::TreeView の宣言。
#ifndef TOKOTACHI_VIEWS_TREE_VIEW_HPP
#define TOKOTACHI_VIEWS_TREE_VIEW_HPP

#include "../core/export.hpp"
#include "item_view.hpp"

namespace tokotachi
{

/// 項目の木を表示するビュー(docs/adr/0026)。子のある項目は、左の印・ダブルクリック・→← で開閉する。
///
/// @image html widget_tree_view.png
///
/// @code
/// tokotachi::TreeModel* tree = new tokotachi::TreeModel(&window);
/// tokotachi::TreeItem* docs = tree->add_item("文書");
/// docs->add_child("報告書.txt");
/// tokotachi::TreeView* tree_view_docs = new tokotachi::TreeView(&window);
/// tree_view_docs->model = tree;
/// tree_view_docs->expand(tree->index_of(docs));
/// @endcode
///
/// → は閉じた項目を開き、開いた項目では最初の子へ動く。← は開いた項目を閉じ、閉じた項目では親へ動く。
/// 選び方・列・見出し・キーの操作は ItemView(item_view.hpp)を参照。
///
/// @ingroup views
class TOKOTACHI_API TreeView : public ItemView
{
public:
    explicit TreeView(Widget* parent = nullptr);
    ~TreeView() override;

    /// 1 段の字下げ(dp。既定 20。8 以上)。
    Property<float> indentation;

    /// 子を見せる・隠す。閉じた中に current があれば、閉じた項目へ移す。
    void expand(const ModelIndex& index);
    void collapse(const ModelIndex& index);
    bool is_expanded(const ModelIndex& index) const;
    void expand_all();
    void collapse_all();

private:
    class Impl;
    Impl* d_;
};

}  // namespace tokotachi

#endif
