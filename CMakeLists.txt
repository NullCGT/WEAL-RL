cmake_minimum_required(VERSION 3.20)

# Set up our projects
project(weal VERSION 1.0)               # Standard executable. Runs in the console with notcurses.
project(weal_ncurses VERSION 1.0)       # Runs in the console with ncurses.
project(weal_opengl VERSION 1.0)        # Runs in an opengl window with bearlibterminal

# Seting flags in this manner is bad practice.
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -pedantic -lm -rdynamic")

# Find required packages
find_package(LibXml2 REQUIRED)
find_package(Curses REQUIRED)
find_package(Notcurses REQUIRED)

# I know globbing c files is bad practice, but when we're this early in development, it's just plain easier.
file(GLOB SRCS "src/*.c")

# Add executables
add_executable(weal ${SRCS} windows/notcurses/windows.c)
add_executable(weal_ncurses ${SRCS} windows/curses/windows.c)
add_executable(weal_opengl ${SRCS} windows/opengl/windows.c)

# Bad practice.
target_include_directories(weal PUBLIC include ${LIBXML2_INCLUDE_DIR} ${Notcurses_INCLUDE_DIRS})
target_include_directories(weal_ncurses PUBLIC include ${LIBXML2_INCLUDE_DIR} ${CURSES_INCLUDE_DIRS})
target_include_directories(weal_opengl PUBLIC include windows/opengl/include ${LIBXML2_INCLUDE_DIR})

# Link notcurses libraries
target_link_libraries(weal PUBLIC LibXml2::LibXml2 ${Notcurses_LIBRARIES})
# Link ncurses libraries
target_link_libraries(weal_ncurses PUBLIC LibXml2::LibXml2 ${CURSES_LIBRARIES})
# Link bearlibterminal libraries. A bit complicated, seeing as different systems require different libraries.
if (${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
    target_link_libraries(weal_opengl PUBLIC LibXml2::LibXml2 ${CMAKE_SOURCE_DIR}/lib/blt/Linux64/libBearLibTerminal.so)
endif()
if (${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
    target_link_libraries(weal_opengl PUBLIC LibXml2::LibXml2 ${CMAKE_SOURCE_DIR}/lib/blt/Windows64/BearLibTerminal.dll ${CMAKE_SOURCE_DIR}/lib/blt/Linux64/BearLibTerminal.lib)
endif()
if (${CMAKE_SYSTEM_NAME} STREQUAL "Darwin")
    target_link_libraries(weal_opengl PUBLIC LibXml2::LibXml2 ${CMAKE_SOURCE_DIR}/lib/blt/OSX/BearLibTerminal.dylib)
endif()

# Set target properties
set_target_properties(weal PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
set_target_properties(weal_ncurses PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
set_target_properties(weal_opengl PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")