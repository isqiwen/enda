file(GLOB_RECURSE test_sources ${CMAKE_CURRENT_LIST_DIR}/*.cpp )

foreach(src_file ${test_sources})
    # Compute the relative path of the file from the current list directory.
    file(RELATIVE_PATH rel_path ${CMAKE_CURRENT_LIST_DIR} ${src_file})

    # Extract the directory part (exclude the file name)
    get_filename_component(dir_path ${rel_path} DIRECTORY)

    # Replace forward slashes with backslashes to match VS folder style.
    string(REPLACE "/" "\\" group_name "${dir_path}")

    get_filename_component(test_name ${src_file} NAME_WE)
    add_executable(${test_name} ${src_file})

    if(group_name STREQUAL "Test")
        set_target_properties(${test_name} PROPERTIES
            FOLDER "UnitTests"
        )
    else()
        set_target_properties(${test_name} PROPERTIES
            FOLDER "UnitTests/${group_name}"
        )
    endif()

    source_group("src" FILES ${src_file})

    if (${CMAKE_PROJECT_NAME}_ENABLE_CODE_COVERAGE)
        target_compile_options(${CMAKE_PROJECT_NAME} PRIVATE -O0 -g -fprofile-arcs -ftest-coverage)
        target_link_options(${CMAKE_PROJECT_NAME} PRIVATE -fprofile-arcs -ftest-coverage)
        message(STATUS "Code coverage is enabled for GCC.")
    endif()

    if(${CMAKE_PROJECT_NAME}_USE_GTEST)
        find_package(GTest REQUIRED)
        target_link_libraries(${test_name}
            PRIVATE
                Enda::Enda
                GTest::GTest
                GTest::Main
        )
    elseif(${CMAKE_PROJECT_NAME}_USE_CATCH2)
        find_package(Catch2 REQUIRED)
        target_link_libraries(${test_name}
            PRIVATE
                Enda::Enda
                Catch2::Catch2
        )
    else()
        message(FATAL_ERROR "No valid unit test framework configured.")
    endif()

    target_compile_options(${test_name} PRIVATE "$<$<C_COMPILER_ID:MSVC>:/utf-8>")

    add_test(
        NAME
            ${test_name}
        COMMAND
            ${test_name}
    )
endforeach()
