find_package(benchmark REQUIRED)

# The list of benchs
file(GLOB_RECURSE all_benchs RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} *.cpp)

foreach(bench ${all_benchs})
    get_filename_component(bench_name ${bench} NAME_WE)
    get_filename_component(bench_dir ${bench} DIRECTORY)
    add_executable(${bench_name} ${bench})
    target_link_libraries(${bench_name} PRIVATE Enda::Enda benchmark::benchmark_main)
    set_property(TARGET ${bench_name} PROPERTY RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/${bench_dir})
    set_target_properties(${bench_name} PROPERTIES
        FOLDER "Benchmarks"
    )
endforeach()
