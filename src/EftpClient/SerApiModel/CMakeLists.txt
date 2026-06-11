# SerApiModel — EFTP 服务端 REST API 封装模块（SHARED DLL）
# 统一管理所有 19 个 EFTP 服务端接口调用
# 依赖：ConfigManager（URL配置）、HttpClient（HTTP请求）、QtLogger（日志）、EftpModels（数据结构）
add_dll_library(SerApiModel dll/modules
    SerApiModel.h
    SerApiModel.cpp
)
target_compile_definitions(SerApiModel PRIVATE SERAPIMODEL_LIBRARY)

target_link_libraries(SerApiModel PRIVATE
    ConfigManager
    HttpClient
    QtLogger
    EftpModels
    JsonHpp
    Qt5::Core
)

target_include_directories(SerApiModel PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/src/core/HttpClient
    ${CMAKE_SOURCE_DIR}/third_party/cpp-httplib
)
