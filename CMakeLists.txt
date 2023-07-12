cmake_minimum_required(VERSION 3.14)

option(SESE_EVENT_BUILD_TEST "build unit test targets")
if (SESE_EVENT_BUILD_TEST)
    if (NOT DEFINED VCPKG_MANIFEST_FEATURES)
        message(STATUS "Auto append features: tests")
        set(VCPKG_MANIFEST_FEATURES "tests")
    else ()
        list(FIND VCPKG_MANIFEST_FEATURES "tests" index)
        if (index EQUAL -1)
            message(STATUS "Auto append features: tests")
            list(APPEND VCPKG_MANIFEST_FEATURES "tests")
        endif ()
    endif ()
endif ()

project(sese-event VERSION 0.1.2)

add_library(Event SHARED)

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

add_library(Event-static STATIC)

set_target_properties(
        Event-static
        PROPERTIES
        CXX_STANDARD 11
)

target_include_directories(
        Event-static
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

target_sources(Event-static PRIVATE $<TARGET_OBJECTS:Event>)

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
        TARGETS Event Event-static
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