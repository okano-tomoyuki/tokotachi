#include <tokotachi/windows/message_dialog.hpp>

#include <tokotachi/layout/box_layout.hpp>
#include <tokotachi/graphics/canvas.hpp>
#include <tokotachi/widgets/label.hpp>

#include "../core/impl.hpp"
#include "../widgets/theme.hpp"

namespace tokotachi
{

namespace detail
{

// 種類の印(色の付いた丸と記号)。
class MessageIcon : public Widget
{
public:
    explicit MessageIcon(Widget* parent_widget) : Widget(parent_widget), type(MessageType::INFORMATION) {}

    MessageType type;

protected:
    Size compute_preferred_size() const override { return Size(32, 32); }
    Size compute_minimum_size() const override { return Size(32, 32); }

    void paint(Canvas& canvas) override
    {
        Color fill = theme::accent();
        const char* glyph = "i";
        switch (type)
        {
        case MessageType::QUESTION:
            glyph = "?";
            break;
        case MessageType::WARNING:
            fill = Color::rgb(0xd97706);
            glyph = "!";
            break;
        case MessageType::CRITICAL:
            fill = Color::rgb(0xdc2626);
            glyph = "\xC3\x97";  // ×
            break;
        default:
            break;
        }
        canvas.fill_round_rect(Rect(0, 0, 32, 32), 16, fill);
        const TextLayout l(glyph, Font(20), display_scale);
        canvas.draw_text(l, Point((32 - l.width()) / 2, (32 - l.height()) / 2), Color::rgb(0xffffff));
        Widget::paint(canvas);
    }
};

}  // namespace detail

class MessageDialog::Impl
{
    TOKOTACHI_IMPL(MessageDialog);

    explicit Impl(MessageDialog* object) : q(object) {}

    std::string get_text() const;
    void set_text(const std::string& v);
    MessageType get_type() const { return type_; }
    void set_type(const MessageType& v);

    MessageType type_;
    detail::MessageIcon* icon_;
    Label* label_;
};

MessageDialog::MessageDialog(Window* parent, const std::string& title_text, const std::string& initial_text,
                             MessageType initial_type)
    : Dialog(parent, title_text),
      TOKOTACHI_IMPL_PROPERTY_GETTER(Impl, std::string, text),
      TOKOTACHI_IMPL_PROPERTY_GETTER(Impl, MessageType, type),
      d_(new Impl(this))
{
    d_->type_ = initial_type;
    d_->icon_ = nullptr;
    d_->label_ = nullptr;
    BoxLayout* box = new BoxLayout(this, Orientation::HORIZONTAL);
    box->margins = Margins(20, 20, 20, 16);
    box->spacing = 16;
    d_->icon_ = new detail::MessageIcon(this);
    d_->icon_->type = initial_type;
    d_->label_ = new Label(this, initial_text);
    d_->label_->word_wrap = true;
    box->add(d_->icon_, 0, Align::START);
    box->add(d_->label_, 1);
}

MessageDialog::~MessageDialog() { delete d_; }

std::string MessageDialog::Impl::get_text() const { return label_->text; }

void MessageDialog::Impl::set_text(const std::string& v) { label_->text = v; }

void MessageDialog::Impl::set_type(const MessageType& v)
{
    type_ = v;
    icon_->type = v;
    icon_->invalidate();
}

MessageDialog* MessageDialog::show(Window* parent, const std::string& title_text, const std::string& message,
                                   const std::vector<DialogButton>& buttons, const NotifyHandler& handler,
                                   MessageType message_type)
{
    MessageDialog* d = new MessageDialog(parent, title_text, message, message_type);
    if (buttons.empty())
    {
        d->add_button(DialogButton::OK);
    }
    for (std::size_t i = 0; i < buttons.size(); ++i)
    {
        d->add_button(buttons[i]);
    }
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

}  // namespace tokotachi
