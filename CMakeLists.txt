cmake_minimum_required(VERSION 3.0 FATAL_ERROR)
project(numpycpp VERSION 1.0)
set(NUMPYCPP_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR})

#Configure the project
set(CMAKE_DISABLE_IN_SOURCE_BUILD ON)

if(NOT CMAKE_CXX_STANDARD)
    set(CMAKE_CXX_STANDARD 17)
    set(CMAKE_CXX_STANDARD_REQUIRED ON)
endif()

# Check if sumpycpp is being used directly or via add_subdirectory, but allow overriding
if(NOT DEFINED NUMPYCPP_MASTER_PROJECT)
    if(CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR)
        set(NUMPYCPP_MASTER_PROJECT ON)
    else()
        set(NUMPYCPP_MASTER_PROJECT OFF)
    endif()
endif()

option(NUMPYCPP_INSTALL     "Generate install targets" ${NUMPYCPP_MASTER_PROJECT})
option(NUMPYCPP_BUILD_TESTS "Build the unit tests"  OFF)
option(NUMPYCPP_USE_PYTHON3 "Enable this option if you don't have pipenv setup and want to run the tests" OFF)

if(${NUMPYCPP_BUILD_TESTS})
    enable_testing()
endif()

#install var
set(include_dest "include/numpycpp-${NUMPYCPP_VERSION}")
set(lib_dest "lib/numpycpp-${NUMPYCPP_VERSION}")

# add subdirs
add_subdirectory(src)
add_subdirectory(test)

# install
if(${NUMPYCPP_INSTALL})
    install(FILES numpycpp-config.cmake DESTINATION ${lib_dest})
    install(EXPORT numpycpp DESTINATION ${lib_dest})
endif()