# 静的ライブラリをまとめる(docs/adr/0037)。tokotachi を静的ライブラリとして作るとき、依存ライブラリ(SDL3 など)の
# 静的ライブラリを libtokotachi.a に入れ、利用者が tokotachi だけをリンクすればよいようにする。
#
#   cmake -DAR=<ar> -DSCRIPT=<MRI スクリプト> -P merge_static.cmake
#
# SCRIPT は GNU ar の MRI スクリプト(CREATE・ADDLIB・SAVE・END)。
execute_process(COMMAND "${AR}" -M INPUT_FILE "${SCRIPT}" RESULT_VARIABLE result)
if(NOT result EQUAL 0)
    message(FATAL_ERROR "merging static libraries failed: ${result}")
endif()
