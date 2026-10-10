// 名前から部品を作り、プロパティを名前で読み書きする対応表(docs/adr/0039)。
// 中身はカタログから生成する(generated/table.cpp。tools/canvas/table.mts)。
#ifndef TOKOTACHI_CANVAS_TABLE_HPP
#define TOKOTACHI_CANVAS_TABLE_HPP

#include <string>
#include <vector>

#include <tokotachi/tokotachi.hpp>

#include "json.hpp"

namespace ttc
{

/// クラスの名前から作る(部品・レイアウトは parent が Widget、Action・Timer 等は持ち主)。作れないクラスなら nullptr。
/// Window・Dialog、add_menu・add_tab 等の結果(Menu・Tab・SplitterPane)は作らない。
tokotachi::Object* create(const std::string& cls, tokotachi::Object* parent);

/// プロパティを設定する。クラス・プロパティが無いか、値が型に合わなければ false。
bool set_property(tokotachi::Object* object, const std::string& cls, const std::string& property, const Json& value);

/// プロパティの値を読む。無ければ false。
bool get_property(tokotachi::Object* object, const std::string& cls, const std::string& property, Json& out);

/// クラスのデザイン時に設定できるプロパティ(カタログの順)。
std::vector<std::string> property_names(const std::string& cls);

/// create で作れるクラス(カタログの順)。
std::vector<std::string> creatable_classes();

}  // namespace ttc

#endif
