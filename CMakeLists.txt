cmake_minimum_required(VERSION 3.20)

# Set up our projects
project(weal_ncurses VERSION 1.0)       # Runs in the console with ncurses.

# Seting flags in this manner is bad practice.
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -pedantic -lm -g -rdynamic")

# Find required packages
find_package(LibXml2 REQUIRED)
find_package(Curses REQUIRED)

# I know globbing c files is bad practice, but when we're this early in development, it's just plain easier.
file(GLOB SRCS "src/*.c")

# Add executables
add_executable(weal_ncurses ${SRCS} windows/curses/windows.c windows/curses/menu.c)

# Bad practice.
target_include_directories(weal_ncurses PUBLIC include ${LIBXML2_INCLUDE_DIR} ${CURSES_INCLUDE_DIRS})

# Link ncurses libraries
target_link_libraries(weal_ncurses PUBLIC LibXml2::LibXml2 ${CURSES_LIBRARIES})

# Set target properties
set_target_properties(weal_ncurses PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")