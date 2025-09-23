cmake_minimum_required(VERSION 3.20)

# Find benchmark library
find_package(benchmark QUIET)

if(benchmark_FOUND)
    # Add benchmark executables
    add_executable(bench_trapdoor bench_trapdoor.cpp)
    target_link_libraries(bench_trapdoor PRIVATE cts::cts benchmark::benchmark)

    add_executable(bench_operations bench_operations.cpp)
    target_link_libraries(bench_operations PRIVATE cts::cts benchmark::benchmark)

    add_executable(bench_sets bench_sets.cpp)
    target_link_libraries(bench_sets PRIVATE cts::cts benchmark::benchmark)
else()
    message(STATUS "Google Benchmark not found. Benchmarks will not be built.")
    message(STATUS "To enable benchmarks, install Google Benchmark:")
    message(STATUS "  git clone https://github.com/google/benchmark.git")
    message(STATUS "  cd benchmark && cmake -E make_directory build")
    message(STATUS "  cd build && cmake .. -DCMAKE_BUILD_TYPE=Release")
    message(STATUS "  sudo cmake --build . --target install")
endif()