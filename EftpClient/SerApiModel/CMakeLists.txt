set(CMAKE_AUTOMOC ON)

add_library(SerApiModel SHARED
    SerApiModel.h
    SerApiModel.cpp
)

set_target_properties(SerApiModel PROPERTIES
    DEBUG_POSTFIX "d"
    DEFINE_SYMBOL SERAPIMODEL_LIBRARY
)

target_link_libraries(SerApiModel PRIVATE
    ConfigManager
    HttpCilent
    QtLogger
    JsonHpp
    Qt${QT_VERSION_MAJOR}::Core
)

target_compile_definitions(SerApiModel PRIVATE SERAPIMODEL_LIBRARY)
target_include_directories(SerApiModel PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
