cmake_minimum_required(VERSION 3.20)

project(weal VERSION 1.0)
project(weal_opengl VERSION 1.0)
# Seting flags in this manner is bad practice.
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -pedantic -lm")
find_package(LibXml2 REQUIRED)
find_package(Curses REQUIRED)
find_package(Notcurses REQUIRED)

# Bad practice.
include_directories(include ${LIBXML2_INCLUDE_DIR} ${CURSES_INCLUDE_DIRS} ${Notcurses_INCLUDE_DIRS})

# I know globbing c files is bad practice, but when we're this early in development, it's just plain easier.
file(GLOB SRCS "src/*.c")
add_executable(weal ${SRCS} windows/notcurses/windows.c)
add_executable(weal_opengl ${SRCS} windows/opengl/windows.c)

if (${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
    target_link_libraries(weal_opengl PUBLIC LibXml2::LibXml2 ${CURSES_LIBRARIES} ${CMAKE_SOURCE_DIR}/lib/blt/Linux64/libBearLibTerminal.so)
endif()
if (${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
    target_link_libraries(weal_opengl PUBLIC LibXml2::LibXml2 ${CURSES_LIBRARIES} ${CMAKE_SOURCE_DIR}/lib/blt/Windows64/BearLibTerminal.dll ${CMAKE_SOURCE_DIR}/lib/blt/Linux64/BearLibTerminal.lib)
endif()
if (${CMAKE_SYSTEM_NAME} STREQUAL "Darwin")
    target_link_libraries(weal_opengl PUBLIC LibXml2::LibXml2 ${CURSES_LIBRARIES} ${CMAKE_SOURCE_DIR}/lib/blt/OSX/BearLibTerminal.dylib)
endif()

target_link_libraries(weal PUBLIC LibXml2::LibXml2 ${CURSES_LIBRARIES} ${Notcurses_LIBRARIES})

set_target_properties(weal PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
set_target_properties(weal_opengl PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")