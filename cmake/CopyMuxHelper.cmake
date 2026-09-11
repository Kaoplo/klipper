set(destination "${DESTINATION_DIR}/${HELPER_NAME}")
file(MAKE_DIRECTORY "${DESTINATION_DIR}")

# Replace the old manual symlink without writing through it into system files.
if(IS_SYMLINK "${destination}")
    file(REMOVE "${destination}")
endif()

execute_process(
    COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${SOURCE_FILE}" "${destination}"
    RESULT_VARIABLE result
)
if(NOT result EQUAL 0)
    message(FATAL_ERROR "Failed to bundle OBS mux helper from ${SOURCE_FILE}")
endif()
