cmake_minimum_required(VERSION 3.16)
project(EftpClient VERSION 1.0.0 LANGUAGES CXX)

# ============================================================
# 全局设置
# ============================================================
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

if(MSVC)
    add_compile_options(/utf-8 /MP /W3 /EHsc /nologo)
    set(CMAKE_SKIP_PREPROCESSED_SOURCE_RULES ON)
    set(CMAKE_SKIP_ASSEMBLY_SOURCE_RULES ON)
endif()

add_definitions(-D_HAS_STD_BYTE=0 -D_UNICODE -D_WINDOWS -DQT_DEPRECATED_WARNINGS)

set(OUTPUT_BIN_DIR "${CMAKE_SOURCE_DIR}/Output/Win32/Bin")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${OUTPUT_BIN_DIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${OUTPUT_BIN_DIR})
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/out/lib")  # .lib .exp 中间文件

# ============================================================
# 工具函数：创建 SHARED DLL 并指定输出子目录
# ============================================================
# 用法：add_dll_library(DeviceModel dll/modules DeviceModel.h DeviceModel.cpp)
#   第1个参数：目标名
#   第2个参数：相对于 OUTPUT_BIN_DIR 的子目录（如 dll/core、dll/modules）
#   后续参数：源文件
function(add_dll_library name subdir)
    add_library(${name} SHARED ${ARGN})
    set_target_properties(${name} PROPERTIES
        DEBUG_POSTFIX "d"
        LIBRARY_OUTPUT_DIRECTORY "${OUTPUT_BIN_DIR}/${subdir}"
        RUNTIME_OUTPUT_DIRECTORY "${OUTPUT_BIN_DIR}/${subdir}"
    )
endfunction()

# ============================================================
# 查找 Qt5
# ============================================================
find_package(Qt5 REQUIRED COMPONENTS Core Network Gui Widgets Concurrent Sql Xml)

set(QT_VERSION_MAJOR 5)

# ============================================================
# 第三方库
# ============================================================
add_subdirectory(third_party/jsonhpp)
add_subdirectory(third_party/cpp-httplib)

# ============================================================
# 基础设施层
# ============================================================
add_subdirectory(src/core/QtLogger)
add_subdirectory(src/core/HttpClient)
add_subdirectory(src/core/ConfigManager)
add_subdirectory(src/core/DownloadManager)

# ============================================================
# EftpClient（数据模型 + 业务模块）
# ============================================================
add_subdirectory(src/EftpClient)

# ============================================================
# 应用层（含所有 UI 组件）
# ============================================================
add_subdirectory(src/app)
add_subdirectory(src/launcher)
