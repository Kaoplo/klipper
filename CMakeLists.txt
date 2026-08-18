cmake_minimum_required(VERSION 3.16)
project(klipper CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)

if(NOT WIN32)
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(GLIB REQUIRED glib-2.0)
endif()

find_package(Qt6 QUIET COMPONENTS Widgets)


find_library(OBS_LIB obs REQUIRED)

add_library(recording_engine STATIC
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
        src/recording_engine/capture/audio_capture_source.cpp
        src/recording_engine/capture/audio_capture_source.h
)

target_include_directories(recording_engine PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_link_libraries(recording_engine PUBLIC ${OBS_LIB})

if(NOT WIN32)
    target_include_directories(recording_engine PUBLIC ${GLIB_INCLUDE_DIRS})
    target_link_libraries(recording_engine PUBLIC ${GLIB_LIBRARIES})
endif()

add_executable(klipper-gui
        src/frontend/main.cpp
        src/frontend/main_window.cpp
        src/frontend/main_window.h
        src/frontend/engine_worker.cpp
        src/frontend/engine_worker.h
        src/frontend/settings_window.cpp
        src/frontend/settings_window.h
)
target_link_libraries(klipper-gui PRIVATE recording_engine)

target_link_libraries(klipper-gui PRIVATE Qt6::Widgets)
