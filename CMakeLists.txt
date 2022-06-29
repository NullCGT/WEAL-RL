cmake_minimum_required(VERSION 3.23.1)
set(CMAKE_C_STANDARD 99)
set(PROJECT_NAME weal_ncurses)

# Set up version.h
set(VERSION_MAJOR 1)
set(VERSION_MINOR 0)
SET(VERSION_PATCH 0)
SET(RELEASE_TYPE "alpha")
configure_file (
    "${CMAKE_SOURCE_DIR}/include/version.h.in"
    "${CMAKE_SOURCE_DIR}/include/version.h"
)

# Set up our projects
project(${PROJECT_NAME} VERSION 1.0)       # Runs in the console with ncurses.

# Find required packages
find_package(Curses REQUIRED)
find_package(cJSON REQUIRED)

# I know globbing c files is bad practice, but when we're this early in development, it's just plain easier.
file(GLOB SRCS "src/*.c")

# Add executables
add_executable(${PROJECT_NAME} ${SRCS} windows/curses/windows.c windows/curses/menu.c)

# Bad practice.
target_include_directories(${PROJECT_NAME} PUBLIC include ${CJSON_INCLUDE_DIR} ${CURSES_INCLUDE_DIRS})

# Link ncurses libraries
target_link_libraries(${PROJECT_NAME} PUBLIC ${CJSON_LIBRARIES} ${CURSES_LIBRARIES})

# Set target properties
set_target_properties(${PROJECT_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}")

# Copy data files
# See https://stackoverflow.com/questions/13429656/how-to-copy-contents-of-a-directory-into-build-directory-after-make-with-cmake
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
                   COMMAND ${CMAKE_COMMAND} -E copy_directory
                       ${CMAKE_SOURCE_DIR}/data $<TARGET_FILE_DIR:${PROJECT_NAME}>/data
                    COMMENT "Copying data folder to output dir.")

message(STATUS "Configuring CMake for ${PROJECT_NAME} v${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}-${RELEASE_TYPE}")

install(TARGETS ${PROJECT_NAME} DESTINATION bin)
#CPack
include(InstallRequiredSystemLibraries)
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_SOURCE_DIR}/LICENSE")
set(CPACK_PACKAGE_VERSION_MAJOR "${VERSION_MAJOR}")
set(CPACK_PACKAGE_VERSION_MINOR "${VERSION_MINOR}")
include(CPack)