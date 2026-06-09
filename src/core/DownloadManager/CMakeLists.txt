# DownloadManager — 文件传输管理器（下载/上传/Dump）— SHARED DLL
find_package(OpenSSL REQUIRED)

add_library(DownloadManager SHARED
    DownloadManager.h
    DownloadManager.cpp
)

set_target_properties(DownloadManager PROPERTIES
    DEBUG_POSTFIX "d"
    DEFINE_SYMBOL DOWNLOADMANAGER_LIBRARY
    LIBRARY_OUTPUT_DIRECTORY "${OUTPUT_BIN_DIR}/dll/core"
    RUNTIME_OUTPUT_DIRECTORY "${OUTPUT_BIN_DIR}/dll/core"
)

target_link_libraries(DownloadManager PRIVATE
    HttpClient
    EftpModels
    QtLogger
    ConfigManager
    SerApiModel
    CppHttplib
    Qt5::Core
    Qt5::Network
    Qt5::Concurrent
    OpenSSL::SSL
    OpenSSL::Crypto
)

target_compile_definitions(DownloadManager PRIVATE CPPHTTPLIB_OPENSSL_SUPPORT)
target_include_directories(DownloadManager PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
