# QtLogger — 日志工具库（SHARED DLL）
# 依赖：仅 Qt5::Core（移除了不必要的 Sql/Xml/Gui/Widgets）
add_dll_library(QtLogger dll/core
    qtlogger.h
    qtlogger.cpp
)
target_compile_definitions(QtLogger PRIVATE QTLOGGER_LIBRARY)

target_link_libraries(QtLogger PRIVATE
    Qt5::Core
)

target_include_directories(QtLogger PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
