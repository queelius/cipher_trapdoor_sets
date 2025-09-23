cmake_minimum_required(VERSION 3.20)
project(cipher_trapdoor_sets 
    VERSION 2.0.0
    DESCRIPTION "Privacy-preserving set operations using cryptographic trapdoors"
    LANGUAGES CXX)

# C++20 for concepts and other modern features
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Export compile commands for tooling
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Options
option(CTS_BUILD_EXAMPLES "Build example programs" ON)
option(CTS_BUILD_TESTS "Build unit tests" ON)
option(CTS_BUILD_BENCHMARKS "Build benchmarks" OFF)
option(CTS_ENABLE_SANITIZERS "Enable address and UB sanitizers" OFF)

# Header-only library
add_library(cipher_trapdoor_sets INTERFACE)
add_library(cts::cts ALIAS cipher_trapdoor_sets)

target_include_directories(cipher_trapdoor_sets
    INTERFACE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

# Require C++20 features
target_compile_features(cipher_trapdoor_sets INTERFACE cxx_std_20)

# Enable parallel STL if available
find_package(TBB QUIET)
if(TBB_FOUND)
    target_link_libraries(cipher_trapdoor_sets INTERFACE TBB::tbb)
    target_compile_definitions(cipher_trapdoor_sets INTERFACE CTS_HAS_PARALLEL_STL)
endif()

# Compiler warnings
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(cipher_trapdoor_sets INTERFACE
        $<$<CONFIG:Debug>:-Wall -Wextra -Wpedantic>
        $<$<CONFIG:Release>:-O3 -march=native>
    )
elseif(MSVC)
    target_compile_options(cipher_trapdoor_sets INTERFACE
        $<$<CONFIG:Debug>:/W4 /WX>
        $<$<CONFIG:Release>:/O2>
    )
endif()

# Sanitizers
if(CTS_ENABLE_SANITIZERS AND CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(cipher_trapdoor_sets INTERFACE
        -fsanitize=address,undefined -fno-omit-frame-pointer)
    target_link_options(cipher_trapdoor_sets INTERFACE
        -fsanitize=address,undefined)
endif()

# Examples
if(CTS_BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()

# Tests
if(CTS_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

# Benchmarks
if(CTS_BUILD_BENCHMARKS)
    add_subdirectory(benchmarks)
endif()

# Installation
include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

install(TARGETS cipher_trapdoor_sets
    EXPORT cipher_trapdoor_sets-targets
)

install(DIRECTORY include/
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

install(EXPORT cipher_trapdoor_sets-targets
    FILE cipher_trapdoor_sets-targets.cmake
    NAMESPACE cts::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/cipher_trapdoor_sets
)

configure_package_config_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/cipher_trapdoor_sets-config.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/cipher_trapdoor_sets-config.cmake"
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/cipher_trapdoor_sets
)

write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/cipher_trapdoor_sets-config-version.cmake"
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion
)

install(FILES
    "${CMAKE_CURRENT_BINARY_DIR}/cipher_trapdoor_sets-config.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/cipher_trapdoor_sets-config-version.cmake"
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/cipher_trapdoor_sets
)