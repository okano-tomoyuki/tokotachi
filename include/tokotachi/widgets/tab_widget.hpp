/// @file
/// tokotachi::TabWidget の宣言。
#ifndef TOKOTACHI_WIDGETS_TAB_WIDGET_HPP
#define TOKOTACHI_WIDGETS_TAB_WIDGET_HPP

#include <string>
#include <vector>

#include "../core/export.hpp"
#include "../graphics/text_layout.hpp"
#include "widget.hpp"

namespace tokotachi
{

class TabWidget;

/// 1 つのタブ。TabWidget が作り、所有する。
/// @ingroup widgets_container
class TOKOTACHI_API Tab : public Object
{
public:
    Property<std::string> title;
    /// false のタブは、押しても・キーでも選べない(current_index への代入では選べる)。
    Property<bool> enabled;
    /// このタブで見せるページ。
    ReadOnlyProperty<Widget*> page;

private:
    friend class TabWidget;

    Tab(TabWidget* tab_widget, Widget* page, const std::string& title);
    ~Tab();

    void set_title(const std::string& v);
    void set_enabled(const bool& v);
    Widget* get_page() const { return page_; }

    TabWidget* tab_widget_;
    Widget* page_;
    std::string title_;
    bool enabled_;
    mutable TextLayout layout_;
};

/// タブ: 上に並べた見出しで、中身(ページ)を切り替える(docs/adr/0025)。
///
/// @image html widget_tab_widget.png
///
/// @code
/// tokotachi::TabWidget* tab_widget_settings = new tokotachi::TabWidget(&window);
/// tokotachi::Tab* tab_general = tab_widget_settings->add_tab(page_general, "一般");
/// tokotachi::Tab* tab_detail = tab_widget_settings->add_tab(page_detail, "詳細");
/// tab_detail->enabled = false;
/// tab_widget_settings->on_change = [=](tokotachi::Object*) { ... tab_widget_settings->current_index ... };
/// @endcode
///
/// ページは TabWidget の子になり、選んでいるページだけを見せる(選んでいないページの visible は変えず、showing が false になる)。
/// ページを破棄する・別の親へ移すと、そのタブも消える。
/// 見出しが幅に収まらないときは、右端の ◀▶ とホイールで横にスクロールする。
/// 見出しを押すと TabWidget にフォーカスが移り、←→ で選べる。中のどこにフォーカスがあっても、Ctrl+Tab・Ctrl+Shift+Tab・
/// Ctrl+Page Up/Down で選べる。
///
/// @ingroup widgets_container
class TOKOTACHI_API TabWidget : public Widget
{
public:
    explicit TabWidget(Widget* parent = nullptr);
    ~TabWidget();

    /// 最後に加える。page を TabWidget の子にする。最初のタブを加えると、それを選ぶ。
    Tab* add_tab(Widget* page, const std::string& title);
    /// index の位置に加える(範囲の外なら最後)。
    Tab* insert_tab(int index, Widget* page, const std::string& title);
    /// タブを除き、ページを返す(ページは親が無くなり、呼んだ側が持つ)。選んでいたタブなら隣を選ぶ。
    Widget* remove_tab(Tab* tab);

    /// タブ(並んだ順)。
    ReadOnlyProperty<std::vector<Tab*> > tabs;
    /// 選んでいるタブの番号(タブが無ければ -1)。範囲の外の値は無視する。
    Property<int> current_index;
    /// 選んでいるタブ(無ければ nullptr)。
    ReadOnlyProperty<Tab*> current_tab;

    /// current_index が変わった(利用者の操作・代入・タブの追加と削除のどれでも)。
    Property<NotifyHandler> on_change;

protected:
    void paint(Canvas& canvas) override;
    void mouse_down(MouseEvent& e) override;
    void mouse_move(MouseEvent& e) override;
    void mouse_leave() override;
    void mouse_wheel(WheelEvent& e) override;
    void key_down(KeyEvent& e) override;
    void focus_in() override;
    void focus_out() override;
    void scale_changed() override;
    void arrange_children() override;
    void child_removed(Widget* child) override;
    Size compute_preferred_size() const override;
    Size compute_minimum_size() const override;

private:
    friend class Tab;

    std::vector<Tab*> get_tabs() const { return tabs_; }
    int get_current_index() const { return current_; }
    void set_current_index(const int& v);
    Tab* get_current_tab() const { return current_ >= 0 ? tabs_[current_] : nullptr; }

    /// 番号 i のタブを除く(ページの親は変えない)。
    void erase_tab(int i);
    /// 見出しが変わった・タブの有効が変わった。
    void tab_changed();
    /// 選ぶタブを変え、ページの表示を切り替え、知らせる。
    void select(int i);
    /// 有効なタブを from から step の向きに探す(見つからなければ -1)。
    int next_enabled(int from, int step) const;

    /// 見出しの並びの高さ・各見出しの幅(dp)。
    float bar_height() const;
    float tab_width(const Tab* tab) const;
    const TextLayout& title_layout(const Tab* tab) const;
    float total_width() const;
    /// 見出しを並べられる幅(はみ出すときは ◀▶ の分を除く)。
    float visible_width() const;
    bool overflowing() const;
    void scroll_to(float x);
    void ensure_visible(int i);
    /// 位置 p にある見出しの番号(無ければ -1)。◀▶ の上なら -2・-3。
    int hit(const Point& p) const;

    std::vector<Tab*> tabs_;
    int current_;
    NotifyHandler on_change_;
    float scroll_;  ///< 見出しの並びの横のスクロール(dp)
    int hover_;     ///< ポインタが乗っている見出し(-1 なら無し)
    bool destroying_;
};

}  // namespace tokotachi

#endif
