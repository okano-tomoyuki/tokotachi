#include <tokotachi/windows/color_dialog.hpp>

#include <algorithm>
#include <cmath>
#include <vector>

#include <tokotachi/graphics/canvas.hpp>
#include <tokotachi/layout/box_layout.hpp>
#include <tokotachi/layout/grid_layout.hpp>
#include <tokotachi/widgets/label.hpp>
#include <tokotachi/widgets/line_edit.hpp>
#include <tokotachi/widgets/spin_box.hpp>

#include "../core/locale.hpp"
#include "../core/impl.hpp"
#include "../widgets/color_util.hpp"
#include "../widgets/theme.hpp"

namespace tokotachi
{

namespace detail
{
class ColorPlane;
class ColorStrip;
class ColorPreview;
class ColorSwatches;
}  // namespace detail

class ColorDialog::Impl
{
    TOKOTACHI_IMPL(ColorDialog);

    explicit Impl(ColorDialog* object) : q(object) {}

    Color get_color() const { return color_; }
    void set_color(const Color& v);
    bool get_alpha_enabled() const { return alpha_enabled_; }
    void set_alpha_enabled(const bool& v);

    /// 色相・彩度・明度・不透明度から色を決め、ほかの欄を合わせる(changed_by の欄は変えない)。
    void apply_hsv(const void* changed_by);
    /// 色から色相などを決め、ほかの欄を合わせる。
    void apply_color(const Color& c, const void* changed_by);
    /// 欄(16 進数・数値・四角・帯・見本)を今の色に合わせる。
    void sync(const void* changed_by);

    Color color_;
    Color original_;    ///< 開いたときの色(比べて見せる)
    bool alpha_enabled_;
    float hue_;         ///< 0〜360
    float saturation_;  ///< 0〜1
    float value_;       ///< 0〜1
    bool syncing_;
    detail::ColorPlane* plane_;
    detail::ColorStrip* hue_strip_;
    detail::ColorStrip* alpha_strip_;
    detail::ColorPreview* preview_;
    detail::ColorSwatches* swatches_;
    LineEdit* hex_;
    SpinBox* channels_[4];  ///< R・G・B・A
    Label* alpha_label_;
};

namespace detail
{

namespace
{

// 選んでいる位置の印(白と黒の二重の輪)。
void paint_marker(Canvas& canvas, const Rect& r)
{
    canvas.stroke_round_rect(r, r.width / 2, 2, Color(255, 255, 255));
    canvas.stroke_round_rect(r.adjusted(-1.5f, -1.5f, -1.5f, -1.5f), r.width / 2 + 1.5f, 1, Color(0, 0, 0, 160));
}

}  // namespace

// 彩度(横)と明度(縦)の四角。
class ColorPlane : public Widget
{
public:
    ColorPlane(Widget* parent_widget, ColorDialog* dialog) : Widget(parent_widget), dialog_(dialog), pressed_(false)
    {
        focus_policy = FocusPolicy::TAB_AND_CLICK;
        minimum_size = Size(220, 170);
    }

protected:
    void paint(Canvas& canvas) override
    {
        const Rect r = local_rect;
        // 色相の色に、左から白を、上から黒を重ねる
        canvas.fill_rect(r, color_from_hsv(detail::ImplAccess<ColorDialog>::of(dialog_)->hue_, 1, 1));
        std::vector<GradientStop> white;
        white.push_back(GradientStop(0, Color(255, 255, 255)));
        white.push_back(GradientStop(1, Color(255, 255, 255, 0)));
        canvas.fill_round_rect(r, 0, Paint::linear(Point(r.x, 0), Point(r.right(), 0), white));
        std::vector<GradientStop> black;
        black.push_back(GradientStop(0, Color(0, 0, 0, 0)));
        black.push_back(GradientStop(1, Color(0, 0, 0)));
        canvas.fill_round_rect(r, 0, Paint::linear(Point(0, r.y), Point(0, r.bottom()), black));
        canvas.stroke_rect(r.adjusted(0.5f, 0.5f, 0.5f, 0.5f), 1, focused ? theme::focus_ring() : theme::border());
        const float x = detail::ImplAccess<ColorDialog>::of(dialog_)->saturation_ * r.width, y = (1 - detail::ImplAccess<ColorDialog>::of(dialog_)->value_) * r.height;
        canvas.save();
        canvas.clip_rect(r);
        paint_marker(canvas, Rect(x - 6, y - 6, 12, 12));
        canvas.restore();
        Widget::paint(canvas);
    }

    void mouse_down(MouseEvent& e) override
    {
        Widget::mouse_down(e);
        if (e.button == MouseButton::LEFT)
        {
            e.accepted = true;
            pressed_ = true;
            pick(e.pos);
        }
    }

    void mouse_move(MouseEvent& e) override
    {
        Widget::mouse_move(e);
        if (pressed_)
        {
            pick(e.pos);
        }
    }

    void mouse_up(MouseEvent& e) override
    {
        Widget::mouse_up(e);
        pressed_ = false;
    }

    void key_down(KeyEvent& e) override
    {
        Widget::key_down(e);
        const float step = e.has_modifier(MODIFIER_SHIFT) ? 0.1f : 0.01f;
        float ds = 0, dv = 0;
        switch (e.key)
        {
        case Key::LEFT:
            ds = -step;
            break;
        case Key::RIGHT:
            ds = step;
            break;
        case Key::UP:
            dv = step;
            break;
        case Key::DOWN:
            dv = -step;
            break;
        default:
            return;
        }
        e.accepted = true;
        detail::ImplAccess<ColorDialog>::of(dialog_)->saturation_ = std::max(0.0f, std::min(1.0f, detail::ImplAccess<ColorDialog>::of(dialog_)->saturation_ + ds));
        detail::ImplAccess<ColorDialog>::of(dialog_)->value_ = std::max(0.0f, std::min(1.0f, detail::ImplAccess<ColorDialog>::of(dialog_)->value_ + dv));
        detail::ImplAccess<ColorDialog>::of(dialog_)->apply_hsv(this);
    }

    void focus_in() override
    {
        invalidate();
        Widget::focus_in();
    }

    void focus_out() override
    {
        invalidate();
        Widget::focus_out();
    }

private:
    void pick(const Point& p)
    {
        const Rect r = local_rect;
        detail::ImplAccess<ColorDialog>::of(dialog_)->saturation_ = std::max(0.0f, std::min(1.0f, p.x / std::max(1.0f, r.width)));
        detail::ImplAccess<ColorDialog>::of(dialog_)->value_ = 1 - std::max(0.0f, std::min(1.0f, p.y / std::max(1.0f, r.height)));
        detail::ImplAccess<ColorDialog>::of(dialog_)->apply_hsv(this);
    }

    ColorDialog* dialog_;
    bool pressed_;
};

// 色相・不透明度の横の帯。
class ColorStrip : public Widget
{
public:
    ColorStrip(Widget* parent_widget, ColorDialog* dialog, bool alpha)
        : Widget(parent_widget), dialog_(dialog), alpha_(alpha), pressed_(false)
    {
        focus_policy = FocusPolicy::TAB_AND_CLICK;
        minimum_size = Size(220, 18);
    }

protected:
    Size compute_preferred_size() const override { return Size(220, 18); }

    void paint(Canvas& canvas) override
    {
        const Rect r = local_rect;
        const Rect band = r.adjusted(0, 2, 0, 2);
        std::vector<GradientStop> stops;
        if (alpha_)
        {
            paint_checker(canvas, band);
            const Color c = detail::ImplAccess<ColorDialog>::of(dialog_)->color_;
            stops.push_back(GradientStop(0, c.with_alpha(0)));
            stops.push_back(GradientStop(1, c.with_alpha(255)));
        }
        else
        {
            for (int k = 0; k <= 6; ++k)
            {
                stops.push_back(GradientStop(k / 6.0f, color_from_hsv(k * 60.0f, 1, 1)));
            }
        }
        canvas.fill_round_rect(band, 0, Paint::linear(Point(band.x, 0), Point(band.right(), 0), stops));
        canvas.stroke_rect(band.adjusted(0.5f, 0.5f, 0.5f, 0.5f), 1, focused ? theme::focus_ring() : theme::border());
        const float t = alpha_ ? detail::ImplAccess<ColorDialog>::of(dialog_)->color_.a / 255.0f : detail::ImplAccess<ColorDialog>::of(dialog_)->hue_ / 360.0f;
        const float x = band.x + t * band.width;
        paint_marker(canvas, Rect(x - 4, r.y + 1, 8, r.height - 2));
        Widget::paint(canvas);
    }

    void mouse_down(MouseEvent& e) override
    {
        Widget::mouse_down(e);
        if (e.button == MouseButton::LEFT)
        {
            e.accepted = true;
            pressed_ = true;
            pick(e.pos.x / std::max(1.0f, width.get()));
        }
    }

    void mouse_move(MouseEvent& e) override
    {
        Widget::mouse_move(e);
        if (pressed_)
        {
            pick(e.pos.x / std::max(1.0f, width.get()));
        }
    }

    void mouse_up(MouseEvent& e) override
    {
        Widget::mouse_up(e);
        pressed_ = false;
    }

    void key_down(KeyEvent& e) override
    {
        Widget::key_down(e);
        if (e.key != Key::LEFT && e.key != Key::RIGHT)
        {
            return;
        }
        e.accepted = true;
        const float step = (e.key == Key::LEFT ? -1.0f : 1.0f) * (e.has_modifier(MODIFIER_SHIFT) ? 10.0f : 1.0f);
        const float now = alpha_ ? detail::ImplAccess<ColorDialog>::of(dialog_)->color_.a / 255.0f : detail::ImplAccess<ColorDialog>::of(dialog_)->hue_ / 360.0f;
        pick(now + step / (alpha_ ? 255.0f : 360.0f));
    }

    void focus_in() override
    {
        invalidate();
        Widget::focus_in();
    }

    void focus_out() override
    {
        invalidate();
        Widget::focus_out();
    }

private:
    void pick(float t)
    {
        t = std::max(0.0f, std::min(1.0f, t));
        if (alpha_)
        {
            detail::ImplAccess<ColorDialog>::of(dialog_)->color_.a = static_cast<std::uint8_t>(std::floor(t * 255 + 0.5f));
        }
        else
        {
            detail::ImplAccess<ColorDialog>::of(dialog_)->hue_ = std::min(t * 360, 359.99f);
        }
        detail::ImplAccess<ColorDialog>::of(dialog_)->apply_hsv(this);
    }

    ColorDialog* dialog_;
    bool alpha_;
    bool pressed_;
};

// 選んでいる色(上)と、開いたときの色(下。押すと戻す)。
class ColorPreview : public Widget
{
public:
    ColorPreview(Widget* parent_widget, ColorDialog* dialog) : Widget(parent_widget), dialog_(dialog) {}

protected:
    Size compute_preferred_size() const override { return Size(120, 56); }

    void paint(Canvas& canvas) override
    {
        const Rect r = local_rect;
        paint_checker(canvas, r);
        canvas.fill_rect(Rect(r.x, r.y, r.width, r.height / 2), detail::ImplAccess<ColorDialog>::of(dialog_)->color_);
        canvas.fill_rect(Rect(r.x, r.y + r.height / 2, r.width, r.height - r.height / 2), detail::ImplAccess<ColorDialog>::of(dialog_)->original_);
        canvas.stroke_rect(r.adjusted(0.5f, 0.5f, 0.5f, 0.5f), 1, theme::border());
        Widget::paint(canvas);
    }

    void mouse_down(MouseEvent& e) override
    {
        Widget::mouse_down(e);
        if (e.button == MouseButton::LEFT && e.pos.y >= height.get() / 2)
        {
            e.accepted = true;
            detail::ImplAccess<ColorDialog>::of(dialog_)->apply_color(detail::ImplAccess<ColorDialog>::of(dialog_)->original_, this);
        }
    }

private:
    ColorDialog* dialog_;
};

// よく使う色の見本。
class ColorSwatches : public Widget
{
public:
    ColorSwatches(Widget* parent_widget, ColorDialog* dialog) : Widget(parent_widget), dialog_(dialog), hover_(-1)
    {
        static const unsigned COLORS[] = {0x000000, 0x404040, 0x808080, 0xC0C0C0, 0xFFFFFF, 0x7F1D1D, 0xDC2626, 0xF97316,
                                          0xFACC15, 0x84CC16, 0x16A34A, 0x0D9488, 0x0EA5E9, 0x2563EB, 0x7C3AED, 0xDB2777};
        for (std::size_t i = 0; i < sizeof(COLORS) / sizeof(COLORS[0]); ++i)
        {
            colors_.push_back(Color::rgb(COLORS[i]));
        }
    }

protected:
    static const int COLUMNS = 8;
    static float cell() { return 22; }
    static float gap() { return 4; }

    Size compute_preferred_size() const override
    {
        const int rows = (static_cast<int>(colors_.size()) + COLUMNS - 1) / COLUMNS;
        return Size(COLUMNS * cell() + (COLUMNS - 1) * gap(), rows * cell() + (rows - 1) * gap());
    }

    Rect rect_of(int i) const
    {
        return Rect((i % COLUMNS) * (cell() + gap()), (i / COLUMNS) * (cell() + gap()), cell(), cell());
    }

    int index_at(const Point& p) const
    {
        for (int i = 0; i < static_cast<int>(colors_.size()); ++i)
        {
            if (rect_of(i).contains(p))
            {
                return i;
            }
        }
        return -1;
    }

    void paint(Canvas& canvas) override
    {
        for (int i = 0; i < static_cast<int>(colors_.size()); ++i)
        {
            const Rect r = rect_of(i);
            canvas.fill_round_rect(r, 3, colors_[static_cast<std::size_t>(i)]);
            canvas.stroke_round_rect(r.adjusted(0.5f, 0.5f, 0.5f, 0.5f), 3, i == hover_ ? 2.0f : 1.0f,
                                     i == hover_ ? theme::accent() : theme::border());
        }
        Widget::paint(canvas);
    }

    void mouse_move(MouseEvent& e) override
    {
        Widget::mouse_move(e);
        const int i = index_at(e.pos);
        if (i != hover_)
        {
            hover_ = i;
            invalidate();
        }
    }

    void mouse_leave() override
    {
        hover_ = -1;
        invalidate();
        Widget::mouse_leave();
    }

    void mouse_down(MouseEvent& e) override
    {
        Widget::mouse_down(e);
        const int i = index_at(e.pos);
        if (e.button == MouseButton::LEFT && i >= 0)
        {
            e.accepted = true;
            // 見本は不透明。不透明度は今のまま
            detail::ImplAccess<ColorDialog>::of(dialog_)->apply_color(colors_[static_cast<std::size_t>(i)].with_alpha(detail::ImplAccess<ColorDialog>::of(dialog_)->color_.a), this);
        }
    }

private:
    ColorDialog* dialog_;
    std::vector<Color> colors_;
    int hover_;
};

}  // namespace detail

namespace
{

std::string default_title() { return detail::prefers_japanese() ? "色の選択" : "Select Color"; }

}  // namespace

ColorDialog::ColorDialog(Window* parent, const std::string& title_text)
    : Dialog(parent, title_text.empty() ? default_title() : title_text),
      TOKOTACHI_IMPL_PROPERTY_GETTER(Impl, Color, color),
      TOKOTACHI_IMPL_PROPERTY_GETTER(Impl, bool, alpha_enabled),
      d_(new Impl(this))
{
    d_->color_ = Color(255, 255, 255);
    d_->original_ = Color(255, 255, 255);
    d_->alpha_enabled_ = false;
    d_->hue_ = 0;
    d_->saturation_ = 0;
    d_->value_ = 1;
    d_->syncing_ = false;
    BoxLayout* box = new BoxLayout(this, Orientation::HORIZONTAL);
    box->margins = Margins(16, 16, 16, 8);
    box->spacing = 16;

    // 左: 四角と帯
    Widget* left = new Widget(this);
    BoxLayout* left_box = new BoxLayout(left);
    left_box->spacing = 8;
    d_->plane_ = new detail::ColorPlane(left, this);
    d_->hue_strip_ = new detail::ColorStrip(left, this, false);
    d_->alpha_strip_ = new detail::ColorStrip(left, this, true);
    left_box->add(d_->plane_, 1);
    left_box->add(d_->hue_strip_);
    left_box->add(d_->alpha_strip_);

    // 右: 比べる見本・16 進数・RGB(A)・よく使う色
    Widget* right = new Widget(this);
    BoxLayout* right_box = new BoxLayout(right);
    right_box->spacing = 10;
    d_->preview_ = new detail::ColorPreview(right, this);
    right_box->add(d_->preview_);
    Widget* form = new Widget(right);
    GridLayout* grid = new GridLayout(form);
    grid->row_spacing = 6;
    grid->column_spacing = 8;
    const bool ja = detail::prefers_japanese();
    grid->add(new Label(form, ja ? "16 進数" : "Hex"), 0, 0);
    d_->hex_ = new LineEdit(form);
    grid->add(d_->hex_, 0, 1);
    const char* const NAMES[] = {"R", "G", "B", "A"};
    for (int k = 0; k < 4; ++k)
    {
        Label* label = new Label(form, NAMES[k]);
        grid->add(label, k + 1, 0);
        SpinBox* spin = new SpinBox(form);
        spin->maximum = 255;
        grid->add(spin, k + 1, 1);
        d_->channels_[k] = spin;
        if (k == 3)
        {
            d_->alpha_label_ = label;
        }
        spin->on_change = [this, k](Object*)
        {
            if (d_->syncing_)
            {
                return;
            }
            Color c = d_->color_;
            const std::uint8_t v = static_cast<std::uint8_t>(d_->channels_[k]->value.get());
            (k == 0 ? c.r : k == 1 ? c.g : k == 2 ? c.b : c.a) = v;
            d_->apply_color(c, d_->channels_[k]);
        };
    }
    grid->set_column_stretch(1, 1);
    right_box->add(form);
    d_->swatches_ = new detail::ColorSwatches(right, this);
    right_box->add(d_->swatches_);
    right_box->add_stretch();

    box->add(left, 1);
    box->add(right);

    // 16 進数: 打ち込み中も、読める長さになったら色にする。Enter で読めなければ元に戻す
    d_->hex_->on_change = [this](Object*)
    {
        if (d_->syncing_)
        {
            return;
        }
        const std::string s = d_->hex_->text;
        Color c;
        const std::size_t digits = s.size() - (s.find('#') == std::string::npos ? 0 : 1);
        if ((digits == 6 || digits == 8) && detail::color_from_hex(s, c))
        {
            d_->apply_color(digits == 8 ? c : c.with_alpha(d_->color_.a), d_->hex_);
        }
    };
    d_->hex_->on_submit = [this](Object*)
    {
        Color c;
        if (detail::color_from_hex(d_->hex_->text, c))
        {
            d_->apply_color(d_->hex_->text.get().size() > 7 ? c : c.with_alpha(d_->color_.a), nullptr);
        }
        else
        {
            d_->sync(nullptr);
        }
    };

    add_button(DialogButton::OK);
    add_button(DialogButton::CANCEL);
    d_->set_alpha_enabled(false);
    d_->sync(nullptr);
}

ColorDialog::~ColorDialog() { delete d_; }

ColorDialog* ColorDialog::show(Window* parent, const Color& initial, const NotifyHandler& handler, bool alpha,
                               const std::string& title_text)
{
    ColorDialog* d = new ColorDialog(parent, title_text);
    d->alpha_enabled = alpha;
    d->color = initial;
    d->on_finish = [handler](Object* sender)
    {
        if (handler)
        {
            handler(sender);
        }
        sender->delete_later();
    };
    d->open();
    return d;
}

void ColorDialog::Impl::set_color(const Color& v)
{
    color_ = alpha_enabled_ ? v : v.with_alpha(255);
    original_ = color_;
    detail::color_to_hsv(color_, hue_, saturation_, value_);
    sync(nullptr);
}

void ColorDialog::Impl::set_alpha_enabled(const bool& v)
{
    alpha_enabled_ = v;
    alpha_strip_->visible = v;
    alpha_label_->visible = v;
    channels_[3]->visible = v;
    if (!v && color_.a != 255)
    {
        color_.a = 255;
        original_.a = 255;
        q->notify_changed(q->color);
    }
    sync(nullptr);
}

void ColorDialog::Impl::apply_hsv(const void* changed_by)
{
    color_ = detail::color_from_hsv(hue_, saturation_, value_, color_.a);
    sync(changed_by);
    q->notify_changed(q->color);
}

void ColorDialog::Impl::apply_color(const Color& c, const void* changed_by)
{
    color_ = alpha_enabled_ ? c : c.with_alpha(255);
    detail::color_to_hsv(color_, hue_, saturation_, value_);
    sync(changed_by);
    q->notify_changed(q->color);
}

void ColorDialog::Impl::sync(const void* changed_by)
{
    syncing_ = true;
    if (changed_by != hex_)
    {
        hex_->text = detail::color_to_hex(color_, alpha_enabled_);
    }
    const int channels[] = {color_.r, color_.g, color_.b, color_.a};
    for (int k = 0; k < 4; ++k)
    {
        if (changed_by != channels_[k])
        {
            channels_[k]->value = channels[k];
        }
    }
    plane_->invalidate();
    hue_strip_->invalidate();
    alpha_strip_->invalidate();
    preview_->invalidate();
    syncing_ = false;
}

}  // namespace tokotachi
