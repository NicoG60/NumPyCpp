cmake_minimum_required(VERSION 3.0 FATAL_ERROR)
project(numpycpp VERSION 1.0)
set(NUMPYCPP_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR})

#Configure the project
set(CMAKE_DISABLE_IN_SOURCE_BUILD ON)

option(ENABLE_NUMPY_TESTING "Build the unit tests"  OFF)
option(USE_PYTHON3    "Enable this option if you don't have pipenv setup and want to run the tests" OFF)

if(${ENABLE_NUMPY_TESTING})
    enable_testing()
endif()

#install var
set(include_dest "include/numpycpp-${NUMPYCPP_VERSION}")
set(lib_dest "lib/numpycpp-${NUMPYCPP_VERSION}")

# add subdirs
add_subdirectory(src)
add_subdirectory(test)

# install
install(FILES numpycpp-config.cmake DESTINATION ${lib_dest})
install(EXPORT numpycpp DESTINATION ${lib_dest})