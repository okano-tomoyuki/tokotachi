// ライブラリの版(docs/adr/0037)。CMake もこのファイルから版を読む(版はここだけで管理する)。
//
//   #if TOKOTACHI_VERSION_AT_LEAST(0, 2, 0)   // コンパイル時の版で分ける
//   std::string v = tokotachi::version();     // 実行時に読み込まれたライブラリの版("0.1.0")
#ifndef TOKOTACHI_CORE_VERSION_HPP
#define TOKOTACHI_CORE_VERSION_HPP

#define TOKOTACHI_VERSION_MAJOR 0
#define TOKOTACHI_VERSION_MINOR 1
#define TOKOTACHI_VERSION_PATCH 0
#define TOKOTACHI_VERSION_STRING "0.1.0"

// コンパイルに使ったヘッダの版が major.minor.patch 以上か。
#define TOKOTACHI_VERSION_AT_LEAST(major, minor, patch)                                                               \
    (TOKOTACHI_VERSION_MAJOR > (major) ||                                                                              \
     (TOKOTACHI_VERSION_MAJOR == (major) &&                                                                            \
      (TOKOTACHI_VERSION_MINOR > (minor) || (TOKOTACHI_VERSION_MINOR == (minor) && TOKOTACHI_VERSION_PATCH >= (patch)))))

namespace tokotachi
{

// 実行時に読み込まれたライブラリの版(共有ライブラリが、コンパイルに使ったヘッダと違う版のこともある)。
const char* version();

}  // namespace tokotachi

#endif
