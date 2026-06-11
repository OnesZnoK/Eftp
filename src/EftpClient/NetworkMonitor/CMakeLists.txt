# NetworkMonitor — 网络监控模块（SHARED DLL）
# 定时 ping 检测 + WiFi 自动切换
add_dll_library(NetworkMonitor dll/modules
    NetworkMonitor.h
    NetworkMonitor.cpp
)
target_compile_definitions(NetworkMonitor PRIVATE NETWORKMONITOR_LIBRARY)

target_link_libraries(NetworkMonitor PRIVATE
    ConfigManager
    QtLogger
    Qt5::Core
    Qt5::Concurrent
    Qt5::Network
)

target_include_directories(NetworkMonitor PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
