find_package(Doxygen REQUIRED)

# Find all the public headers
file(GLOB_RECURSE YADSL_HEADERS ${YADSL_SOURCE_DIR}/*.h)
file(GLOB_RECURSE DOC_FILES ${CMAKE_CURRENT_SOURCE_DIR}/*.md)

set(DOXYGEN_OUTPUT_DIR ${PROJECT_SOURCE_DIR}/html)
set(DOXYGEN_INDEX_FILE ${DOXYGEN_OUTPUT_DIR}/index.html)
set(DOXYFILE ${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile)

# Only regenerate Doxygen when the Doxyfile or public headers change
add_custom_command(OUTPUT ${DOXYGEN_INDEX_FILE}
                   DEPENDS ${YADSL_HEADERS} ${DOC_FILES}
                   COMMAND ${DOXYGEN_EXECUTABLE} ${DOXYFILE}
                   WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
                   MAIN_DEPENDENCY ${DOXYFILE}
                   COMMENT "Generating documentation with Doxygen"
                   VERBATIM)

add_custom_target(Doxygen ALL DEPENDS ${DOXYGEN_INDEX_FILE})

set_target_properties(Doxygen PROPERTIES FOLDER docs)