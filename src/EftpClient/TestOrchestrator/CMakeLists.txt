# TestOrchestrator — 测试流程状态机（SHARED DLL）
# SHARED 理由：测试流程可能因产品线不同而需要替换
# TODO: 待实现
add_dll_library(TestOrchestrator dll/modules
    TestOrchestrator.h
    TestOrchestrator.cpp
)
target_compile_definitions(TestOrchestrator PRIVATE TESTORCHESTRATOR_LIBRARY)

target_link_libraries(TestOrchestrator PRIVATE
    SerApiModel
    ConfigManager
    EftpModels
    EftpJsonParser
    QtLogger
    Qt5::Core
    Qt5::Concurrent
)

target_include_directories(TestOrchestrator PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
