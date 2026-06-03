set(CMAKE_AUTOMOC ON)

add_library(DeviceModel SHARED
    DeviceModel.h
    DeviceModel.cpp
)

set_target_properties(DeviceModel PROPERTIES
    DEBUG_POSTFIX "d"
    DEFINE_SYMBOL DEVICEMODEL_LIBRARY
)

target_link_libraries(DeviceModel PRIVATE
    QtLogger
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Network
    Qt${QT_VERSION_MAJOR}::Gui
    Qt${QT_VERSION_MAJOR}::Widgets
    Qt${QT_VERSION_MAJOR}::Concurrent
)

target_compile_definitions(DeviceModel PRIVATE DEVICEMODEL_LIBRARY)
target_include_directories(DeviceModel PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
