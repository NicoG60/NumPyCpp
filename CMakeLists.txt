CMAKE_MINIMUM_REQUIRED(VERSION 3.0 FATAL_ERROR)

project(NumpyCpp)

set(CMAKE_CXX_STANDARD 11)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

option(ENABLE_STATIC "Build static (.a) library" ON)

find_package(ZLIB REQUIRED)

include_directories(${ZLIB_INCLUDE_DIRS})
include_directories(include)

set(SRC src/numpycpp.cpp)
set(SH_NAME ${PROJECT_NAME})
set(ST_NAME ${PROJECT_NAME}-static)

add_library(${SH_NAME} SHARED ${SRC})
target_link_libraries(${SH_NAME} ${ZLIB_LIBRARIES})
install(TARGETS ${SH_NAME} LIBRARY DESTINATION lib PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE)

if(ENABLE_STATIC)
    add_library(${ST_NAME} STATIC ${SRC})
    set_target_properties(${ST_NAME} PROPERTIES OUTPUT_NAME ${SH_NAME})
    install(TARGETS ${ST_NAME} ARCHIVE DESTINATION lib)
endif(ENABLE_STATIC)

install(FILES "numpycpp.h" DESTINATION include)

add_executable(test test/main.cpp test/test.cpp)
target_link_libraries(test ${ST_NAME})
add_test(NAME AllTests COMMAND test WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/test)
