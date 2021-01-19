cmake_minimum_required(VERSION 3.0 FATAL_ERROR)
project(numpycpp)

# define library version (update: apparently you can also do it in project()!)
set(NUMPYCPP_VERSION_MAJOR 1 CACHE STRING "major version" FORCE)
set(NUMPYCPP_VERSION_MINOR 0 CACHE STRING "minor version" FORCE)
set(NUMPYCPP_LIBRARY_VERSION ${NUMPYCPP_VERSION_MAJOR}.${NUMPYCPP_VERSION_MINOR} CACHE STRING "version" FORCE)
set(NUMPYCPP_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR} CACHE STRING "source dir" FORCE)

set(CMAKE_DISABLE_IN_SOURCE_BUILD ON)

#Configure the project
option(ENABLE_TESTING "Build the unit tests"  OFF)
option(USE_PYTHON3    "Enable this option if you don't have pipenv setup and want to run the tests" OFF)

#install var
set(include_dest "include/numpycpp-${NUMPYCPP_VERSION}")
set(lib_dest "lib/numpycpp-${NUMPYCPP_VERSION}")

# add subdirs
add_subdirectory(src)
add_subdirectory(test)

# install
install(FILES numpycpp-config.cmake DESTINATION ${lib_dest})
install(EXPORT numpycpp DESTINATION ${lib_dest})