// <tokotachi-designer:begin id="tokotachi_header_begin">
#ifndef LABELED_EDIT_HPP
#define LABELED_EDIT_HPP

#include <tokotachi/tokotachi.hpp>
// <tokotachi-designer:end id="tokotachi_header_begin" hash="87f563d8">

// <tokotachi-designer:begin id="tokotachi_namespace_begin">
// <tokotachi-designer:end id="tokotachi_namespace_begin" hash="811c9dc5">

/// Widget created with the Tokotachi designer (labeled_edit.ttui.json). Regions enclosed in markers are overwritten when regenerated.
class LabeledEdit : public tokotachi::Widget
{
public:
    explicit LabeledEdit(tokotachi::Widget* parent);

    // <tokotachi-designer:begin id="declarations">
    tokotachi::Label* label_caption;
    tokotachi::LineEdit* line_edit_value;

    void value_change(tokotachi::Object* sender);
    // <tokotachi-designer:end id="declarations" hash="adbf2523">

private:
    void tokotachi_setup_ui();
};

// <tokotachi-designer:begin id="tokotachi_namespace_end">
// <tokotachi-designer:end id="tokotachi_namespace_end" hash="811c9dc5">

// <tokotachi-designer:begin id="tokotachi_header_end">
#endif  // LABELED_EDIT_HPP
// <tokotachi-designer:end id="tokotachi_header_end" hash="b9c16149">
