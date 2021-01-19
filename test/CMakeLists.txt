if(${ENABLE_TESTING})
    enable_testing()

    message(WARNING "Tests uses python and numpy. Be aware numpy will be installed either using pipenv and pip3.")

    if(${USE_PYTHON3})
        execute_process(
            COMMAND pip3 install numpy
            WORKING_DIRECTORY ${NUMPYCPP_SOURCE_DIR}
        )
        add_definitions(-DUSE_PYTHON3)
        set(gen_file_command python3 test/gen_test_files.py)
    else()
        execute_process(
            COMMAND pipenv sync --dev
            WORKING_DIRECTORY ${NUMPYCPP_SOURCE_DIR}
        )
        set(gen_file_command pipenv run python test/gen_test_files.py)
    endif()

    execute_process(
        COMMAND ${gen_file_command}
        WORKING_DIRECTORY ${NUMPYCPP_SOURCE_DIR}
    )

    find_package(Catch2 QUIET)

    if(NOT Catch2_FOUND)
        execute_process(
            COMMAND git submodule update --init -- third_party/Catch2
            WORKING_DIRECTORY ${NUMPYCPP_SOURCE_DIR}
        )
        add_subdirectory(${NUMPYCPP_SOURCE_DIR}/third_party/Catch2 ${CMAKE_CURRENT_BINARY_DIR}/Catch2)
        include(${NUMPYCPP_SOURCE_DIR}/third_party/Catch2/contrib/Catch.cmake)
    endif()

    file(GLOB test_src *.cpp)

    add_executable(numpycpp-test ${test_src})
    target_link_libraries(numpycpp-test numpycpp Catch2::Catch2)

    include(CTest)

    catch_discover_tests(numpycpp-test WORKING_DIRECTORY ${NUMPYCPP_SOURCE_DIR}/test)
endif()

