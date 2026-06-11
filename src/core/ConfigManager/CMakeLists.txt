# ConfigManager — 配置管理单例（SHARED DLL）
# 依赖：JsonHpp（JSON解析）
add_dll_library(ConfigManager dll/core
    ConfigManager.h
    ConfigManager.cpp
)
target_compile_definitions(ConfigManager PRIVATE CONFIGMANAGER_LIBRARY)

target_link_libraries(ConfigManager PUBLIC
    JsonHpp
    Qt5::Core
)

target_include_directories(ConfigManager PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

# 构建后复制配置文件到 exe 目录（而非 dll 目录）
add_custom_command(TARGET ConfigManager POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    "${CMAKE_SOURCE_DIR}/configs"
    "${OUTPUT_BIN_DIR}/configs"
    COMMENT "Copying config files to output..."
)
