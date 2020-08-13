CMAKE_MINIMUM_REQUIRED(VERSION 3.0 FATAL_ERROR)

#Configure the project
project(NumpyCpp)

if(NOT DEFINED CMAKE_CXX_STANDARD)
    set(CMAKE_CXX_STANDARD 17)
    set(CMAKE_CXX_STANDARD_REQUIRED ON)
    set(CMAKE_CXX_EXTENSIONS OFF)
endif()

if(CMAKE_CXX_STANDARD LESS 17)
    message(FATAL_ERROR "CMAKE_CXX_STANDARD is less than 17, NumpyCpp only works with C++17 and above.")
endif()

option(ENABLE_STATIC  "Build static library"  ON)
option(ENABLE_DYNAMIC "Build dynamic library" ON)
option(ENABLE_TESTING "Build the unit tests"  ON)

set(SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)
set(TP_DIR ${CMAKE_CURRENT_SOURCE_DIR}/third_party)

set(INTERFACE_TARGET ${PROJECT_NAME})
set(STATIC_TARGET    ${PROJECT_NAME}-static)
set(DYNAMIC_TARGET   ${PROJECT_NAME}-dynamic)
set(TEST_TARGET      ${PROJECT_NAME}-test)

include_directories(${SRC_DIR})

add_library(${INTERFACE_TARGET} INTERFACE)
target_sources(${INTERFACE_TARGET} INTERFACE ${SRC_DIR}/numpycpp.cpp)
target_include_directories(${INTERFACE_TARGET} INTERFACE ${TP_DIR}/miniz-cpp)

if(ENABLE_DYNAMIC)
    add_library(${DYNAMIC_TARGET} SHARED)
    target_link_libraries(${DYNAMIC_TARGET} ${INTERFACE_TARGET})
    set_target_properties(${DYNAMIC_TARGET} PROPERTIES OUTPUT_NAME ${PROJECT_NAME})

    if(CMAKE_CXX_COMPILER_ID STREQUAL GNU)
        target_link_libraries(${DYNAMIC_TARGET} stdc++fs)
    elseif(CMAKE_CXX_COMPILER_ID STREQUAL Clang)
        target_link_libraries(${DYNAMIC_TARGET} c++fs)
    endif()
endif(ENABLE_DYNAMIC)

if(ENABLE_STATIC)
    add_library(${STATIC_TARGET} STATIC)
    target_link_libraries(${STATIC_TARGET} ${INTERFACE_TARGET})
    set_target_properties(${STATIC_TARGET} PROPERTIES OUTPUT_NAME ${PROJECT_NAME})

    if(CMAKE_CXX_COMPILER_ID STREQUAL GNU)
        target_link_libraries(${STATIC_TARGET} stdc++fs)
    elseif(CMAKE_CXX_COMPILER_ID STREQUAL Clang)
        target_link_libraries(${STATIC_TARGET} c++fs)
    endif()
endif(ENABLE_STATIC)

if(ENABLE_TESTING)
    enable_testing()

    add_subdirectory(${TP_DIR}/Catch2)
    add_executable(${TEST_TARGET}
        test/main.cpp
        test/test_types.cpp
        test/test_npz.cpp
        test/test_f16_error.cpp
        test/test_array.cpp)
    target_link_libraries(${TEST_TARGET} ${INTERFACE_TARGET} Catch2::Catch2)

    include(CTest)
    include(${TP_DIR}/Catch2/contrib/Catch.cmake)

    catch_discover_tests(${TEST_TARGET} WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/test)
endif(ENABLE_TESTING)


