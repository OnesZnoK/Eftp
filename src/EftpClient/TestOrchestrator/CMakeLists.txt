# TestOrchestrator — 测试流程状态机（SHARED DLL）
# SHARED 理由：测试流程可能因产品线不同而需要替换
# TODO: 待实现
add_library(TestOrchestrator SHARED
    TestOrchestrator.h
    TestOrchestrator.cpp
)

set_target_properties(TestOrchestrator PROPERTIES
    DEBUG_POSTFIX "d"
    DEFINE_SYMBOL TESTORCHESTRATOR_LIBRARY
    LIBRARY_OUTPUT_DIRECTORY "${OUTPUT_BIN_DIR}/dll/modules"
    RUNTIME_OUTPUT_DIRECTORY "${OUTPUT_BIN_DIR}/dll/modules"
)

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
