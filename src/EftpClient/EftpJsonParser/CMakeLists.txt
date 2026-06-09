# EftpJsonParser — JSON 解析工具（STATIC）
add_library(EftpJsonParser STATIC
    EftpJsonParser.h
    EftpJsonParser.cpp
)

set_target_properties(EftpJsonParser PROPERTIES
    DEBUG_POSTFIX "d"
)

target_link_libraries(EftpJsonParser PRIVATE
    JsonHpp
    EftpModels
    Qt5::Core
)

target_include_directories(EftpJsonParser PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
