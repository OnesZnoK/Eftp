# EftpLauncher — 加载器（无依赖，纯 Win32 API）
# 读取 dll.ini 配置，设置 DLL 搜索路径，启动 EFTP.exe
add_executable(EftpLauncher WIN32
    main.cpp
    Launcher.rc
)

set_target_properties(EftpLauncher PROPERTIES
    OUTPUT_NAME "EftpLauncher"
    RUNTIME_OUTPUT_DIRECTORY "${OUTPUT_BIN_DIR}"
    # 禁用 CMake 自动生成的 manifest，使用 RC 中嵌入的 UAC manifest
    LINK_FLAGS "/MANIFEST:NO"
)

# 链接 Windows 库
target_link_libraries(EftpLauncher PRIVATE
    kernel32
    user32
)
