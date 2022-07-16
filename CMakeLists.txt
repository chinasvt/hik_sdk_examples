cmake_minimum_required(VERSION 3.22)
project(sdk)

set(CMAKE_CXX_STANDARD 11)

add_definitions(-D__LINUX__)

include_directories(inc)
link_directories(lib)

list(APPEND LINKER_LIBS HCCore hcnetsdk)

add_executable(sdk main.cpp)

target_link_libraries(sdk ${LINKER_LIBS})