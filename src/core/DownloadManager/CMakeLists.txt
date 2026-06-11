# DownloadManager — 文件传输管理器（下载/上传/Dump）— SHARED DLL
find_package(OpenSSL REQUIRED)

add_dll_library(DownloadManager dll/core
    DownloadManager.h
    DownloadManager.cpp
)
target_compile_definitions(DownloadManager PRIVATE DOWNLOADMANAGER_LIBRARY)

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
