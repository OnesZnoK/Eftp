# NetworkMonitor — 网络监控模块（SHARED DLL）
# 定时 ping 检测 + WiFi 自动切换
add_library(NetworkMonitor SHARED
    NetworkMonitor.h
    NetworkMonitor.cpp
)

set_target_properties(NetworkMonitor PROPERTIES
    DEBUG_POSTFIX "d"
    DEFINE_SYMBOL NETWORKMONITOR_LIBRARY
    LIBRARY_OUTPUT_DIRECTORY "${OUTPUT_BIN_DIR}/dll/modules"
    RUNTIME_OUTPUT_DIRECTORY "${OUTPUT_BIN_DIR}/dll/modules"
)

target_link_libraries(NetworkMonitor PRIVATE
    ConfigManager
    QtLogger
    Qt5::Core
    Qt5::Concurrent
    Qt5::Network
)

target_include_directories(NetworkMonitor PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
