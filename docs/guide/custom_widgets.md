# 自作の部品 {#custom_widgets_guide}

[TOC]

自分の部品は、目的に合ったクラスから派生して作ります(docs/adr/0058)。

| 作りたいもの | 派生するクラス | 書くこと |
|---|---|---|
| 既存の部品にプロパティ・ハンドラを足したもの | その部品(`ProgressBar`・`Button`・`ListView` など) | プロパティ・ハンドラ。見た目を足すなら `paint` を上書きし、基底の `paint` を呼んでから描く |
| 押して使う部品(独自の見た目のボタン) | `ButtonBase` | `paint` だけ(押す・ホバー・クリック・キーボード・ニーモニックは `ButtonBase` が持つ) |
| 範囲の中の値を変える部品(つまみなど) | `RangeBase` | `paint` と、マウスで値を変える処理(値・範囲・刻みは `RangeBase` が持つ) |
| 自由に描く部品 | `PaintBox`(ハンドラ `on_paint` でもよい) | `paint` |
| 何でも自分で決める部品・自分で子を並べる入れ物 | `Widget` | `paint`・入力のイベント・`compute_preferred_size`・`arrange_children` など |
| 自分の並べ方 | `Layout` | `compute_preferred_size`・`compute_minimum_size`・`arrange`・`widget_removed` |
| 自分のデータの一覧・木 | `ItemModel` | 項目の数・値などの仮想関数 |

`TextInputBase`・`ItemView` からは直接派生できません(ライブラリの中だけの基底)。入力欄・一覧を元にするときは、
`LineEdit`・`TextEdit`・`ListView`・`TreeView`・`TableView` から派生します。

## 上書きする口

`Widget` の protected の仮想関数のうち、上書きしてよいのは次のものです。基底の振る舞いも欲しいときは、基底の関数を呼びます。

- 描画と大きさ: `paint`・`resized`・`compute_preferred_size`・`compute_minimum_size`・`arrange_children`(レイアウトを使わずに子を並べる)
- 入力: `mouse_down`・`mouse_up`・`mouse_move`・`mouse_enter`・`mouse_leave`・`mouse_wheel`・`key_down`・`key_up`・`text_input`・
  `text_editing`・`focus_in`・`focus_out`
- ドラッグ&ドロップ: `drag_move`・`drag_leave`・`drop`・`drag_finished`
- 状態の変化: `scale_changed`(倍率)・`theme_changed`(テーマ)

`ButtonBase` は `click`(押された)を、`RangeBase` は `range_changed`(範囲・値が変わった)を加えます。
これらと protected の関数(`ButtonBase::state`・`text_layout`・`RangeBase::ratio`・`step_by` など)は、1.0 から互換を守ります
(docs/adr/0057)。

## 自分のプロパティ

プロパティは、所有者(`this`)と、値を読む関数・書く関数(どちらも `Object*` を受け取る静的な関数)から作ります。
利用者の操作などで値が変わったら、`notify_changed` で知らせます(結びつけた相手へ伝わります)。

```cpp
class LevelBar : public tokotachi::ProgressBar
{
public:
    explicit LevelBar(tokotachi::Widget* parent = nullptr)
        : tokotachi::ProgressBar(parent), warning_level(this, &LevelBar::get_level, &LevelBar::set_level), level_(80)
    {
    }

    /// これを超えたら警告とみなす値。
    tokotachi::Property<double> warning_level;

private:
    static double get_level(tokotachi::Object* self) { return static_cast<LevelBar*>(self)->level_; }
    static void set_level(tokotachi::Object* self, const double& v)
    {
        LevelBar* bar = static_cast<LevelBar*>(self);
        bar->level_ = v;
        bar->invalidate();
        bar->notify_changed(bar->warning_level);
    }

    double level_;
};
```

デザイナーは、プロジェクトに登録したヘッダからこのような部品を読み、パレットの「自作部品」に出します(docs/adr/0044)。

## 標準の見た目で描く

`<tokotachi/widgets/draw.hpp>` の `tokotachi::draw` の関数は、組み込みの部品と同じ見た目を、今のテーマ(部品ごとのテーマの上書きを
含む)で描きます。組み込みの部品も、これらの関数で自分を描いています。

| 関数 | 描くもの |
|---|---|
| `draw::button_face`・`draw::button_text_color` | ボタンの面(`ButtonStyle` の PRIMARY・SECONDARY)と、その上の文字の色 |
| `draw::check_mark`・`draw::radio_mark` | チェック・ラジオの印 |
| `draw::focus_ring` | キーボードのフォーカスの枠 |
| `draw::field_frame` | 入力欄の地と枠 |

状態は `ControlState`(有効・ホバー・押下・フォーカス・チェック)で渡します。`ButtonBase::state()` はそのまま渡せます。

```cpp
// 右上に未読の数を出すボタン
class BadgeButton : public tokotachi::ButtonBase
{
public:
    explicit BadgeButton(tokotachi::Widget* parent) : tokotachi::ButtonBase(parent, "受信") {}

    int unread = 3;

protected:
    void paint(tokotachi::Canvas& canvas) override
    {
        const tokotachi::Rect r = local_rect;
        const tokotachi::ControlState s = state();
        tokotachi::draw::button_face(canvas, r, s);
        const tokotachi::TextLayout& l = text_layout();
        canvas.draw_text(l, tokotachi::Point((r.width - l.width()) / 2, (r.height - l.height()) / 2),
                         tokotachi::draw::button_text_color(s));
        if (unread > 0)
        {
            canvas.fill_round_rect(tokotachi::Rect(r.width - 14, 4, 10, 10), 5, tokotachi::Color(220, 40, 40));
        }
        if (s.focused)
        {
            tokotachi::draw::focus_ring(canvas, r);
        }
    }

    tokotachi::Size compute_preferred_size() const override
    {
        return tokotachi::Size(std::ceil(text_layout().width()) + 40, std::ceil(text_layout().height()) + 16);
    }
};
```

```cpp
// 丸いつまみ(値は RangeBase の value。マウスを上下に動かして変える)
class Knob : public tokotachi::RangeBase
{
public:
    explicit Knob(tokotachi::Widget* parent) : tokotachi::RangeBase(parent), dragging_(false), last_y_(0) {}

protected:
    void paint(tokotachi::Canvas& canvas) override
    {
        const tokotachi::Rect r = local_rect;
        const tokotachi::Point c = r.center();
        const float radius = std::min(r.width, r.height) / 2 - 2;
        tokotachi::Path dial;
        dial.add_ellipse(c, radius, radius);
        canvas.fill_path(dial, theme.get()[tokotachi::ColorRole::FIELD]);
        const float angle = static_cast<float>(-2.4 + 4.8 * ratio());  // 範囲の中の位置(0〜1)を角度に
        canvas.draw_line(c, tokotachi::Point(c.x + radius * std::sin(angle), c.y - radius * std::cos(angle)), 3,
                         theme.get()[tokotachi::ColorRole::ACCENT]);
    }

    void mouse_down(tokotachi::MouseEvent& e) override
    {
        dragging_ = true;
        last_y_ = e.pos.y;
        e.accepted = true;
    }

    void mouse_move(tokotachi::MouseEvent& e) override
    {
        if (dragging_)
        {
            step_by((last_y_ - e.pos.y) * single_step.get());  // 上へ動かすと増える(single_step に合わせる)
            last_y_ = e.pos.y;
        }
    }

    void mouse_up(tokotachi::MouseEvent&) override { dragging_ = false; }

private:
    bool dragging_;
    float last_y_;
};
```
