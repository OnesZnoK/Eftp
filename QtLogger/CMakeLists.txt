set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON) 

add_library(QtLogger SHARED
    qtlogger.h
    qtlogger.cpp
)

set_target_properties(QtLogger PROPERTIES
    DEBUG_POSTFIX "d"          
    DEFINE_SYMBOL QTLOGGER_LIBRARY 
)

target_link_libraries(QtLogger PRIVATE
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Gui
    Qt${QT_VERSION_MAJOR}::Widgets
    Qt${QT_VERSION_MAJOR}::Sql
    Qt${QT_VERSION_MAJOR}::Xml
)

target_compile_definitions(QtLogger PRIVATE QTLOGGER_LIBRARY)

target_include_directories(QtLogger PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})