// <tokotachi-designer:begin id="tokotachi_source_begin">
#include "labeled_edit.hpp"
// <tokotachi-designer:end id="tokotachi_source_begin" hash="ed02c380">

// <tokotachi-designer:begin id="tokotachi_namespace_begin">
// <tokotachi-designer:end id="tokotachi_namespace_begin" hash="811c9dc5">

LabeledEdit::LabeledEdit(tokotachi::Widget* parent)
    : tokotachi::Widget(parent)
{
    tokotachi_setup_ui();
}

// <tokotachi-designer:begin id="tokotachi_setup_ui">
// Creates the widgets, sets their properties, adds them to the layouts and connects the handlers (generated).
void LabeledEdit::tokotachi_setup_ui()
{
    tokotachi::BoxLayout* window_layout = new tokotachi::BoxLayout(this);
    window_layout->orientation = tokotachi::Orientation::HORIZONTAL;

    label_caption = new tokotachi::Label(this);
    label_caption->text = "名前";
    window_layout->add(label_caption);

    line_edit_value = new tokotachi::LineEdit(this);
    line_edit_value->on_change = [this](tokotachi::Object* sender) { value_change(sender); };
    window_layout->add(line_edit_value, 1);
}
// <tokotachi-designer:end id="tokotachi_setup_ui" hash="b48ed944">

// <tokotachi-designer:handler-stubs>
void LabeledEdit::value_change(tokotachi::Object* sender)
{
}

// <tokotachi-designer:begin id="tokotachi_namespace_end">
// <tokotachi-designer:end id="tokotachi_namespace_end" hash="811c9dc5">
