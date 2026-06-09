# HttpClient — HTTP 客户端库（SHARED DLL）
# 依赖：QtLogger（日志）、cpp-httplib（HTTP）、OpenSSL（HTTPS）

add_library(HttpClient SHARED
    HttpClient.h
    HttpClient.cpp
)

set_target_properties(HttpClient PROPERTIES
    DEBUG_POSTFIX "d"
    DEFINE_SYMBOL HTTPCLIENT_LIBRARY
    LIBRARY_OUTPUT_DIRECTORY "${OUTPUT_BIN_DIR}/dll/core"
    RUNTIME_OUTPUT_DIRECTORY "${OUTPUT_BIN_DIR}/dll/core"
)

# OpenSSL — 直接使用 vendored 路径
set(OPENSSL_DIR ${CMAKE_SOURCE_DIR}/third_party/openssl)

target_link_libraries(HttpClient PRIVATE
    QtLogger
    CppHttplib
    Qt5::Core
    ${OPENSSL_DIR}/lib/MD-Release-x86/libssl.lib
    ${OPENSSL_DIR}/lib/MD-Release-x86/libcrypto.lib
)

target_compile_definitions(HttpClient PUBLIC CPPHTTPLIB_OPENSSL_SUPPORT)
target_include_directories(HttpClient PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/third_party/cpp-httplib
    ${OPENSSL_DIR}/include
)
