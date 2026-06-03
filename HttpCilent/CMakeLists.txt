set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(OpenSSL REQUIRED)

add_library(HttpCilent SHARED
    httplib.h
    HttpClient.h
    HttpClient.cpp
)

set_target_properties(HttpCilent PROPERTIES
    DEBUG_POSTFIX "d"
    DEFINE_SYMBOL HTTPCLIENT_LIBRARY
)

target_link_libraries(HttpCilent PRIVATE
    QtLogger
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Gui
    Qt${QT_VERSION_MAJOR}::Widgets
    Qt${QT_VERSION_MAJOR}::Sql
    Qt${QT_VERSION_MAJOR}::Xml
    libssl.lib
    libcrypto.lib
)

target_compile_definitions(HttpCilent PRIVATE HTTPCLIENT_LIBRARY CPPHTTPLIB_OPENSSL_SUPPORT)
target_include_directories(HttpCilent PRIVATE ${OPENSSL_INCLUDE_DIR})
target_include_directories(HttpCilent PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
