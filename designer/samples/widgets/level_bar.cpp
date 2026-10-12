#include "level_bar.hpp"

namespace samples
{

LevelBar::LevelBar(tokotachi::Widget* parent)
    : tokotachi::ProgressBar(parent),
      warning_level(this, &LevelBar::get_warning_level, &LevelBar::set_warning_level),
      on_warning(this, &LevelBar::get_on_warning, &LevelBar::set_on_warning),
      warning_level_(80)
{
}

bool LevelBar::is_warning() const
{
    return value.get() > warning_level_;
}

double LevelBar::get_warning_level(tokotachi::Object* self)
{
    return static_cast<LevelBar*>(self)->warning_level_;
}

void LevelBar::set_warning_level(tokotachi::Object* self, const double& value)
{
    static_cast<LevelBar*>(self)->warning_level_ = value;
}

tokotachi::NotifyHandler LevelBar::get_on_warning(tokotachi::Object* self)
{
    return static_cast<LevelBar*>(self)->on_warning_;
}

void LevelBar::set_on_warning(tokotachi::Object* self, const tokotachi::NotifyHandler& handler)
{
    static_cast<LevelBar*>(self)->on_warning_ = handler;
}

}  // namespace samples
