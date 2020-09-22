CMAKE_MINIMUM_REQUIRED(VERSION 3.0 FATAL_ERROR)

#Configure the project
project(NumPyCpp)

if(NOT DEFINED CMAKE_CXX_STANDARD)
    set(CMAKE_CXX_STANDARD 17)
    set(CMAKE_CXX_STANDARD_REQUIRED ON)
    set(CMAKE_CXX_EXTENSIONS OFF)
endif()

if(CMAKE_CXX_STANDARD LESS 17)
    message(FATAL_ERROR "CMAKE_CXX_STANDARD is less than 17, NumpyCpp only works with C++17 and above.")
endif()

get_directory_property(hasParent PARENT_DIRECTORY)
if(hasParent)
    set(STATIC_DEFAULT OFF)
    set(DYNAMIC_DEFAULT OFF)
    set(TESTING_DEFAULT OFF)
else()
    set(STATIC_DEFAULT ON)
    set(DYNAMIC_DEFAULT ON)
    set(TESTING_DEFAULT ON)
endif()

option(ENABLE_STATIC  "Build static library"  ${STATIC_DEFAULT})
option(ENABLE_DYNAMIC "Build dynamic library" ${DYNAMIC_DEFAULT})
option(ENABLE_TESTING "Build the unit tests"  ${TESTING_DEFAULT})

set(SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)
set(TP_DIR ${CMAKE_CURRENT_SOURCE_DIR}/third_party)

set(INTERFACE_TARGET ${PROJECT_NAME})
set(STATIC_TARGET    ${PROJECT_NAME}-static)
set(DYNAMIC_TARGET   ${PROJECT_NAME}-dynamic)
set(TEST_TARGET      ${PROJECT_NAME}-test)

file(GLOB SRC ${SRC_DIR}/*.cpp)

include_directories(${SRC_DIR})

add_library(${INTERFACE_TARGET} INTERFACE)
target_sources(${INTERFACE_TARGET} INTERFACE ${SRC})
target_include_directories(${INTERFACE_TARGET} INTERFACE ${SRC_DIR} ${TP_DIR}/miniz-cpp)
if(CMAKE_CXX_COMPILER_ID STREQUAL GNU)
    target_link_libraries(${INTERFACE_TARGET} INTERFACE stdc++fs)
elseif(CMAKE_CXX_COMPILER_ID STREQUAL Clang)
    target_link_libraries(${INTERFACE_TARGET} INTERFACE c++fs)
endif()

if(${ENABLE_DYNAMIC})
    add_library(${DYNAMIC_TARGET} SHARED)
    target_link_libraries(${DYNAMIC_TARGET} ${INTERFACE_TARGET})
    set_target_properties(${DYNAMIC_TARGET} PROPERTIES OUTPUT_NAME ${PROJECT_NAME})
endif()

if(${ENABLE_STATIC})
    add_library(${STATIC_TARGET} STATIC)
    target_link_libraries(${STATIC_TARGET} ${INTERFACE_TARGET})
    set_target_properties(${STATIC_TARGET} PROPERTIES OUTPUT_NAME ${PROJECT_NAME})
endif()

if(${ENABLE_TESTING})
    enable_testing()

    if(DEFINED ENV{USE_PYTHON3})
        add_definitions(-DUSE_PYTHON3)
    endif()

    file(GLOB TEST_SRC ${CMAKE_CURRENT_SOURCE_DIR}/test/*.cpp)

    add_subdirectory(${TP_DIR}/Catch2)

    add_executable(${TEST_TARGET} ${TEST_SRC})
    target_link_libraries(${TEST_TARGET} ${INTERFACE_TARGET} Catch2::Catch2)

    include(CTest)
    include(${TP_DIR}/Catch2/contrib/Catch.cmake)

    catch_discover_tests(${TEST_TARGET} WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/test)
endif()


