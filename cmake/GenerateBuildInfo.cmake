set(GIT_SHA "unknown")
set(GIT_SHORT_SHA "unknown")
set(GIT_SUFFIX "")
set(GIT_STATE "unknown")
set(GIT_ERROR "Git is unavailable or the source directory has no .git metadata.")

find_package(Git QUIET)
# Do not accidentally pick up a parent repository for a source archive.
if(GIT_FOUND AND EXISTS "${SOURCE_DIR}/.git")
    execute_process(
        COMMAND "${GIT_EXECUTABLE}" rev-parse --verify HEAD
        WORKING_DIRECTORY "${SOURCE_DIR}"
        OUTPUT_VARIABLE HEAD_SHA
        OUTPUT_STRIP_TRAILING_WHITESPACE
        RESULT_VARIABLE HEAD_RESULT
        ERROR_VARIABLE HEAD_ERROR
    )
    if(HEAD_RESULT EQUAL 0 AND HEAD_SHA MATCHES "^[0-9a-f]+$")
        set(GIT_SHA "${HEAD_SHA}")
        string(SUBSTRING "${GIT_SHA}" 0 8 GIT_SHORT_SHA)
        execute_process(
            COMMAND "${GIT_EXECUTABLE}" status --porcelain --untracked-files=normal
            WORKING_DIRECTORY "${SOURCE_DIR}"
            OUTPUT_VARIABLE GIT_STATUS
            OUTPUT_STRIP_TRAILING_WHITESPACE
            RESULT_VARIABLE STATUS_RESULT
            ERROR_VARIABLE STATUS_ERROR
        )
        if(STATUS_RESULT EQUAL 0)
            if(GIT_STATUS STREQUAL "")
                set(GIT_STATE "clean")
            else()
                set(GIT_STATE "dirty")
                set(GIT_SUFFIX "-dirty")
            endif()
        else()
            set(GIT_ERROR "git status failed (${STATUS_RESULT}): ${STATUS_ERROR}")
        endif()
    else()
        set(GIT_ERROR "git rev-parse HEAD failed (${HEAD_RESULT}): ${HEAD_ERROR}")
    endif()
endif()

if(GIT_SHA STREQUAL "unknown" OR GIT_STATE STREQUAL "unknown")
    if(REQUIRE_GIT_METADATA)
        message(FATAL_ERROR "Cannot determine build Git metadata. ${GIT_ERROR}")
    else()
        message(WARNING "Cannot determine build Git metadata. ${GIT_ERROR}")
    endif()
endif()

configure_file("${CMAKE_CURRENT_LIST_DIR}/build_info.h.in" "${OUTPUT_FILE}" @ONLY)
