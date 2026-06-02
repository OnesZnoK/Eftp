set(SOURCES
    main.cpp
    MainWindow/MainWindow.cpp
)

set(HEADERS
    MainWindow/MainWindow.h
    resource.h
)

set(UI_FILES
    MainWindow/MainWindow.ui
)

set(QRC_FILES
    resource/ComputerTest.qrc
)

set(WIN_RC_FILES
    ComputerTest.rc
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
    ${CMAKE_CURRENT_SOURCE_DIR}/MainWindow
)

target_link_libraries(${PROJECT_NAME} PRIVATE
    QtLogger
    Qt5::Core
    Qt5::Network
    Qt5::Gui
    Qt5::Widgets
    Qt5::Concurrent
)

# Auto deploy Qt DLLs after build
find_program(WINDEPLOYQT_EXECUTABLE windeployqt HINTS "${Qt5_DIR}/../../../bin")
if(WINDEPLOYQT_EXECUTABLE)
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND "${WINDEPLOYQT_EXECUTABLE}" --release --no-translations --no-opengl-sw --no-compiler-runtime
                "$<TARGET_FILE:${PROJECT_NAME}>"
        COMMENT "Deploying Qt runtime..."
    )
endif()
