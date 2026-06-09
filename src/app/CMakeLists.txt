# EFTP — 主应用程序（WIN32 可执行文件）
set(SOURCES
    main.cpp
    MainWindow.cpp
    DeviceInfoPanel.cpp
    SettingsDialog.cpp
    # 控件
    EDYTabBar.cpp
    # Model/Delegate
    ItemProgramTestModel.cpp
    ItemProgramTestDelegate.cpp
    ItemDetailInfoWidgetModel.cpp
    ItemDetailInfoWidgetDelegate.cpp
    # 对话框
    ItemDetailInfoDialog.cpp
    ErrorDialog.cpp
    SNMacBindDialog.cpp
    # 页面
    InitialPage.cpp
    StageTestPage.cpp
)

set(HEADERS
    MainWindow.h
    DeviceInfoPanel.h
    SettingsDialog.h
    EDYTabBar.h
    EftpTabWidget.h
    ItemProgramTestModel.h
    ItemProgramTestDelegate.h
    ItemDetailInfoWidgetModel.h
    ItemDetailInfoWidgetDelegate.h
    ItemDetailInfoDialog.h
    ErrorDialog.h
    SNMacBindDialog.h
    InitialPage.h
    StageTestPage.h
)

set(UI_FILES
    MainWindow.ui
    DeviceInfoPanel.ui
    ItemDetailInfoDialog.ui
    SNMacBindDialog.ui
    ErrorDialog.ui
    InitialPage.ui
    StageTestPage.ui
    SettingsDialog.ui
)

set(QRC_FILES
    resource/Eftp.qrc
)

set(WIN_RC_FILES
    Eftp.rc
)

add_executable(${PROJECT_NAME} WIN32
    ${SOURCES}
    ${HEADERS}
    ${UI_FILES}
    ${QRC_FILES}
    ${WIN_RC_FILES}
)

set_target_properties(${PROJECT_NAME} PROPERTIES
    DEBUG_POSTFIX "d"
    OUTPUT_NAME "EFTP"
)

if(MSVC)
    set_property(TARGET ${PROJECT_NAME} PROPERTY
        MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
endif()

target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/src/EftpClient
    ${CMAKE_SOURCE_DIR}/src/core/QtLogger
    ${CMAKE_SOURCE_DIR}/src/core/HttpClient
    ${CMAKE_SOURCE_DIR}/src/core/ConfigManager
    ${CMAKE_SOURCE_DIR}/src/core/DownloadManager
    ${CMAKE_SOURCE_DIR}/src/EftpClient/SerApiModel
    ${CMAKE_SOURCE_DIR}/src/EftpClient/DeviceModel
    ${CMAKE_SOURCE_DIR}/src/EftpClient/TestOrchestrator
    ${CMAKE_SOURCE_DIR}/src/EftpClient/NetworkMonitor
    ${CMAKE_SOURCE_DIR}/src/EftpClient/EftpJsonParser
    ${CMAKE_SOURCE_DIR}/src/EftpClient/VersionChecker
    ${CMAKE_SOURCE_DIR}/src/EftpClient/SystemTrayManager/actions
)

target_link_libraries(${PROJECT_NAME} PRIVATE
    # 基础设施层
    QtLogger
    HttpClient
    ConfigManager
    DownloadManager
    # 业务模块
    DeviceModel
    SerApiModel
    NetworkMonitor
    TestOrchestrator
    SystemTrayManager
    EftpJsonParser
    EftpModels
    # 第三方
    JsonHpp
    # Qt
    Qt5::Core
    Qt5::Network
    Qt5::Gui
    Qt5::Widgets
    Qt5::Concurrent
)

# Auto deploy Qt DLLs
find_program(WINDEPLOYQT_EXECUTABLE windeployqt HINTS "${Qt5_DIR}/../../../bin")
if(WINDEPLOYQT_EXECUTABLE)
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND "${WINDEPLOYQT_EXECUTABLE}" --release --no-translations --no-opengl-sw --no-compiler-runtime
                "$<TARGET_FILE:${PROJECT_NAME}>"
        COMMENT "Deploying Qt runtime..."
    )
endif()

# Auto deploy OpenSSL DLLs
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_SOURCE_DIR}/third_party/openssl/dll/libcrypto-3.dll"
        "$<TARGET_FILE_DIR:${PROJECT_NAME}>"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_SOURCE_DIR}/third_party/openssl/dll/libssl-3.dll"
        "$<TARGET_FILE_DIR:${PROJECT_NAME}>"
    COMMENT "Deploying OpenSSL DLLs..."
)
