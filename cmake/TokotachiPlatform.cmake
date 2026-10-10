# 依存ライブラリのインストール先を分けるためのプラットフォーム名(windows-x86_64、linux-x86_64、linux-aarch64 など)。
# deps/CMakeLists.txt と本体の CMakeLists.txt の両方で使う。
string(TOLOWER "${CMAKE_SYSTEM_NAME}" _gui_os)
string(TOLOWER "${CMAKE_SYSTEM_PROCESSOR}" _gui_arch)
if(_gui_arch STREQUAL "amd64")
    set(_gui_arch x86_64)
elseif(_gui_arch STREQUAL "arm64")
    set(_gui_arch aarch64)
endif()
set(TOKOTACHI_PLATFORM "${_gui_os}-${_gui_arch}")
