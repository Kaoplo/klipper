cmake_minimum_required(VERSION 3.16)
project(klipper CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(PkgConfig REQUIRED)
pkg_check_modules(GLIB REQUIRED glib-2.0)

find_library(OBS_LIB obs REQUIRED)

add_executable(klipper
        src/main.cpp
        src/recording_engine/core/obs_context.cpp
        src/recording_engine/core/module_loader.cpp
        src/recording_engine/core/module_loader.cpp
        src/recording_engine/core/module_loader.h
        src/recording_engine/core/module_loader.cpp
        src/recording_engine/core/module_loader.cpp
        src/recording_engine/config/recording_config.h
        src/recording_engine/capture/capture_source.cpp
        src/recording_engine/capture/capture_source.h
        src/recording_engine/encoding/encoder_factory.cpp
        src/recording_engine/encoding/encoder_factory.h
        src/recording_engine/output/recording_output.cpp
        src/recording_engine/output/recording_output.h
        src/recording_engine/recording_engine.cpp
        src/recording_engine/recording_engine.h
        src/recording_engine/output/replay_buffer_output.cpp
        src/recording_engine/output/replay_buffer_output.h
)

# Lets every file use full-path includes like
# "recording_engine/config/recording_config.h" regardless of which
# subfolder it's compiled from.
target_include_directories(klipper PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
        ${GLIB_INCLUDE_DIRS}
)

target_link_libraries(klipper PRIVATE ${OBS_LIB} ${GLIB_LIBRARIES})