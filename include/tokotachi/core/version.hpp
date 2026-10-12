/// @file
/// ライブラリの版(docs/adr/0037)。CMake もこのファイルから版を読む(版はここだけで管理する)。
///
/// @code
/// #if TOKOTACHI_VERSION_AT_LEAST(0, 2, 0)   // コンパイル時の版で分ける
/// std::string v = tokotachi::version();     // 実行時に読み込まれたライブラリの版("0.2.0")
/// @endcode
#ifndef TOKOTACHI_CORE_VERSION_HPP
#define TOKOTACHI_CORE_VERSION_HPP

#include "export.hpp"

/// 大きい版(コンパイルに使ったヘッダの版)。
#define TOKOTACHI_VERSION_MAJOR 0
/// 小さい版。
#define TOKOTACHI_VERSION_MINOR 2
/// 修正の版。
#define TOKOTACHI_VERSION_PATCH 0
/// 版の文字("major.minor.patch")。
#define TOKOTACHI_VERSION_STRING "0.2.0"

/// コンパイルに使ったヘッダの版が major.minor.patch 以上か。
#define TOKOTACHI_VERSION_AT_LEAST(major, minor, patch)                                                               \
    (TOKOTACHI_VERSION_MAJOR > (major) ||                                                                              \
     (TOKOTACHI_VERSION_MAJOR == (major) &&                                                                            \
      (TOKOTACHI_VERSION_MINOR > (minor) || (TOKOTACHI_VERSION_MINOR == (minor) && TOKOTACHI_VERSION_PATCH >= (patch)))))

namespace tokotachi
{

/// 実行時に読み込まれたライブラリの版(共有ライブラリが、コンパイルに使ったヘッダと違う版のこともある)。
TOKOTACHI_API const char* version();

}  // namespace tokotachi

#endif
