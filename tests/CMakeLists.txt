# Tests for Cipher Trapdoor Sets

enable_testing()

set(TEST_SOURCES
    test_trapdoor.cpp
    test_sets.cpp
    test_operations.cpp
    test_core.cpp
    test_homomorphic.cpp
    test_serialization_keys.cpp
)

foreach(test_source ${TEST_SOURCES})
    get_filename_component(test_name ${test_source} NAME_WE)
    add_executable(${test_name} ${test_source})
    target_link_libraries(${test_name} PRIVATE cts::cts)
    
    # Register with CTest
    add_test(NAME ${test_name}
             COMMAND ${test_name})
    
    # Set test properties
    set_tests_properties(${test_name} PROPERTIES
        TIMEOUT 30
        LABELS "unit"
    )
endforeach()

# Add a custom target to run all tests
add_custom_target(run_tests
    COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure
    DEPENDS ${TEST_SOURCES}
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    COMMENT "Running all unit tests"
)