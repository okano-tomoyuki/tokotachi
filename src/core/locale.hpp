// 利用者の言語(内部。docs/adr/0051)。標準の文字(ダイアログのボタン・暦の月と曜日など)を選ぶ。
#ifndef TOKOTACHI_SRC_CORE_LOCALE_HPP
#define TOKOTACHI_SRC_CORE_LOCALE_HPP

#include <string>

namespace tokotachi
{
namespace detail
{

/// OS の設定の最初の言語(「ja」「en-US」など。分からなければ空)。
std::string system_language();

/// 利用者の言語(Application::language。Application が無ければ OS の言語)が日本語か。
bool prefers_japanese();

}  // namespace detail
}  // namespace tokotachi

#endif
