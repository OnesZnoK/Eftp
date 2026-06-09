# QtLogger — 日志工具库（SHARED DLL）
# 依赖：仅 Qt5::Core（移除了不必要的 Sql/Xml/Gui/Widgets）
add_library(QtLogger SHARED
    qtlogger.h
    qtlogger.cpp
)

set_target_properties(QtLogger PROPERTIES
    DEBUG_POSTFIX "d"
    DEFINE_SYMBOL QTLOGGER_LIBRARY
    LIBRARY_OUTPUT_DIRECTORY "${OUTPUT_BIN_DIR}/dll/core"
    RUNTIME_OUTPUT_DIRECTORY "${OUTPUT_BIN_DIR}/dll/core"
)

target_link_libraries(QtLogger PRIVATE
    Qt5::Core
)

target_include_directories(QtLogger PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
