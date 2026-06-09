# cpp-httplib — header-only HTTP library
# 使用时需要 #define CPPHTTPLIB_OPENSSL_SUPPORT 启用 HTTPS
add_library(CppHttplib INTERFACE)
target_include_directories(CppHttplib INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})
