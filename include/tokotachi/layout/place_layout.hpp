/// @file
/// tokotachi::PlaceLayout の宣言。
#ifndef TOKOTACHI_LAYOUT_PLACE_LAYOUT_HPP
#define TOKOTACHI_LAYOUT_PLACE_LAYOUT_HPP

#include "../core/export.hpp"
#include "layout.hpp"

namespace tokotachi
{

/// 子のどの点を、指定した位置に合わせるか。
enum class Anchor
{
    TOP_LEFT,
    TOP,
    TOP_RIGHT,
    LEFT,
    CENTER,
    RIGHT,
    BOTTOM_LEFT,
    BOTTOM,
    BOTTOM_RIGHT
};

/// PlaceLayout の項目の位置と大きさ(固定の dp と、入れ物に対する割合)。
///
/// @ingroup layout
struct TOKOTACHI_API Placement
{
    float x, y;  ///< dp
    float relative_x, relative_y;  ///< 親の大きさに対する割合
    float width, height;  ///< dp
    float relative_width, relative_height;
    Anchor anchor;

    Placement()
        : x(0), y(0), relative_x(0), relative_y(0), width(0), height(0), relative_width(0), relative_height(0),
          anchor(Anchor::TOP_LEFT)
    {
    }
};

/// 子を親に対する位置・割合で置くレイアウト(cpp_tk の place に当たる。docs/adr/0017)。
///
/// @code
/// tokotachi::Placement p;
/// p.relative_x = 0.5f;            // 親の幅の半分の位置
/// p.relative_y = 0.5f;
/// p.anchor = tokotachi::Anchor::CENTER; // 子の中央をその位置に合わせる
/// place->add(button_center, p);
/// @endcode
///
/// 位置は (x + relative_x × 親の幅, y + relative_y × 親の高さ)、大きさは (width + relative_width × 親の幅, ...)。
/// 大きさの指定が 0(width も relative_width も 0)なら、子の望む大きさを使う。
/// レイアウトを付けない自由配置(bounds を利用者が決める)とは違い、親の大きさが変わると置き直す。
///
/// @ingroup layout
class TOKOTACHI_API PlaceLayout : public Layout
{
public:
    explicit PlaceLayout(Widget* target);

    /// w(target の子)を置く。すでに置いていれば置き方を変える。
    void add(Widget* w, const Placement& placement);

protected:
    Size compute_preferred_size() const override;
    Size compute_minimum_size() const override;
    void arrange(const Rect& area) override;
    void widget_removed(Widget* w) override;

private:
    struct Item
    {
        Widget* widget;
        Placement placement;
    };

    std::vector<Item> items_;
};

}  // namespace tokotachi

#endif
