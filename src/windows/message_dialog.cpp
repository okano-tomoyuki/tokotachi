#include <tokotachi/windows/message_dialog.hpp>

#include <tokotachi/layout/box_layout.hpp>
#include <tokotachi/graphics/canvas.hpp>
#include <tokotachi/widgets/label.hpp>

#include "../core/property_impl.hpp"
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

MessageDialog::MessageDialog(Window* parent, const std::string& title_text, const std::string& initial_text,
                             MessageType initial_type)
    : Dialog(parent, title_text),
      TOKOTACHI_PROPERTY_GETTER(MessageDialog, std::string, text),
      TOKOTACHI_PROPERTY_GETTER(MessageDialog, MessageType, type),
      type_(initial_type),
      icon_(nullptr),
      label_(nullptr)
{
    BoxLayout* box = new BoxLayout(this, Orientation::HORIZONTAL);
    box->margins = Margins(20, 20, 20, 16);
    box->spacing = 16;
    icon_ = new detail::MessageIcon(this);
    icon_->type = initial_type;
    label_ = new Label(this, initial_text);
    label_->word_wrap = true;
    box->add(icon_, 0, Align::START);
    box->add(label_, 1);
}

std::string MessageDialog::get_text() const { return label_->text; }

void MessageDialog::set_text(const std::string& v) { label_->text = v; }

void MessageDialog::set_type(const MessageType& v)
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
