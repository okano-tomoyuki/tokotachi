// 結びつきの一覧(内部。docs/adr/0016)。
#ifndef TOKOTACHI_SRC_CORE_BINDING_REGISTRY_HPP
#define TOKOTACHI_SRC_CORE_BINDING_REGISTRY_HPP

#include <tokotachi/core/binding.hpp>

namespace tokotachi
{
namespace detail
{

// owner の結びつきをすべて外す(Object のデストラクタが呼ぶ)。
void remove_bindings_of(Object* owner);

// いま登録されている結びつきの数(テスト用)。
std::size_t binding_count();

}  // namespace detail
}  // namespace tokotachi

#endif
