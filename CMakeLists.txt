cmake_minimum_required(VERSION 3.14)
project(sese.event VERSION 0.1.0)

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
    target_compile_options(Event PRIVATE /utf-8)
elseif (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    target_sources(
            Event
            PRIVATE
            src/EpollEvent.cpp
    )
elseif (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
else ()
endif ()

option(SESE_EVENT_BUILD_TEST "build unit test targets")

if (SESE_EVENT_BUILD_TEST)
    enable_testing()
    add_subdirectory(test)
endif ()