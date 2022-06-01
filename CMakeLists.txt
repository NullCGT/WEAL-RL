cmake_minimum_required(VERSION 3.23.1)
set(CMAKE_C_STANDARD 99)

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
project(weal_ncurses VERSION 1.0)       # Runs in the console with ncurses.

# Find required packages
find_package(Curses REQUIRED)
find_package(cJSON REQUIRED)

# I know globbing c files is bad practice, but when we're this early in development, it's just plain easier.
file(GLOB SRCS "src/*.c")

# Add executables
add_executable(weal_ncurses ${SRCS} windows/curses/windows.c windows/curses/menu.c)

# Bad practice.
target_include_directories(weal_ncurses PUBLIC include ${CJSON_INCLUDE_DIR} ${CURSES_INCLUDE_DIRS})

# Link ncurses libraries
target_link_libraries(weal_ncurses PUBLIC ${CJSON_LIBRARIES} ${CURSES_LIBRARIES})

# Set target properties
set_target_properties(weal_ncurses PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")

# Copy data files
# See https://stackoverflow.com/questions/13429656/how-to-copy-contents-of-a-directory-into-build-directory-after-make-with-cmake
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
                   COMMAND ${CMAKE_COMMAND} -E copy_directory
                       ${CMAKE_SOURCE_DIR}/data $<TARGET_FILE_DIR:${PROJECT_NAME}>/data)

message(STATUS "Configuring CMake for WEAL v${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}-${RELEASE_TYPE}")