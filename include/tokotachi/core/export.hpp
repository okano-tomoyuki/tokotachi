/// @file
/// 公開するシンボルの印 TOKOTACHI_API(docs/adr/0057)。
///
/// 公開のクラス・関数にだけ付け、それ以外(実装のクラスなど)は共有ライブラリの外に見せない。
/// - Linux など: 既定の可視性(ライブラリは -fvisibility=hidden でビルドする)。
/// - Windows(MinGW): 共有ライブラリをビルドするときだけ dllexport。利用する側では空にし、MinGW の自動の読み込み(auto-import)に任せる
///   (共有か静的かを利用する側で知らせずに済む)。
#ifndef TOKOTACHI_CORE_EXPORT_HPP
#define TOKOTACHI_CORE_EXPORT_HPP

#if defined(_WIN32)
#if defined(TOKOTACHI_BUILDING_SHARED)
#define TOKOTACHI_API __declspec(dllexport)
#else
#define TOKOTACHI_API
#endif
#else
#define TOKOTACHI_API __attribute__((visibility("default")))
#endif

#endif
