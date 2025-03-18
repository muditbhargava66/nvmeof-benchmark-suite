# Add integration test sources
set(INTEGRATION_TEST_SOURCES
    # End-to-end tests
    end_to_end_test.cpp
    
    # Component interaction tests
    workload_and_bottleneck_test.cpp
    bottleneck_and_optimization_test.cpp
    
    # Performance tests
    performance_test.cpp
)

# Add the integration test executable
add_executable(integration_tests
    ${INTEGRATION_TEST_SOURCES}
    main_test.cpp  # Contains test main function
)

# Link with the required libraries
target_link_libraries(integration_tests
    PRIVATE
        nvmeof_lib
        gtest
        gtest_main
        gmock
)

# Add include directories
target_include_directories(integration_tests
    PRIVATE
        ${CMAKE_SOURCE_DIR}/include
        ${CMAKE_SOURCE_DIR}/src
        ${gtest_SOURCE_DIR}/include
        ${gmock_SOURCE_DIR}/include
)

# Enable testing with GoogleTest
gtest_discover_tests(integration_tests
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    PROPERTIES
        TIMEOUT 120  # Longer timeout for integration tests
)

# Create individual test targets for test categories
add_custom_target(run_end_to_end_tests
    COMMAND ${CMAKE_CTEST_COMMAND} -R "end_to_end" --verbose
    DEPENDS integration_tests
    COMMENT "Running end-to-end integration tests"
)

add_custom_target(run_component_interaction_tests
    COMMAND ${CMAKE_CTEST_COMMAND} -R "workload_and_bottleneck|bottleneck_and_optimization" --verbose
    DEPENDS integration_tests
    COMMENT "Running component interaction integration tests"
)

add_custom_target(run_performance_tests
    COMMAND ${CMAKE_CTEST_COMMAND} -R "performance" --verbose
    DEPENDS integration_tests
    COMMENT "Running performance integration tests"
)

# Add test data dependencies
add_custom_command(
    TARGET integration_tests POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
            ${CMAKE_SOURCE_DIR}/data/workload_profiles
            ${CMAKE_CURRENT_BINARY_DIR}/workload_profiles
    COMMENT "Copying test data for integration tests"
)