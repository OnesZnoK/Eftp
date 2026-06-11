# SystemTrayManager — 系统托盘管理器（STATIC）
add_library(SystemTrayManager STATIC
    SystemTrayManager.h
    SystemTrayManager.cpp
    actions/ITrayAction.h
    actions/SettingsAction.h
    actions/BindDeviceAction.h
)

set_target_properties(SystemTrayManager PROPERTIES
    DEBUG_POSTFIX "d"
    MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>"
)

target_link_libraries(SystemTrayManager PRIVATE
    Qt5::Core
    Qt5::Widgets
)

target_include_directories(SystemTrayManager PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/actions
)
