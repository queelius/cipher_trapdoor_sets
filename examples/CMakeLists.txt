# Examples for Cipher Trapdoor Sets

set(EXAMPLE_SOURCES
    basic_trapdoor.cpp
    set_operations.cpp
    serialization.cpp
    cardinality_estimation.cpp
    homomorphic_operations.cpp
    similarity_search.cpp
)

foreach(example_source ${EXAMPLE_SOURCES})
    get_filename_component(example_name ${example_source} NAME_WE)
    add_executable(${example_name} ${example_source})
    target_link_libraries(${example_name} PRIVATE cts::cts)
    
    # Add to test suite
    add_test(NAME example_${example_name}
             COMMAND ${example_name})
endforeach()