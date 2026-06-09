# DeviceModel — 设备硬件信息采集模块（SHARED DLL）
# 通过 PowerShell WMI 采集 BIOS SN、主板 SN、MAC 地址
# 依赖：QtLogger（日志）、EftpModels（DeviceInfo 结构体）
add_library(DeviceModel SHARED
    DeviceModel.h
    DeviceModel.cpp
)

set_target_properties(DeviceModel PROPERTIES
    DEBUG_POSTFIX "d"
    DEFINE_SYMBOL DEVICEMODEL_LIBRARY
    LIBRARY_OUTPUT_DIRECTORY "${OUTPUT_BIN_DIR}/dll/modules"
    RUNTIME_OUTPUT_DIRECTORY "${OUTPUT_BIN_DIR}/dll/modules"
)

target_link_libraries(DeviceModel PRIVATE
    QtLogger
    EftpModels
    Qt5::Core
)

target_include_directories(DeviceModel PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
