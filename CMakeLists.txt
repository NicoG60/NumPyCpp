CMAKE_MINIMUM_REQUIRED(VERSION 3.0 FATAL_ERROR)

#Configure the project
project(NumpyCpp)

set(CMAKE_CXX_STANDARD 17)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/bin)

option(ENABLE_STATIC "Build static (.a) library" ON)
option(ENABLE_DYNAMIC "Build dynamic lib" OFF)

#Variables
set(TP_DIR ./third_party)
set(MINIZ_INCLUDE ${TP_DIR}/miniz-cpp)
set(CATCH_INCLUDE ${TP_DIR}/Catch2/single_include)

set(SRC ./src/numpycpp.cpp)
set(SH_NAME ${PROJECT_NAME})
set(ST_NAME ${PROJECT_NAME}-static)

#Binaries
include_directories(./src ${MINIZ_INCLUDE} ${CATCH_INCLUDE})

if(ENABLE_DYNAMIC)
    add_library(${SH_NAME} SHARED ${SRC})
    install(TARGETS ${SH_NAME} LIBRARY DESTINATION lib PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE)
endif(ENABLE_DYNAMIC)

if(ENABLE_STATIC)
    add_library(${ST_NAME} STATIC ${SRC})
    set_target_properties(${ST_NAME} PROPERTIES OUTPUT_NAME ${SH_NAME})
    install(TARGETS ${ST_NAME} ARCHIVE DESTINATION lib)

    enable_testing()

    add_executable(unit_test
        test/main.cpp
        test/test_types.cpp
        test/test_npz.cpp
        test/test_f16_error.cpp
        test/test_array.cpp)
    target_link_libraries(unit_test ${ST_NAME})
    add_test(NAME AllTests COMMAND ${CMAKE_CURRENT_BINARY_DIR}/bin/unit_test WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/test)
endif(ENABLE_STATIC)

if(ENABLE_STATIC OR ENABLE_DYNAMIC)
    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/src/numpycpp.h ${CMAKE_CURRENT_BINARY_DIR}/include/numpycpp.h COPYONLY)
    install(FILES "numpycpp.h" DESTINATION include)
endif()


