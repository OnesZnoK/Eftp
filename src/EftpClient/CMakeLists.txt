# EftpClient 业务模块集合
# EftpTypes.h — 纯头文件数据结构（INTERFACE）
add_library(EftpModels INTERFACE)
target_include_directories(EftpModels INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(EftpModels INTERFACE JsonHpp)

# 子模块
add_subdirectory(EftpJsonParser)
add_subdirectory(SerApiModel)
add_subdirectory(DeviceModel)
add_subdirectory(NetworkMonitor)
add_subdirectory(TestOrchestrator)
add_subdirectory(SystemTrayManager)
