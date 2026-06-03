set(CMAKE_AUTOMOC ON)

add_library(ConfigManager SHARED
    ConfigManager.h
    ConfigManager.cpp
)

set_target_properties(ConfigManager PROPERTIES
    DEBUG_POSTFIX "d"
    DEFINE_SYMBOL CONFIGMANAGER_LIBRARY
)

target_link_libraries(ConfigManager PRIVATE
    JsonHpp
)

target_compile_definitions(ConfigManager PRIVATE CONFIGMANAGER_LIBRARY)
target_include_directories(ConfigManager PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/JsonHpp
)

# 构建后复制配置文件到输出目录
add_custom_command(TARGET ConfigManager POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    "${CMAKE_CURRENT_SOURCE_DIR}/configs"
    "$<TARGET_FILE_DIR:ConfigManager>/configs"
    COMMENT "Copying config files to output..."
)
