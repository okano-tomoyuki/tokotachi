/// @file
/// tokotachi::Item の宣言。
#ifndef TOKOTACHI_VIEWS_ITEM_HPP
#define TOKOTACHI_VIEWS_ITEM_HPP

#include <string>
#include <vector>

#include "../core/export.hpp"
#include "../graphics/image.hpp"
#include "../graphics/text_layout.hpp"
#include "../graphics/theme.hpp"
#include "../core/property.hpp"

namespace tokotachi
{

class Item;
class ListModel;
class TreeModel;

namespace detail
{
/// 項目を持つモデルの実装(内部)。
class ItemHost;
}  // namespace detail

/// ListModel・TreeModel の 1 つの項目(docs/adr/0026)。モデルが作り、所有する。
///
/// @code
/// tokotachi::Item* item = list->add_item("報告書.txt");
/// item->column_texts = std::vector<std::string>{"12 KB", "2026-10-04"};  // 2 列目以降
/// item->icon = file_icon;
/// item->checkable = true;
/// @endcode
///
/// 値を変えると、表示しているビューが描き直す。
///
/// @ingroup views
class TOKOTACHI_API Item : public Object
{
public:
    /// 1 列目の文字。
    Property<std::string> text;
    /// 2 列目以降の文字。
    Property<std::vector<std::string> > column_texts;
    /// 1 列目の文字の前に描く画像。
    Property<Image> icon;
    /// false の項目は薄く描き、選べない。
    Property<bool> enabled;
    /// チェックボックスを付けるか・付いているか(利用者が押すと checked が変わる)。
    Property<bool> checkable;
    /// チェックしているか(checkable のとき)。
    Property<bool> checked;
    /// 文字の色とフォント(すべての列。既定はビューの標準。docs/adr/0046)。
    Property<ThemeColor> text_color;
    /// 文字のフォント(すべての列)。
    Property<Font> font;
    /// ビューで文字を編集できるか(すべての列。F2・選んでいる項目のクリックで編集する。docs/adr/0046)。
    Property<bool> editable;

    /// column 列目の文字(列が無ければ空)。
    std::string column_text(int column) const;
    /// column 列目の文字を変える(1 列目なら text、2 列目以降なら column_texts。足りなければ空の文字で埋める)。
    void set_column_text(int column, const std::string& value);

protected:
    /// ライブラリが作る(ListModel::add_item・TreeModel::add_item などを使う)。
    Item(detail::ItemHost* host, const std::string& text);
    ~Item();

private:
    friend class ListModel;
    friend class TreeModel;
    class Impl;
    Impl* d_;
};

}  // namespace tokotachi

#endif
