// 自作部品の見本(docs/adr/0044)。デザイナーはこのヘッダを読み、パレットの「自作部品」に出す。
#ifndef SAMPLES_LEVEL_BAR_HPP
#define SAMPLES_LEVEL_BAR_HPP

#include <tokotachi/tokotachi.hpp>

namespace samples
{

/// 警告の段階を持つ進み具合のバー。
class LevelBar : public tokotachi::ProgressBar
{
public:
    explicit LevelBar(tokotachi::Widget* parent = nullptr);

    /// これを超えたら警告とみなす値。
    tokotachi::Property<double> warning_level;
    /// 警告の段階を超えたときに呼ぶ。
    tokotachi::Property<tokotachi::NotifyHandler> on_warning;

    /// 今の値が警告の段階を超えているか。
    bool is_warning() const;

private:
    static double get_warning_level(tokotachi::Object* self);
    static void set_warning_level(tokotachi::Object* self, const double& value);
    static tokotachi::NotifyHandler get_on_warning(tokotachi::Object* self);
    static void set_on_warning(tokotachi::Object* self, const tokotachi::NotifyHandler& handler);
    double warning_level_;
    tokotachi::NotifyHandler on_warning_;
};

}  // namespace samples

#endif  // SAMPLES_LEVEL_BAR_HPP
