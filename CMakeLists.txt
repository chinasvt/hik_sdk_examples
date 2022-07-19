cmake_minimum_required(VERSION 3.22)
project(hik_sdk_samples)

set(CMAKE_CXX_STANDARD 11)

add_definitions(-D__LINUX__)

add_subdirectory(samples)

include_directories(inc)
link_directories(lib)

list(APPEND LINKER_LIBS HCCore hcnetsdk PlayCtrl)

foreach(SAMPLE_NAME ${SAMPLE_SOURCES})
    #unpack the dir "/"
    string(REPLACE "/" ";" SEXY_LIST ${SAMPLE_NAME})
    list(GET SEXY_LIST -1 TEST_CASE_NAME)
    #get the file name without suffix
    string(REPLACE "." ";" SEXY_LIST ${TEST_CASE_NAME})
    list(GET SEXY_LIST 0 TEST_CASE_NAME)
    add_executable(${TEST_CASE_NAME} ${SAMPLE_NAME})
    target_link_libraries(${TEST_CASE_NAME} ${LINKER_LIBS})
    set_target_properties(${TEST_CASE_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY .)
endforeach()