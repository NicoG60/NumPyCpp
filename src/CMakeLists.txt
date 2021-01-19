# set headers
set(header_path "${NUMPYCPP_SOURCE_DIR}/include/numpycpp")
file(GLOB header ${header_path}/*.h)

# set sources
file(GLOB src *.cpp)

# setup deps
execute_process(
    COMMAND git submodule update --init -- third_party/miniz-cpp third_party/utf-cpp
    WORKING_DIRECTORY ${NUMPYCPP_SOURCE_DIR}
)

set(MINIZ_INCLUDE_DIR ${NUMPYCPP_SOURCE_DIR}/third_party/miniz-cpp
    CACHE PATH "miniz include directory")

set(UTF_INCLUDE_DIR ${NUMPYCPP_SOURCE_DIR}/third_party/utf-cpp/include
    CACHE PATH "utf include directory")

if(CMAKE_CXX_COMPILER_ID STREQUAL GNU)
    set(cpp_fs stdc++fs)
elseif(CMAKE_CXX_COMPILER_ID STREQUAL Clang)
    set(cpp_fs c++fs)
endif()

# define library target
add_library(numpycpp ${headers} ${src})
target_link_libraries(numpycpp ${cpp_fs})
target_include_directories(
    numpycpp
    PUBLIC
        $<BUILD_INTERFACE:${NUMPYCPP_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:${lib_dest}>

    PRIVATE
        ${MINIZ_INCLUDE_DIR}
        ${UTF_INCLUDE_DIR}
)
target_compile_features(numpycpp PUBLIC cxx_std_17)

# install
install(TARGETS numpycpp EXPORT numpycpp DESTINATION "${lib_dest}")
install(FILES ${header} DESTINATION "${include_dest}")