cmake_minimum_required(VERSION 3.14)
project(sese.event VERSION 0.1.1)

add_library(Event)

set_target_properties(
        Event
        PROPERTIES
        CXX_STANDARD 11
)

target_include_directories(
        Event
        PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

if (WIN32)
    set_target_properties(
            Event
            PROPERTIES
            WINDOWS_EXPORT_ALL_SYMBOLS ON
            OUTPUT_NAME "sese.event"
    )
    target_compile_options(Event PRIVATE /utf-8)
    target_sources(
            Event
            PRIVATE
            src/WSAEventLoop.cpp
            src/WSAEventConvert.cpp
    )
    target_link_libraries(Event PRIVATE ws2_32)
elseif (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    set_target_properties(
            Event
            PROPERTIES
            OUTPUT_NAME "sese.event"
            PREFIX ""
    )
    target_sources(
            Event
            PRIVATE
            src/EpollEventLoop.cpp
            src/EpollEventConvert.cpp
    )
elseif (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    set_target_properties(
            Event
            PROPERTIES
            OUTPUT_NAME "sese.event"
            PREFIX ""
    )
    target_sources(
            Event
            PRIVATE
            src/KqueueEventLoop.cpp
    )
else ()
    message(FATAL_ERROR "Unknown system type")
endif ()

option(SESE_EVENT_BUILD_TEST "build unit test targets")

if (SESE_EVENT_BUILD_TEST)
    enable_testing()
    add_subdirectory(test)
endif ()

include(GNUInstallDirs)
include(CMakePackageConfigHelpers)
configure_package_config_file(
        ${PROJECT_SOURCE_DIR}/cmake/sese-event-config.cmake.in
        ${PROJECT_BINARY_DIR}/sese-event-config.cmake
        INSTALL_DESTINATION lib/cmake/sese-event
)

install(
        TARGETS Event
        EXPORT SeseEventTargets
        RUNTIME DESTINATION bin
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib
        PUBLIC_HEADER DESTINATION include
)

install(
        DIRECTORY "${CMAKE_CURRENT_LIST_DIR}/include/sese"
        DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}"
)

install(
        FILES "${PROJECT_BINARY_DIR}/sese-event-config.cmake"
        DESTINATION lib/cmake/sese-event
)

install(
        FILES "${PROJECT_BINARY_DIR}/sese-event-config.cmake"
        DESTINATION debug/lib/cmake/sese-event
)

install(
        EXPORT SeseEventTargets
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/sese-event
        NAMESPACE Sese::
)