cmake_minimum_required(VERSION 3.6.3)
project(GStreamerTask2)

set(CMAKE_CXX_STANDARD 20)

include(FindPkgConfig)

find_package(PkgConfig REQUIRED)
pkg_search_module(gstreamer REQUIRED IMPORTED_TARGET gstreamer-1.0>=1.4)
pkg_search_module(gstreamer-sdp REQUIRED IMPORTED_TARGET gstreamer-sdp-1.0>=1.4)
pkg_search_module(gstreamer-app REQUIRED IMPORTED_TARGET gstreamer-app-1.0>=1.4)
pkg_search_module(gstreamer-video REQUIRED IMPORTED_TARGET gstreamer-video-1.0>=1.4)

add_executable(gsttogglesources src/main.cpp)

set_target_properties(gsttogglesources PROPERTIES PREFIX "")

target_link_libraries(
    gsttogglesources
    PkgConfig::gstreamer
    PkgConfig::gstreamer-sdp
    PkgConfig::gstreamer-app
    PkgConfig::gstreamer-video
)

add_custom_command(
    TARGET gsttogglesources POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_SOURCE_DIR}/resources/
    ${CMAKE_CURRENT_BINARY_DIR}/
)

