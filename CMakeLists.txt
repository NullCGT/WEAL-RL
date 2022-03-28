cmake_minimum_required(VERSION 3.20)

project(weal VERSION 1.0)
# Seting flags in this manner is bad practice.
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -pedantic -lm")

find_package(LibXml2 REQUIRED)
find_package(Curses REQUIRED)

# Bad practice.
include_directories(include ${LIBXML2_INCLUDE_DIR} ${CURSES_INCLUDE_DIRS})

# I know globbing c files is bad practice, but when we're this early in development, it's just plain easier.
file(GLOB SRCS "src/*.c")
add_executable(weal ${SRCS})

# target_link_libraries(weal PUBLIC LibXml2::LibXml2 ${CURSES_LIBRARIES} ${CMAKE_SOURCE_DIR}/lib/blt/Linux64/libBearLibTerminal.so)
target_link_libraries(weal PUBLIC LibXml2::LibXml2 ${CURSES_LIBRARIES})