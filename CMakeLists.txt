cmake_minimum_required(VERSION 3.20)

project(klipper LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(PkgConfig REQUIRED)
pkg_check_modules(LIBOBS REQUIRED libobs)

find_package(PkgConfig REQUIRED)
pkg_check_modules(GLIB REQUIRED glib-2.0)


add_executable(klipper src/main.cpp)

target_include_directories(klipper PRIVATE
    ${LIBOBS_INCLUDE_DIRS}
)

target_link_libraries(klipper PRIVATE
    ${LIBOBS_LIBRARIES}
)
target_include_directories(klipper PRIVATE ${GLIB_INCLUDE_DIRS})
target_link_libraries(klipper PRIVATE ${GLIB_LIBRARIES} obs)

set(LIBOBS_CFLAGS_OTHER_CXX ${LIBOBS_CFLAGS_OTHER})
list(FILTER LIBOBS_CFLAGS_OTHER_CXX EXCLUDE REGEX "^-std=.*")

target_compile_options(klipper PRIVATE ${LIBOBS_CFLAGS_OTHER_CXX})